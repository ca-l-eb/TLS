#ifndef CMD_WEBSOCKET_H
#define CMD_WEBSOCKET_H

#include <string>
#include <vector>

#include "http_response.h"
#include "socket.h"
#include "stream.h"

namespace cmd
{
enum class websocket_opcode : unsigned char {
    continuation = 0x0,
    text = 0x1,
    binary = 0x2,
    close = 0x8,
    ping = 0x9,
    pong = 0xA
};

enum class websocket_status_code : uint16_t {
    normal = 1000,
    going_away = 1001,
    protocol_error = 1002,
    data_error = 1003,  // e.g. got binary when expected text
    reserved = 1004,
    no_status_code_present = 1005,  // don't send
    closed_abnormally = 1006,       // don't send
    inconsistent_data = 1007,
    policy_violation = 1008,  // generic code return
    message_too_big = 1009,
    extension_negotiation_failure = 1010,
    unexpected_error = 1011,
    tls_handshake_error = 1015  // don't send
};

struct websocket_frame {
    bool fin;
    websocket_opcode op;
    std::vector<unsigned char> data;
};

static const std::string websocket_guid{"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"};

class websocket
{
public:
    websocket();
    websocket(const socket &) = delete;
    websocket &operator=(const socket &) = delete;
    ~websocket();
    void connect(const std::string &url);
    void connect(const std::string &host, int port, const std::string &resource, bool secure);
    void close(websocket_status_code = websocket_status_code::normal);
    int send(const std::string &str);
    int send(const void *buffer, size_t size);
    websocket_frame next_frame();
    std::vector<unsigned char> next_message();
    size_t next_message(std::vector<unsigned char> &buf);

private:
    cmd::stream stream;
    bool closed;
    uint16_t close_code;

    void check_websocket_upgrade(const std::string &expect, cmd::http_response &response);
    std::vector<unsigned char> build_frame(const void *buffer, size_t size, websocket_opcode op);
    unsigned char *resize_buffer_and_write_size(std::vector<unsigned char> &buf, size_t size);
    void write_masked_data(const uint8_t *in, unsigned char *out, size_t size);
    void pong(std::vector<unsigned char> &msg);
};

}  // namespace cmd

#endif
