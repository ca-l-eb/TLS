#include <cstdint>
#include <chrono>
#include <exception>
#include <random>

#include "base64.h"
#include "websocket.h"
#include "tcp_socket.h"
#include "tls_socket.h"

thread_local std::mt19937 generator{std::chrono::system_clock::now().time_since_epoch().count()};
cmd::websocket::websocket(const std::string &resource, bool secure)
    : resource{resource}, secure{secure}
{
}

cmd::websocket::~websocket() {}

void cmd::websocket::connect(const std::string &host, int port)
{
    connect(host.c_str(), port);
}

void cmd::websocket::connect(const char *host, int port)
{
//    if (secure)
//        sock = std::make_shared<cmd::tls_socket>();
//    else
//        sock = std::make_shared<cmd::tcp_socket>();
//
//    sock->connect(host, port);


    // Want 16 byte random key
    char nonce[16];

    int n = sizeof(nonce) / sizeof(int32_t);
    for (int i = 0; i < n; i++) {
        *((int*) &nonce[i*sizeof(int32_t)]) = generator();
    }

    std::string encoded = cmd::base64::encode(nonce, sizeof(nonce));

    std::string request = "GET " + resource + " HTTP/1.1\r\n";
    request += "Host: " + std::string(host) + "\r\n";
    request += "Upgrade: websocket\r\n";
    request += "Connection: Upgrade\r\n";
    request += "Sec-WebSocket-Key: " + encoded;

}

int cmd::websocket::send(const char *buffer, int size, int flags)
{
    return sock->send(buffer, size, flags);
}

int cmd::websocket::send(const std::string &str, int flags)
{
    return sock->send(str, flags);
}

int cmd::websocket::recv(char *buffer, int size, int flags)
{
    return sock->recv(buffer, size, flags);
}

int cmd::websocket::recv(std::vector<char> &buf, int flags)
{
    return sock->recv(buf, flags);
}

int cmd::websocket::get_fd()
{
    return sock->get_fd();
}

void cmd::websocket::close()
{
    sock->close();
}
