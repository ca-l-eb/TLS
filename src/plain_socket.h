#ifndef CMD_PLAIN_SOCKET_H
#define CMD_PLAIN_SOCKET_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <vector>

#include "socket.h"

namespace cmd {

class plain_socket : public cmd::socket {

public:
    plain_socket();
    ~plain_socket();
    void connect(const std::string& host, int port);
    void connect(const char *host, int port);
    void close();
    void send(const char *buffer, int size, int flags = 0);
    void send(const uint8_t *buffer, int size, int flags = 0);
    void send(const std::string& str, int flags = 0);
    int recv(char *buffer, int size, int flags = 0);
    int recv(uint8_t *buffer, int size, int flags = 0);
    int recv(std::vector<char>& buf, int flags = 0);
    int get_fd();

private:
    int sock_fd;
    struct sockaddr_in sin;
};

};

#endif
