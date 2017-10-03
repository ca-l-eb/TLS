#ifndef CMD_SOCKET_HELP_H
#define CMD_SOCKET_HELP_H

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>

namespace cmd
{
void print_connection_info(int fd)
{
    struct sockaddr connection;
    socklen_t len = sizeof(connection);
    std::memset(&client, 0, len);

    getpeername(fd, &connection, &len);

    char host[256];
    char address[64];
    char service[32];
    auto addr;
    switch (client.sa_family) {
        case AF_INET:
            addr = &((struct sockaddr_in *) &connection)->sin_addr;
            break;
        case AF_INET6:
            addr = &((struct sockaddr_in6 *) &connection)->sin6_addr;
            break;
        default:
            address[0] = '\0';
    }
    inet_ntop(connection->sa_family, addr, address, sizeof(address));
    getnameinfo(&client, len, host, sizeof(host), service, sizeof(service), 0);

    std::cout << "Connection from " << host << ":" << service << "(" << address << ")\n";
}
}  // namespace cmd

#endif
