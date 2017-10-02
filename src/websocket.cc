#include <openssl/sha.h>
#include <chrono>
#include <cstdint>
#include <exception>
#include <map>
#include <random>

#include "base64.h"
#include "http_response.h"
#include "tcp_socket.h"
#include "tls_socket.h"
#include "websocket.h"

thread_local std::mt19937 generator{
    (unsigned) std::chrono::system_clock::now().time_since_epoch().count()};

cmd::websocket::websocket(const std::string &resource, bool secure)
    : stream{nullptr}, resource{resource}, secure{secure}, closed{false}
{
}

cmd::websocket::~websocket() {}

void cmd::websocket::connect(const std::string &host, int port)
{
    if (secure)
        sock = std::make_shared<cmd::tls_socket>();
    else
        sock = std::make_shared<cmd::tcp_socket>();

    sock->connect(host, port);
    stream = cmd::stream{sock};

    // Want 16 byte random key
    char nonce[16];

    int n = sizeof(nonce) / sizeof(int32_t);
    for (int i = 0; i < n; i++) {
        *((int *) &nonce[i * sizeof(int32_t)]) = generator();
    }

    std::string encoded = cmd::base64::encode(nonce, sizeof(nonce));

    std::string request = "GET " + resource + " HTTP/1.1\r\n";
    request += "Host: " + std::string(host) + "\r\n";
    request += "Upgrade: websocket\r\n";
    request += "Connection: Upgrade\r\n";
    request += "Sec-WebSocket-Key: " + encoded + "\r\n";
    request += "Origin: " + std::string(host) + "\r\n";
    request += "Sec-WebSocket-Version: 13\r\n";
    request += "\r\n";

    sock->send(request, 0);

    std::string concat = encoded + websocket_guid;
    unsigned char shasum[20];
    SHA1((const unsigned char *) concat.c_str(), concat.size(), shasum);
    std::string expect = cmd::base64::encode((char *) shasum, sizeof(shasum));

    cmd::http_response response{stream};  // Read http_response from stream
    check_websocket_upgrade(expect, response);

    // Connection upgraded! WebSocket is ready
}

void cmd::websocket::check_websocket_upgrade(const std::string &expect,
                                             cmd::http_response &response)
{
    if (response.status_code() != 101)
        throw std::runtime_error("Server did not accept WebSocket upgrade");
    // No version renegotiation. We only support WebSocket v13

    auto headers_map = response.headers();
    auto it = headers_map.find("Sec-WebSocket-Accept");
    if (it == headers_map.end())
        throw std::runtime_error("No Sec-WebSocket-Accept found");
    if (it->second != expect)
        throw std::runtime_error("Got invalid Sec-WebSocket-Accept");
}

int cmd::websocket::send(const void *buffer, int size, int flags)
{
    std::vector<unsigned char> buf = build_frame(buffer, size, false);  // Always text for now
    return sock->send(buf.data(), buf.size(), flags);
}

std::vector<unsigned char> cmd::websocket::build_frame(const void *buffer, int size,
                                                       bool binary_data)
{
    std::vector<unsigned char> buf;

    const uint8_t *buffer8 = (const uint8_t *) buffer;
    int loc;
    if (size < 126) {
        buf.resize(size + 6);
        buf[1] = 0x80 | (uint8_t) size;
        loc = 2;
    } else if (size < 65536) {  // 16 bit unsigned max
        buf.resize(size + 8);
        buf[1] = 0x80 | 126;
        buf[2] = (size & 0xFF00) >> 8;
        buf[3] = (size & 0x00FF);
        loc = 4;
    } else {
        buf.resize(size + 14);
        buf[1] = 0x80 | 127;
        // We're only reading sizes up to an int, but fill 8 bytes according
        // to WebSocket protocol. (64 bit message size seems overkill)
        buf[2] = (size & 0x7F00000000000000) >> 56;  // MSB always 0
        buf[3] = (size & 0x00FF000000000000) >> 48;
        buf[4] = (size & 0x0000FF0000000000) >> 40;
        buf[5] = (size & 0x000000FF00000000) >> 32;
        buf[6] = (size & 0x00000000FF000000) >> 24;
        buf[7] = (size & 0x0000000000FF0000) >> 16;
        buf[8] = (size & 0x000000000000FF00) >> 8;
        buf[9] = (size & 0x00000000000000FF);
        loc = 10;
    }
    buf[0] = binary_data ? 0x82 : 0x81;  // 8 marks this is entire frame, 2 binary, 1 text

    uint32_t mask = generator();

    uint8_t mask0 = static_cast<uint8_t>((mask & 0xFF000000) >> 24);
    uint8_t mask1 = static_cast<uint8_t>((mask & 0x00FF0000) >> 16);
    uint8_t mask2 = static_cast<uint8_t>((mask & 0x0000FF00) >> 8);
    uint8_t mask3 = static_cast<uint8_t>(mask & 0x000000FF);

    buf[loc++] = mask0;
    buf[loc++] = mask1;
    buf[loc++] = mask2;
    buf[loc++] = mask3;

    int i = 0;
    while (size > 3) {
        buf[loc++] = buffer8[i++] ^ mask0;
        buf[loc++] = buffer8[i++] ^ mask1;
        buf[loc++] = buffer8[i++] ^ mask2;
        buf[loc++] = buffer8[i++] ^ mask3;
        size -= 4;
    }
    if (size > 0)
        buf[loc++] = buffer8[i++] ^ mask0;
    if (size > 1)
        buf[loc++] = buffer8[i++] ^ mask1;
    if (size > 2)
        buf[loc++] = buffer8[i++] ^ mask2;

    return buf;
}

int cmd::websocket::send(const std::string &str, int flags)
{
    return send(str.c_str(), str.size(), flags);
}

cmd::websock::frame cmd::websocket::next_frame()
{
    cmd::websock::frame frame{};

    uint8_t tmp[8];  // Temporary buffer for frame data
    stream.read(tmp, 2);

    bool fin = tmp[0] & 0x80;
    websock::opcode op = static_cast<websock::opcode>(tmp[0] & 0xF);
    uint64_t len = tmp[1] & 0x7F;
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

    if (len > 0)
        stream.read(&frame.data[0], len);

    switch (op) {
        case websock::opcode::close:
            closed = true;
            break;
        case websock::opcode::ping:
            // TODO: reply with pong
            break;
        default:
            // Other opcodes do not require special behavior
            break;
    }

    return frame;
}

void cmd::websocket::close()
{
    sock->close();
}
