#ifndef CMD_WEBSOCKET_H
#define CMD_WEBSOCKET_H

#include <string>
#include <vector>

#include "socket.h"

namespace cmd
{
class websocket : public socket
{
public:
    websocket(const std::string &resource, bool secure);
    ~websocket();
    void connect(const std::string &host, int port);
    void connect(const char *host, int port);
    void close();
    int send(const char *buffer, int size, int flags = 0);
    int send(const std::string &str, int flags = 0);
    int recv(char *buffer, int size, int flags = 0);
    int recv(std::vector<char> &buf, int flags = 0);
    int get_fd();

private:
    cmd::socket::ptr sock;
    std::string resource;
    bool secure;
    const std::string websocket_guid {"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"};

};
}

#endif
