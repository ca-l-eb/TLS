#include <unistd.h>
#include <cstring>

#include "exceptions.h"
#include "udp_socket.h"

#include <arpa/inet.h>

cmd::bound_udp_socket::bound_udp_socket(const cmd::inet_addr_list &addrs) : sock_fd{-1}
{
    // Loop through each address trying each
    for (auto a = addrs.addrs; a != nullptr; a = a->ai_next) {
        sock_fd = ::socket(a->ai_family, a->ai_socktype, a->ai_protocol);
        if (sock_fd == -1)
            continue;

        if (::bind(sock_fd, a->ai_addr, a->ai_addrlen) != -1) {
            addr = cmd::inet_addr{*a};
            return;  // Success!
        }

        ::close(sock_fd);  // close, try next
    }
    throw cmd::socket_exception("Could bind socket to host: " + addrs.host);
}

cmd::bound_udp_socket::~bound_udp_socket()
{
    close();
}

void cmd::bound_udp_socket::close()
{
    if (sock_fd >= 0)
        ::close(sock_fd);
    sock_fd = -1;
}

ssize_t cmd::bound_udp_socket::send(const cmd::inet_addr &addr, const void *buffer, size_t size,
                                    int flags)
{
    return sendto(sock_fd, buffer, size, flags, addr.addr.ai_addr, addr.addr.ai_addrlen);
}

ssize_t cmd::bound_udp_socket::send(const cmd::inet_addr &addr, const std::string &str, int flags)
{
    return send(addr, str.c_str(), str.size(), flags);
}

ssize_t cmd::bound_udp_socket::send(const void *buffer, size_t size, int flags)
{
    return send(addr, buffer, size, flags);
}

ssize_t cmd::bound_udp_socket::send(const std::string &str, int flags)
{
    return send(addr, str.c_str(), str.size(), flags);
}

ssize_t cmd::bound_udp_socket::recv(void *buffer, size_t size, int flags)
{
    return ::recv(sock_fd, buffer, size, flags);
}

ssize_t cmd::bound_udp_socket::recv(std::vector<unsigned char> &buf, int flags)
{
    return recv(buf.data(), buf.size(), flags);
}

ssize_t cmd::bound_udp_socket::recv(cmd::inet_addr &from, void *buffer, size_t size, int flags)
{
    return ::recvfrom(sock_fd, buffer, size, flags, from.addr.ai_addr, &from.addr.ai_addrlen);
}

ssize_t cmd::bound_udp_socket::recv(cmd::inet_addr &from, std::vector<unsigned char> &buf,
                                    int flags)
{
    return recv(from, buf.data(), buf.size(), flags);
}

const cmd::inet_addr cmd::bound_udp_socket::get_address() const
{
    return addr;
}

cmd::inet_addr_list::inet_addr_list(int port) : host{"localhost"}
{
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;      // Use IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;   // Request datagram
    hints.ai_protocol = IPPROTO_UDP;  // UDP
    hints.ai_flags = AI_PASSIVE;
    int ret = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &addrs);
    if (ret != 0) {
        throw cmd::socket_exception("Error getaddrinfo: " + std::string(gai_strerror(ret)));
    }
}

cmd::inet_addr_list::inet_addr_list(const std::string &host, int port) : host{host}
{
    addrinfo hints{};
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;      // Use IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;   // Request datagram
    hints.ai_protocol = IPPROTO_UDP;  // UDP

    int ret = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &addrs);
    if (ret != 0) {
        throw cmd::socket_exception("Error getaddrinfo: " + std::string(gai_strerror(ret)));
    }
}

cmd::inet_addr_list::~inet_addr_list()
{
    if (addrs)
        freeaddrinfo(addrs);
}

cmd::udp_socket::udp_socket() : sock_fd{-1}
{
    sock_fd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_fd == -1)
        throw cmd::socket_exception("Could not create IPv4 socket for UDP");
}

cmd::udp_socket::~udp_socket()
{
    close();
}

void cmd::udp_socket::close()
{
    if (sock_fd >= 0)
        ::close(sock_fd);
    sock_fd = -1;
}

ssize_t cmd::udp_socket::send(const cmd::inet_addr &addr, const void *buffer, size_t size,
                              int flags)
{
    return ::sendto(sock_fd, buffer, size, flags, addr.addr.ai_addr, addr.addr.ai_addrlen);
}

ssize_t cmd::udp_socket::send(const cmd::inet_addr &addr, const std::string &str, int flags)
{
    return send(addr, str.c_str(), str.size(), flags);
}

cmd::inet_addr::inet_addr() : addr{}, storage{}
{
    addr.ai_addr = &storage;
    addr.ai_addrlen = sizeof(storage);
}

cmd::inet_addr::inet_addr(const addrinfo &a)
{
    addr = a;
    addr.ai_canonname = nullptr;
    addr.ai_next = nullptr;
    addr.ai_addr = &storage;
    std::memcpy(&storage, a.ai_addr, sizeof(storage));
}

cmd::inet_addr::inet_addr(const cmd::inet_addr &other)
{
    addr = other.addr;
    addr.ai_addr = &storage;
    storage = other.storage;
}

cmd::inet_addr &cmd::inet_addr::operator=(const cmd::inet_addr &other)
{
    addr = other.addr;
    addr.ai_addr = &storage;
    storage = other.storage;
    return *this;
}

std::string cmd::inet_addr::to_string()
{
    char address_string[INET6_ADDRSTRLEN];
    const char *ret = nullptr;
    if (addr.ai_addr->sa_family == AF_INET) {
        auto *s = (sockaddr_in *) addr.ai_addr;
        ret = inet_ntop(AF_INET, &s->sin_addr, address_string, sizeof(address_string));

    } else if (addr.ai_addr->sa_family == AF_INET6) {
        auto *s = (sockaddr_in6 *) addr.ai_addr;
        ret = inet_ntop(AF_INET6, &s->sin6_addr, address_string, sizeof(address_string));
    }
    if (!ret)
        return "";
    return std::string{address_string};
}

int cmd::inet_addr::get_port()
{
    int port;
    if (addr.ai_addr->sa_family == AF_INET) {
        auto *s = (sockaddr_in *) addr.ai_addr;
        port = ntohs(s->sin_port);
    } else {  // AF_INET6
        auto *s = (sockaddr_in6 *) addr.ai_addr;
        port = ntohs(s->sin6_port);
    }
    return port;
}
