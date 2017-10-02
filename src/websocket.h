#ifndef CMD_WEBSOCKET_H
#define CMD_WEBSOCKET_H

#include <string>
#include <vector>

#include "http_response.h"
#include "socket.h"
#include "stream.h"

namespace cmd
{
namespace websock
{
enum class opcode : unsigned char {
    continuation = 0x0,
    text = 0x1,
    binary = 0x2,
    close = 0x8,
    ping = 0x9,
    pong = 0xA
};

struct frame {
    bool fin;
    opcode op;
    std::vector<unsigned char> data;
};
}

class websocket
{
public:
    websocket(const std::string &resource, bool secure);
    ~websocket();
    void connect(const std::string &host, int port);
    void close();
    int send(const void *buffer, int size, int flags = 0);
    int send(const std::string &str, int flags = 0);
    websock::frame next_frame();

private:
    cmd::socket::ptr sock;
    cmd::stream stream;
    std::string resource;
    bool secure;
    bool closed;
    const std::string websocket_guid{"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"};

    void check_websocket_upgrade(const std::string &expect, cmd::http_response &response);
    std::vector<unsigned char> build_frame(const void *buffer, int size, bool binary_data);
};
}

#endif
