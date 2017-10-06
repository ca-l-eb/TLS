#ifndef CMD_TCP_SERVER_H
#define CMD_TCP_SERVER_H

#include "server_socket.h"

namespace cmd
{
class tcp_server : public cmd::server_socket
{
public:
    tcp_server();
    tcp_server(const tcp_server &) = delete;
    tcp_server &operator=(const tcp_server &) = delete;
    ~tcp_server() override;
    cmd::socket::ptr accept() override;
    void bind(int port) override;
    void close() override;
    void listen(int waiting) override;
    int get_port() override;

private:
    int sock_fd;
    int port;
};
}  // namespace cmd

#endif
