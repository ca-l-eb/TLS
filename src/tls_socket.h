#ifndef CMD_TLS_SOCKET
#define CMD_TLS_SOCKET

#include <tls.h>
#include <string>

#include "socket.h"

namespace cmd {

class tls_socket : public cmd::socket {
public:    
    tls_socket(struct tls_config *tls_conf);
    ~tls_socket();
    void connect(const std::string &host, int port);
    void connect(const char *host, int port);
    void close();
    void send(const char *buffer, int size, int flags = 0);
    void send(const uint8_t *buffer, int size, int flags = 0);
    void send(const std::string& str, int flags = 0);
    int recv(char *buffer, int size, int flags = 0);
    int recv(uint8_t *buffer, int size, int flags = 0);
    int recv(std::vector<char>& buf, int flags = 0);

private:
    struct tls *client; 
};

};

#endif
