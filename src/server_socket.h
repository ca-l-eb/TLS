#ifndef CMD_SERVER_SOCKET_H
#define CMD_SERVER_SOCKET_H

#include "socket.h"

namespace cmd
{
class server_socket
{
public:
    virtual ~server_socket() = default;
    virtual cmd::socket::ptr accept() = 0;
    virtual void bind(int port) = 0;
    virtual void close() = 0;
    virtual void listen(int waiting) = 0;
    virtual int get_port() = 0;

protected:
    // return new socket descriptor bound to port
    int bind_server_socket(int port);
};
}  // namespace cmd

#endif
