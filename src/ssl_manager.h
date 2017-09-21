#ifndef CMD_SSL_MANAGER_H
#define CMD_SSL_MANAGER_H

#include <openssl/ssl.h>

#include "socket.h"

namespace cmd
{
class ssl_manager
{
public:
    static ssl_manager &instance()
    {
        static ssl_manager instance;
        return instance;
    }
    ssl_manager(ssl_manager const &) = delete;
    void operator=(ssl_manager const &) = delete;

    SSL_CTX *get_context() const;
    cmd::socket::ptr get_socket_ptr();

private:
    ssl_manager();
    ~ssl_manager();
    SSL_CTX *context;
    const SSL_METHOD *method;
};
};

#endif
