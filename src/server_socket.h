#ifndef CMD_SERVER_SOCKET_H
#define CMD_SERVER_SOCKET_H

#include "socket.h"

namespace cmd
{
class server_socket
{
public:
    virtual ~server_socket() {}
    virtual cmd::socket::ptr accept() = 0;
    virtual void bind(int port) = 0;
    virtual void close() = 0;
    virtual void listen(int waiting) = 0;
    int get_port()
    {
        return port;
    }

protected:
    int port;
};
}

#endif
