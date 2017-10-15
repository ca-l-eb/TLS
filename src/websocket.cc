#include <openssl/sha.h>
#include <chrono>
#include <cmath>
#include <exception>
#include <map>
#include <random>

#include "base64.h"
#include "http_pool.h"
#include "http_request.h"
#include "tcp_socket.h"
#include "tls_socket.h"
#include "websocket.h"

using random_byte_engine =
    std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char>;
thread_local random_byte_engine generator{
    static_cast<unsigned char>(std::chrono::system_clock::now().time_since_epoch().count())};

cmd::websocket::socket::socket(const std::string &resource, bool secure)
    : stream{nullptr}, resource{resource}, secure{secure}, closed{false}
{
}

cmd::websocket::socket::~socket()
{
    sock->close();
}

void cmd::websocket::socket::connect(const std::string &host, int port)
{
    sock = cmd::http_pool::get_connection(host, port, secure);

    // Consider storing streams somewhere with global access
    // because they can contain information from other "connections"
    stream = cmd::stream{sock};

    // Want 16 byte random key
    std::vector<uint8_t> nonce(16);
    std::generate(nonce.begin(), nonce.end(), std::ref(generator));

    std::string encoded = cmd::base64::encode(nonce.data(), nonce.size());

    cmd::http_request req{stream};
    req.set_resource(resource);
    req.set_header("Upgrade", "websocket");
    req.set_header("Connection", "Upgrade");
    req.set_header("Sec-WebSocket-Key", encoded);
    req.set_header("Origin", host);
    req.set_header("Sec-WebSocket-Version", "13");

    for (auto &a : headers)
        req.set_header(a.first, a.second);
    headers.clear();

    req.connect();  // Send the request

    std::string concat = encoded + cmd::websocket::guid;
    unsigned char shasum[20];
    SHA1(reinterpret_cast<const unsigned char *>(concat.c_str()), concat.size(), shasum);
    std::string expect = cmd::base64::encode(shasum, sizeof(shasum));

    // Get the http response from stream, check for upgrade
    http_response response{req.response()};
    check_websocket_upgrade(expect, response);  // Exception if fails

    // Connection upgraded! WebSocket is ready
}

void cmd::websocket::socket::check_websocket_upgrade(const std::string &expect,
                                                     cmd::http_response &response)
{
    if (response.status_code() != 101) {
        // No version renegotiation. We only support WebSocket v13
        throw std::runtime_error(
            "Server did not accept WebSocket upgrade: " + std::to_string(response.status_code()) +
            " " + response.status_message());
    }

    auto &headers_map = response.headers();
    //    auto it = headers_map.find("Sec-WebSocket-Accept");
    auto it = headers_map.find("sec-websocket-accept");
    if (it == headers_map.end())
        throw std::runtime_error("No Sec-WebSocket-Accept found");
    if (it->second != expect)
        throw std::runtime_error("Got invalid Sec-WebSocket-Accept");
}

int cmd::websocket::socket::send(const void *buffer, size_t size, int flags)
{
    std::vector<unsigned char> buf =
        build_frame(buffer, size, websocket::opcode::text);  // Always text for now

    return static_cast<int>(sock->send(buf.data(), buf.size(), flags));
}

std::vector<unsigned char> cmd::websocket::socket::build_frame(const void *buffer, size_t size,
                                                               cmd::websocket::opcode op)
{
    std::vector<unsigned char> buf;

    const auto *read_from = reinterpret_cast<const uint8_t *>(buffer);

    // location where to start writing the rest of the message
    // (mask + mask-encoded data)
    unsigned char *write_to = resize_buffer_and_write_size(buf, size);

    // Set FIN bit and opcode in first byte of frame
    buf[0] = 0x80;  // FIN bit set (this is a complete frame)
    buf[0] |= static_cast<unsigned char>(op);

    write_masked_data(read_from, write_to, size);

    return buf;
}

// Resizes the vector to hold the entire WebSocket frame and returns
// the address of where to begin writing the rest of the frame
unsigned char *cmd::websocket::socket::resize_buffer_and_write_size(std::vector<unsigned char> &buf,
                                                                    size_t size)
{
    const uint8_t mask_bit = 0x80;
    if (size == 0) {
        buf.resize(6);      // 6 byte is smallest frame size
        buf[1] = mask_bit;  // Data is masked... but there actually no data
        return &buf[2];
    } else if (size < 126) {
        buf.resize(size + 6);
        buf[1] = static_cast<unsigned char>(mask_bit | (uint8_t) size);
        return &buf[2];
    } else if (size <= std::numeric_limits<uint16_t>::max()) {  // 16 bit unsigned max
        buf.resize(size + 8);
        buf[1] = mask_bit | 126;
        buf[2] = static_cast<unsigned char>((size & 0xFF00) >> 8);
        buf[3] = static_cast<unsigned char>(size & 0x00FF);
        return &buf[4];
    } else {
        buf.resize(size + 14);
        buf[1] = mask_bit | 127;
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
void cmd::websocket::socket::write_masked_data(const uint8_t *in, unsigned char *out, size_t size)
{
    auto mask0 = generator();
    auto mask1 = generator();
    auto mask2 = generator();
    auto mask3 = generator();

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

int cmd::websocket::socket::send(const std::string &str, int flags)
{
    return send(str.c_str(), str.size(), flags);
}

// Returns the next WebSocket frame. May be a fragmented message;
// to read an entire message, use next_message()
cmd::websocket::frame cmd::websocket::socket::next_frame()
{
    websocket::frame frame{};

    uint8_t tmp[8];  // Temporary buffer for frame data
    size_t read = stream.read(tmp, 2);
    if (read != 2) {
        if (closed) {
            frame.fin = true;
            frame.op = websocket::opcode::close;
            // If connection already closed and we couldn't read more from the stream. The reason we
            // don't check closed above before the read is because there might be messages queued up
            // that have yet to be received by the caller (e.g. multiple sends followed by a close
            // without any reads in between)
            return frame;
        }
        throw std::runtime_error("Could not read next WebSocket frame: no data");
    }

    const uint8_t reserved_flags = 0x70;
    const uint8_t mask_bit = 0x80;

    if (tmp[0] & reserved_flags) {
        throw std::runtime_error("Unexpected extension flag");
    }
    if (tmp[1] & mask_bit) {
        // ERROR: server sent masked data
        close(websocket::status_code::protocol_error);
        throw std::runtime_error("WebSocket protocol error: server sent masked data");
    }

    auto fin = static_cast<bool>(tmp[0] & 0x80);
    auto op = static_cast<websocket::opcode>(tmp[0] & 0xF);

    auto len = static_cast<uint64_t>(tmp[1] & 0x7F);
    if (len < 126) {
        // len is already set
    } else if (len == 126) {
        // Next 2 bytes are length
        read = stream.read(tmp, 2);
        if (read != 2) {
            throw std::runtime_error("Could not read WebSocket frame length");
        }
        // Convert next 2 bytes to network order
        len = (tmp[0] << 8) | tmp[1];
    } else {
        // Next 8 bytes are length
        read = stream.read(tmp, 8);
        if (read != 8) {
            throw std::runtime_error("Could not read WebSocket frame length");
        }
        // Convert next 8 bytes from network order
        len = (static_cast<uint64_t>(tmp[7]) << 56) | (static_cast<uint64_t>(tmp[6]) << 48) |
              (static_cast<uint64_t>(tmp[5]) << 40) | (static_cast<uint64_t>(tmp[4]) << 32) |
              (static_cast<uint64_t>(tmp[3]) << 24) | (static_cast<uint64_t>(tmp[2]) << 16) |
              (static_cast<uint64_t>(tmp[1]) << 8) | (static_cast<uint64_t>(tmp[0]));
    }

    frame.fin = fin;
    frame.op = op;
    frame.data.resize(len);

    if (len > 0)
        read = stream.read(frame.data.data(), len);

    if (read != len) {
        throw std::runtime_error("Could not read entire WebSocket frame: expected " +
                                 std::to_string(len) + " but read " + std::to_string(read));
    }

    switch (op) {
        case websocket::opcode::close:
            if (closed)
                break;
            if (len >= 2) {
                // Respond with the same code sent
                uint16_t code = 0;
                code |= (frame.data[0] << 8) | frame.data[1];
                close(static_cast<websocket::status_code>(code));
            } else {
                // Invalid WebSocket close frame... respond with normal close response
                close(websocket::status_code::normal);
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

void cmd::websocket::socket::pong(std::vector<unsigned char> &msg)
{
    std::vector<unsigned char> f{build_frame(msg.data(), msg.size(), websocket::opcode::pong)};
    // At most, send 125 bytes in control frame
    send(f.data(), std::min((size_t) f.size(), (size_t) 125), 0);
}

std::vector<unsigned char> cmd::websocket::socket::next_message()
{
    std::vector<unsigned char> message;
    websocket::frame f = next_frame();
    websocket::opcode code = f.op;
    if (code == websocket::opcode::text || code == websocket::opcode::binary)
        message.insert(message.end(), f.data.begin(), f.data.end());

    while (!f.fin) {
        f = next_frame();
        switch (f.op) {
            case websocket::opcode::continuation:
                message.insert(message.end(), f.data.begin(), f.data.end());
                break;
            case websocket::opcode::text:
            case websocket::opcode::binary:
                // We're expecting continuation frame if the FIN bit wasn't set
                close(websocket::status_code::inconsistent_data);
                break;
            case websocket::opcode::close:
            case websocket::opcode::ping:
            case websocket::opcode::pong:
                // next_frame handles pings and close
                break;
            default:
                close(websocket::status_code::protocol_error);
        }
    }
    return message;
}

void cmd::websocket::socket::close()
{
    if (!closed)
        close(websocket::status_code::normal);
    closed = true;
}

void cmd::websocket::socket::close(websocket::status_code code)
{
    auto c = static_cast<std::underlying_type<cmd::websocket::status_code>::type>(code);

    // Make sure the code is in network byte order
    unsigned char buf[2];
    buf[0] = static_cast<unsigned char>((c & 0xFF00) >> 8);
    buf[1] = static_cast<unsigned char>(c & 0x00FF);

    std::vector<unsigned char> frame{build_frame(buf, sizeof(buf), websocket::opcode::close)};
    sock->send(frame.data(), frame.size(), 0);
}

void cmd::websocket::socket::set_header(const std::string &header, const std::string &value)
{
    headers[header] = value;
}
