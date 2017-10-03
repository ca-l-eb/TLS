#ifndef CMD_TCP_SERVER_H
#define CMD_TCP_SERVER_H

#include "server_socket.h"

namespace cmd
{
// return new socket descriptor bound to port
int bind_server_socket(int port);

class tcp_server : public cmd::server_socket
{
public:
    tcp_server();
    ~tcp_server();
    cmd::socket::ptr accept();
    void bind(int port);
    void close();
    void listen(int waiting);

private:
    int sock_fd;
};
}  // namespace cmd

#endif
