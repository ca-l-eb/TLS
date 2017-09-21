#ifndef CMD_HTTP_POOL_H
#define CMD_HTTP_POOL_H

#include <map>
#include <string>

#include "socket.h"

namespace cmd
{
class http_pool
{
public:
    static cmd::socket::ptr get_connection(const std::string &host, int port, bool is_ssl);
    static void mark_closed(const std::string &host, int port);

    http_pool(http_pool const &) = delete;
    void operator=(http_pool const &) = delete;

private:
    http_pool() {}
    static http_pool &instance()
    {
        static http_pool instance;
        return instance;
    }
    std::map<std::string, cmd::socket::ptr> host_socket_map;
};
}

#endif
