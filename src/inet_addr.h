#ifndef CMD_INET_ADDR_H
#define CMD_INET_ADDR_H

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <string>
#include <vector>

namespace cmd
{
enum class inet_proto { udp = IPPROTO_UDP, tcp = IPPROTO_TCP };

enum class inet_family { unspecified = AF_UNSPEC, ipv4 = AF_INET, ipv6 = AF_INET6 };

struct inet_addr {
    addrinfo addr;
    sockaddr storage;

    inet_addr();
    inet_addr(const addrinfo &addr);
    inet_addr(const inet_addr &other);
    inet_addr &operator=(const inet_addr &other);
    std::string to_string();
    int get_port();
    inet_family family();
    inet_proto protocol();
};

struct inet_resolver {
    std::vector<inet_addr> addresses;
    std::string host;

    inet_resolver(const char *host, int port, cmd::inet_proto type,
                  cmd::inet_family = cmd::inet_family::unspecified);
};
}

#endif