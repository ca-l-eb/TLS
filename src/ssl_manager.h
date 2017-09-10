#ifndef CMD_SSL_MANAGER_H
#define CMD_SSL_MANAGER_H

#include <openssl/ssl.h>

#include "tls_socket.h"

namespace cmd {

class ssl_manager {
public:
    ssl_manager();
    ~ssl_manager();
    SSL_CTX *get_context() const;

private:
    SSL_CTX *context;
    const SSL_METHOD *method;
};

};

#endif
