#ifndef CMDSOCK_UDP_SOCKET_H
#define CMDSOCK_UDP_SOCKET_H

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <string>
#include <vector>

namespace cmd
{
struct inet_addr {
    addrinfo addr;
    sockaddr storage;

    inet_addr();
    inet_addr(const addrinfo &addr);
    inet_addr(const inet_addr &other);
    inet_addr &operator=(const inet_addr &other);
    std::string to_string();
    int get_port();
};

struct inet_addr_list {
    addrinfo *addrs;
    std::string host;

    explicit inet_addr_list(int port);
    inet_addr_list(const std::string &host, int port);
    ~inet_addr_list();
};

class bound_udp_socket
{
public:
    explicit bound_udp_socket(const inet_addr_list &addrs);
    ~bound_udp_socket();
    bound_udp_socket(const bound_udp_socket &) = delete;
    bound_udp_socket &operator=(const bound_udp_socket &) = delete;

    void close();

    ssize_t send(const void *buffer, size_t size, int flags = 0);
    ssize_t send(const std::string &str, int flags = 0);
    ssize_t recv(void *buffer, size_t size, int flags = 0);
    ssize_t recv(std::vector<unsigned char> &buf, int flags = 0);

    ssize_t send(const inet_addr &addr, const void *buffer, size_t size, int flags = 0);
    ssize_t send(const inet_addr &addr, const std::string &str, int flags = 0);
    ssize_t recv(inet_addr &from, void *buffer, size_t size, int flags = 0);
    ssize_t recv(inet_addr &from, std::vector<unsigned char> &buf, int flags = 0);

    const cmd::inet_addr get_address() const;

public:
    int sock_fd;
    cmd::inet_addr addr;
};

class udp_socket
{
public:
    udp_socket();
    ~udp_socket();
    udp_socket(const bound_udp_socket &) = delete;
    udp_socket &operator=(const bound_udp_socket &) = delete;

    void close();
    ssize_t send(const inet_addr &addr, const void *buffer, size_t size, int flags = 0);
    ssize_t send(const inet_addr &addr, const std::string &str, int flags = 0);

private:
    int sock_fd;
};
}

#endif
