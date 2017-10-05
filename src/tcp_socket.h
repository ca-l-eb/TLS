#ifndef CMD_PLAIN_SOCKET_H
#define CMD_PLAIN_SOCKET_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <vector>

#include "socket.h"

namespace cmd
{
class tcp_socket : public cmd::socket
{
public:
    tcp_socket();

    explicit tcp_socket(int fd);
    ~tcp_socket() override;
    void connect(const std::string &host, int port) override;
    void close() override;
    ssize_t send(const void *buffer, size_t size, int flags) override;
    ssize_t send(const std::string &str, int flags) override;
    ssize_t recv(void *buffer, size_t size, int flags) override;
    ssize_t recv(std::vector<unsigned char> &buf, int flags) override;
    int get_fd() override;
    int get_port() override;
    std::string get_host() override;

private:
    void connect_host(const std::string &host, int port);
    int sock_fd;
    int port;
    std::string host;
};
}  // namespace cmd

#endif
