#ifndef CMD_HTTP_POOL_H
#define CMD_HTTP_POOL_H

#include <map>
#include <string>

#include "stream.h"

namespace cmd
{
class http_pool
{
public:
    http_pool(const http_pool &) = delete;
    http_pool &operator=(const http_pool &) = delete;
    static cmd::stream &get_connection(const std::string &host, int port, bool is_ssl);
    static void mark_closed(const std::string &host, int port);

private:
    http_pool() = default;
    static http_pool &instance()
    {
        static http_pool instance;
        return instance;
    }
    std::map<std::string, cmd::stream> host_to_stream_map;
};
}  // namespace cmd

#endif
