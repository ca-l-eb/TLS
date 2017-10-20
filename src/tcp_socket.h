#ifndef CMD_PLAIN_SOCKET_H
#define CMD_PLAIN_SOCKET_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <vector>

#include "inet_addr.h"
#include "socket.h"

namespace cmd
{
class tcp_socket : public cmd::socket
{
public:
    explicit tcp_socket(cmd::inet_family family = cmd::inet_family::unspecified);
    explicit tcp_socket(int fd);
    tcp_socket(const tcp_socket &) = delete;
    tcp_socket &operator=(const tcp_socket &) = delete;
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
    void connect_host(const std::string &host, int port, cmd::inet_family family);
    int sock_fd;
    int port;
    std::string host;
    cmd::inet_family family;
};
}  // namespace cmd

#endif
