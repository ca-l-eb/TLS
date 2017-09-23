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
    tcp_socket(int fd);
    ~tcp_socket();
    void connect(const std::string &host, int port);
    void connect(const char *host, int port);
    void close();
    int send(const char *buffer, int size, int flags = 0);
    int send(const std::string &str, int flags = 0);
    int recv(char *buffer, int size, int flags = 0);
    int recv(std::vector<char> &buf, int flags = 0);
    int get_fd();

private:
    void connect_host(const std::string &host, int port);
    int sock_fd;
};
};

#endif
