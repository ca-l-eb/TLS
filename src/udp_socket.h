#ifndef CMDSOCK_UDP_SOCKET_H
#define CMDSOCK_UDP_SOCKET_H

#include <string>
#include <vector>

#include "inet_addr.h"

namespace cmd
{
class bound_udp_socket
{
public:
    explicit bound_udp_socket(int port, inet_family family = inet_family::unspecified);
    ~bound_udp_socket();
    bound_udp_socket(const bound_udp_socket &) = delete;
    bound_udp_socket &operator=(const bound_udp_socket &) = delete;

    void close();

    ssize_t send(const void *buffer, size_t size, int flags = 0);
    ssize_t send(const std::string &str, int flags = 0);
    ssize_t recv(void *buffer, size_t size, int flags = 0);

    ssize_t send(const inet_addr &addr, const void *buffer, size_t size, int flags = 0);
    ssize_t send(const inet_addr &addr, const std::string &str, int flags = 0);
    ssize_t recv(inet_addr &from, void *buffer, size_t size, int flags = 0);

    const cmd::inet_addr get_address() const;

public:
    int sock_fd;
    cmd::inet_addr addr;
};

class udp_socket
{
public:
    explicit udp_socket(inet_family family = inet_family::ipv4);
    ~udp_socket();
    udp_socket(const udp_socket &) = delete;
    udp_socket &operator=(const udp_socket &) = delete;

    void close();
    ssize_t send(const inet_addr &addr, const void *buffer, size_t size, int flags = 0);
    ssize_t send(const inet_addr &addr, const std::string &str, int flags = 0);
    ssize_t recv(inet_addr &addr, void *buffer, size_t size, int flags = 0);
    ssize_t recv(void *buffer, size_t size, int flags = 0);

private:
    int sock_fd;
};
}

#endif
