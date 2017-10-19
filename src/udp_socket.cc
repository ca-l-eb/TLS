#include <unistd.h>
#include <cstring>

#include "exceptions.h"
#include "udp_socket.h"

cmd::bound_udp_socket::bound_udp_socket(int port, inet_family family) : sock_fd{-1}
{
    inet_resolver list{nullptr, port, inet_proto::udp, family};
    // Loop through each address trying each
    for (auto &address : list.addresses) {
        sock_fd =
            ::socket(address.addr.ai_family, address.addr.ai_socktype, address.addr.ai_protocol);
        if (sock_fd == -1)
            continue;

        if (::bind(sock_fd, address.addr.ai_addr, address.addr.ai_addrlen) != -1) {
            addr = address;
            return;  // Success!
        }

        ::close(sock_fd);  // close, try next
    }
    throw cmd::socket_exception("Could bind socket to host: " + list.host);
}

cmd::bound_udp_socket::~bound_udp_socket()
{
    close();
}

void cmd::bound_udp_socket::close()
{
    if (sock_fd >= 0) {
        ::close(sock_fd);
        sock_fd = -1;
    }
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

ssize_t cmd::bound_udp_socket::recv(cmd::inet_addr &from, void *buffer, size_t size, int flags)
{
    return ::recvfrom(sock_fd, buffer, size, flags, from.addr.ai_addr, &from.addr.ai_addrlen);
}

const cmd::inet_addr cmd::bound_udp_socket::get_address() const
{
    return addr;
}

cmd::udp_socket::udp_socket(cmd::inet_family family) : sock_fd{-1}
{
    switch (family) {
        case inet_family::ipv4:
            sock_fd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (sock_fd >= 0)
                break;
            throw cmd::socket_exception{"Could not make ipv4 UDP socket"};
        case inet_family::ipv6:
            sock_fd = ::socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
            if (sock_fd >= 0)
                break;
            throw cmd::socket_exception{"Could not make ipv6 UDP socket"};
        default:
            throw cmd::socket_exception{"Unsupported internet family for UDP: " +
                                        std::to_string((int) family)};
    }
}

cmd::udp_socket::~udp_socket()
{
    close();
}

void cmd::udp_socket::close()
{
    if (sock_fd >= 0) {
        ::close(sock_fd);
        sock_fd = -1;
    }
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

ssize_t cmd::udp_socket::recv(cmd::inet_addr &addr, void *buffer, size_t size, int flags)
{
    return ::recvfrom(sock_fd, buffer, size, flags, addr.addr.ai_addr, &addr.addr.ai_addrlen);
}

ssize_t cmd::udp_socket::recv(void *buffer, size_t size, int flags)
{
    return ::recvfrom(sock_fd, buffer, size, flags, nullptr, nullptr);
}