#ifndef CMD_SSL_MANAGER_H
#define CMD_SSL_MANAGER_H

#include <openssl/ssl.h>

#include "socket.h"

namespace cmd
{
class ssl_manager
{
public:
    static cmd::socket::ptr get_socket_ptr();
    static SSL_CTX *get_context();
    static SSL_CTX *get_server_context();

private:
    ssl_manager();
    ~ssl_manager();
    static ssl_manager &instance()
    {
        static ssl_manager instance;
        return instance;
    }

    SSL_CTX *context;
    const SSL_METHOD *method;
};
};

#endif
