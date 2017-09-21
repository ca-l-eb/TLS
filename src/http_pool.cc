#include <memory>

#include "http_pool.h"
#include "plain_socket.h"
#include "ssl_manager.h"

cmd::socket::ptr cmd::http_pool::get_connection(const std::string &host, int port, bool is_ssl)
{
    std::map<std::string, cmd::socket::ptr> &m = instance().host_socket_map;
    std::string full_host = host + ":" + std::to_string(port);
    auto it = m.find(full_host);
    if (it != m.end()) {
        return it->second;
    }

    cmd::socket::ptr sock;
    if (is_ssl) {
        sock = cmd::ssl_manager::instance().get_socket_ptr();
    } else {
        sock = std::make_shared<cmd::plain_socket>();
    }
    m[full_host] = sock;
    sock->connect(host, port);
    return sock;
}

void cmd::http_pool::mark_closed(const std::string &host, int port)
{
    std::string full_host = host + ":" + std::to_string(port);
    auto &map = instance().host_socket_map;
    auto it = map.find(full_host);
    if (it != map.end()) {
        map.erase(it);
    }
}
