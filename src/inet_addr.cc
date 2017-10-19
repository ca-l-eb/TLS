#include <arpa/inet.h>
#include <cstring>

#include "exceptions.h"
#include "inet_addr.h"

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
    int port = -1;
    if (addr.ai_addr->sa_family == AF_INET) {
        auto *s = (sockaddr_in *) addr.ai_addr;
        port = ntohs(s->sin_port);
    } else if (addr.ai_addr->sa_family == AF_INET6) {
        auto *s = (sockaddr_in6 *) addr.ai_addr;
        port = ntohs(s->sin6_port);
    }
    return port;
}

cmd::inet_family cmd::inet_addr::family() {
    return static_cast<inet_family>(addr.ai_family);
}

cmd::inet_proto cmd::inet_addr::protocol() {
    return static_cast<inet_proto>(addr.ai_protocol);
}

// nullptr for host indicates localhost
cmd::inet_resolver::inet_resolver(const char *host, int port, inet_proto type, inet_family proto)
{
    addrinfo hints{};
    switch (proto) {
        case inet_family::unspecified:
            hints.ai_family = AF_UNSPEC;
            break;
        case inet_family::ipv4:
            hints.ai_family = AF_INET;
            break;
        case inet_family::ipv6:
            hints.ai_family = AF_INET6;
            break;
    }
    switch (type) {
        case inet_proto::tcp:
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            break;
        case inet_proto::udp:
            hints.ai_socktype = SOCK_DGRAM;
            hints.ai_protocol = IPPROTO_UDP;
            break;
    }
    if (host == nullptr) {
        hints.ai_flags |= AI_PASSIVE;
        this->host = "localhost";
    } else {
        this->host = host;
    }

    addrinfo *addrs;
    int ret = getaddrinfo(host, std::to_string(port).c_str(), &hints, &addrs);
    if (ret != 0)
        throw cmd::socket_exception("Error getaddrinfo: " + std::string(gai_strerror(ret)));

    // Populate addresses
    for (auto i = addrs; i != nullptr; i = i->ai_next)
        addresses.emplace_back(*i);

    if (addrs)
        freeaddrinfo(addrs);
}