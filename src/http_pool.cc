#include <memory>

#include "http_pool.h"
#include "ssl_manager.h"
#include "tcp_socket.h"
#include "tls_socket.h"

cmd::stream &cmd::http_pool::get_connection(const std::string &host, int port, bool is_ssl)
{
    std::map<std::string, cmd::stream> &m = instance().host_to_stream_map;
    std::string full_host = host + ":" + std::to_string(port);
    auto it = m.find(full_host);
    if (it != m.end()) {
        return it->second;
    }

    cmd::socket::ptr sock;
    if (is_ssl) {
        sock = std::make_shared<cmd::tls_socket>();
    } else {
        sock = std::make_shared<cmd::tcp_socket>();
    }
    sock->connect(host, port);
    m.emplace(full_host, sock);
    return m[full_host];
}

void cmd::http_pool::mark_closed(const std::string &host, int port)
{
    std::string full_host = host + ":" + std::to_string(port);
    auto &map = instance().host_to_stream_map;
    auto it = map.find(full_host);
    if (it != map.end()) {
        map.erase(it);
    }
}
