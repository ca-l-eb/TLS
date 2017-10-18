#ifndef CMDSOCK_UDP_SOCKET_H
#define CMDSOCK_UDP_SOCKET_H

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <string>
#include <vector>

namespace cmd
{
using address = struct addrinfo;

class address_list
{
public:
    struct addrinfo *addrs;
    std::string host;

    explicit address_list(int port);
    ~address_list();
    address_list(const std::string &host, int port);
};

class bound_udp_socket
{
public:
    explicit bound_udp_socket(const address_list &addrs);
    ~bound_udp_socket();
    bound_udp_socket(const bound_udp_socket &) = delete;
    bound_udp_socket &operator=(const bound_udp_socket &) = delete;

    void close();

    ssize_t send(const address &addr, const void *buffer, size_t size, int flags);
    ssize_t send(const address &addr, const std::string &str, int flags);
    ssize_t send(const void *buffer, size_t size, int flags);
    ssize_t send(const std::string &str, int flags);
    ssize_t recv(void *buffer, size_t size, int flags);
    ssize_t recv(std::vector<unsigned char> &buf, int flags);

    const address get_address() const;

private:
    int sock_fd;
    struct addrinfo addr;
};

class udp_socket
{
public:
    udp_socket();
    ~udp_socket();
    udp_socket(const bound_udp_socket &) = delete;
    udp_socket &operator=(const bound_udp_socket &) = delete;

    void close();

    ssize_t send(const address &addr, const void *buffer, size_t size, int flags);
    ssize_t send(const address &addr, const std::string &str, int flags);

private:
    int sock_fd;
};
}

#endif
