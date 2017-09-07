#ifndef CMD_SOCKET_H
#define CMD_SOCKET_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <vector>

namespace cmd {

class socket {

public:
    socket(const std::string& host, int port);
    socket(const char *host, int port);
    socket(int sock_fd, struct sockaddr_in sin);
    ~socket();
    void connect();
    void close();
    void send(char *buffer, int size, int flags = 0);
    void send(const std::string& str, int flags = 0);
    int recv(char *buffer, int size, int flags = 0);
    int recv(std::vector<char>& buf, int flags = 0);

private:
    int sock_fd;
    struct sockaddr_in sin;
};

};

#endif
