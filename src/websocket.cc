#include <openssl/sha.h>
#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>

#include "base64.h"
#include "http_pool.h"
#include "http_request.h"
#include "tcp_socket.h"
#include "tls_socket.h"
#include "websocket.h"

thread_local std::mt19937 generator{
    (unsigned) std::chrono::system_clock::now().time_since_epoch().count()};

cmd::websocket::websock::websock(const std::string &resource, bool secure)
    : stream{nullptr}, resource{resource}, secure{secure}, closed{false}
{
}

void cmd::websocket::websock::connect(const std::string &host, int port)
{
    sock = cmd::http_pool::get_connection(host, port, secure);

    // Consider storing streams somewhere with global access
    // because they can contain information from other "connections"
    stream = cmd::stream{sock};

    // Want 16 byte random key
    unsigned char nonce[16];

    // generator() returns unsigned long, figure out how many calls we need
    size_t n = sizeof(nonce) / sizeof(unsigned long);
    auto *nonce32 = reinterpret_cast<unsigned long *>(nonce);
    for (size_t i = 0; i < n; i++) {
        nonce32[i] = generator();
    }

    std::string encoded = cmd::base64::encode(nonce, sizeof(nonce));

    cmd::http_request req{stream};
    req.set_resource(resource);
    req.set_header("Upgrade", "websocket");
    req.set_header("Connection", "Upgrade");
    req.set_header("Sec-WebSocket-Key", encoded);
    req.set_header("Origin", host);
    req.set_header("Sec-WebSocket-Version", "13");
    req.connect();  // Send the request

    std::string concat = encoded + cmd::websocket::guid;
    unsigned char shasum[20];
    SHA1((const unsigned char *) concat.c_str(), concat.size(), shasum);
    std::string expect = cmd::base64::encode(shasum, sizeof(shasum));

    cmd::http_response response{req.response()};  // Read http_response from stream
    check_websocket_upgrade(expect, response);    // Exception if fails

    // Connection upgraded! WebSocket is ready
}

void cmd::websocket::websock::check_websocket_upgrade(const std::string &expect,
                                                      cmd::http_response &response)
{
    if (response.status_code() != 101) {
        throw std::runtime_error(
            "Server did not accept WebSocket upgrade: " + std::to_string(response.status_code()) +
            " " + response.status_message());
    }
    // No version renegotiation. We only support WebSocket v13

    auto headers_map = response.headers();
    auto it = headers_map.find("Sec-WebSocket-Accept");
    if (it == headers_map.end())
        throw std::runtime_error("No Sec-WebSocket-Accept found");
    if (it->second != expect)
        throw std::runtime_error("Got invalid Sec-WebSocket-Accept");
}

int cmd::websocket::websock::send(const void *buffer, size_t size, int flags)
{
    std::vector<unsigned char> buf =
        build_frame(buffer, size, websocket::opcode::text);  // Always text for now

    auto f = std::cout.flags();
    for (auto i : buf) {
        std::cout << std::setw(3) << std::hex << (int) i;
    }
    std::cout.flags(f);
    std::cout << "\n";

    return static_cast<int>(sock->send(buf.data(), buf.size(), flags));
}

std::vector<unsigned char> cmd::websocket::websock::build_frame(const void *buffer, size_t size,
                                                                cmd::websocket::opcode op)
{
    std::vector<unsigned char> buf;

    const auto *read_from = reinterpret_cast<const uint8_t *>(buffer);
    unsigned char *write_to = resize_buffer_and_write_size(buf, size);
    // location where to start writing the rest of the message
    // (mask + mask-encoded data)

    auto opcode_value = static_cast<std::underlying_type<cmd::websocket::opcode>::type>(op);
    buf[0] = 0x80;  // FIN bit set (this is a complete frame)
    buf[0] |= opcode_value;

    write_masked_data(read_from, write_to, size);

    return buf;
}

// Resizes the vector to hold the entire WebSocket frame and returns
// the index of where to begin writing the rest of the frame
unsigned char *cmd::websocket::websock::resize_buffer_and_write_size(
    std::vector<unsigned char> &buf, size_t size)
{
    if (size == 0) {
        buf.resize(6);  // buf[0] = {FIN(1), RSV(3), opcode(4)}, buf[1] = {mask(1),
                        // size(7)}, buf[2-5] = {32-bit mask}
        buf[1] = 0x8;   // Mask = 1, size = 0;
        return &buf[2];
    } else if (size < 126) {
        buf.resize(size + 6);
        buf[1] = static_cast<unsigned char>(0x80 | (uint8_t) size);
        return &buf[2];
    } else if (size < 65536) {  // 16 bit unsigned max
        buf.resize(size + 8);
        buf[1] = 0x80 | 126;
        buf[2] = static_cast<unsigned char>((size & 0xFF00) >> 8);
        buf[3] = static_cast<unsigned char>(size & 0x00FF);
        return &buf[4];
    } else {
        buf.resize(size + 14);
        buf[1] = 0x80 | 127;
        // We're only reading sizes up to an int, but fill 8 bytes according
        // to WebSocket protocol. (64 bit message size seems overkill)
        buf[2] = static_cast<unsigned char>((size & 0x7F00000000000000) >> 56);  // MSB always 0
        buf[3] = static_cast<unsigned char>((size & 0x00FF000000000000) >> 48);
        buf[4] = static_cast<unsigned char>((size & 0x0000FF0000000000) >> 40);
        buf[5] = static_cast<unsigned char>((size & 0x000000FF00000000) >> 32);
        buf[6] = static_cast<unsigned char>((size & 0x00000000FF000000) >> 24);
        buf[7] = static_cast<unsigned char>((size & 0x0000000000FF0000) >> 16);
        buf[8] = static_cast<unsigned char>((size & 0x000000000000FF00) >> 8);
        buf[9] = static_cast<unsigned char>(size & 0x00000000000000FF);
        return &buf[10];
    }
}

// Assumes there is room in out for size + 4 bytes
void cmd::websocket::websock::write_masked_data(const uint8_t *in, unsigned char *out, size_t size)
{
    auto mask = static_cast<uint32_t>(generator());

    auto mask0 = static_cast<uint8_t>((mask & 0xFF000000) >> 24);
    auto mask1 = static_cast<uint8_t>((mask & 0x00FF0000) >> 16);
    auto mask2 = static_cast<uint8_t>((mask & 0x0000FF00) >> 8);
    auto mask3 = static_cast<uint8_t>(mask & 0x000000FF);

    *out++ = mask0;
    *out++ = mask1;
    *out++ = mask2;
    *out++ = mask3;

    while (size > 3) {
        *out++ = *in++ ^ mask0;
        *out++ = *in++ ^ mask1;
        *out++ = *in++ ^ mask2;
        *out++ = *in++ ^ mask3;
        size -= 4;
    }
    if (size > 0)
        *out++ = *in++ ^ mask0;
    if (size > 1)
        *out++ = *in++ ^ mask1;
    if (size > 2)
        *out++ = *in++ ^ mask2;
}

int cmd::websocket::websock::send(const std::string &str, int flags)
{
    return send(str.c_str(), str.size(), flags);
}

// Returns the next WebSocket frame. May be a fragmented message;
// to read an entire message, use next_message()
cmd::websocket::frame cmd::websocket::websock::next_frame()
{
    websocket::frame frame{};

    uint8_t tmp[8];  // Temporary buffer for frame data
    stream.read(tmp, 2);

    bool fin = static_cast<bool>(tmp[0] & 0x80);
    if (tmp[0] & 0x70) {
        throw std::runtime_error("Unexpected extenstion flag");
    }
    auto op = static_cast<websocket::opcode>(tmp[0] & 0xF);
    auto len = static_cast<uint64_t>(tmp[1] & 0x7F);
    if (len < 126) {
        // len is already set
    } else if (len == 126) {
        // Next 2 bytes are length
        stream.read(tmp, 2);
        len = (tmp[0] << 8) | tmp[1];
    } else {
        // Next 8 bytes are length
        stream.read(tmp, 8);
        len = (static_cast<uint64_t>(tmp[7]) << 56) | (static_cast<uint64_t>(tmp[6]) << 48) |
              (static_cast<uint64_t>(tmp[5]) << 40) | (static_cast<uint64_t>(tmp[4]) << 32) |
              (static_cast<uint64_t>(tmp[3]) << 24) | (static_cast<uint64_t>(tmp[2]) << 16) |
              (static_cast<uint64_t>(tmp[1]) << 8) | (static_cast<uint64_t>(tmp[0]));
    }

    frame.fin = fin;
    frame.op = op;
    frame.data.resize(len);

    // TODO: keep track of the type of data (binary/text) currently being
    //       read if the frame is fragmented

    if (len > 0)
        stream.read(frame.data.data(), len);

    switch (op) {
        case websocket::opcode::close:
            closed = true;
            if (len > 0) {
                // TODO: extract closing information (e.g. code and message)
                //       and reply with close status
            }
            break;
        case websocket::opcode::ping:
            // Reply with pong; build frame containing the Application data read
            pong(frame.data);
            break;
        default:
            // Other opcodes do not require special behavior
            break;
    }

    return frame;
}

void cmd::websocket::websock::pong(std::vector<unsigned char> &msg)
{
    std::vector<unsigned char> f{build_frame(msg.data(), msg.size(), websocket::opcode::pong)};
    send(f.data(), f.size(), 0);
}

std::vector<unsigned char> cmd::websocket::websock::next_message()
{
    std::vector<unsigned char> message;
    websocket::frame f;
    do {
        f = next_frame();
        // Append the frame to the message
        message.insert(message.end(), f.data.begin(), f.data.end());
    } while (!f.fin);
    return message;
}

void cmd::websocket::websock::close()
{
    sock->close();
}
