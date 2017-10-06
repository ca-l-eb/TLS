#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>

#include "server_socket.h"
#include "ssl_manager.h"
#include "stream.h"
#include "tcp_server.h"
#include "tcp_socket.h"
#include "tls_server.h"
#include "tls_socket.h"

void handle_client(cmd::socket::ptr client)
{
    cmd::stream s{client};
    int fd = client->get_fd();
    std::cout << "Connection " << fd << " opened\n";
    std::string line;
    while (s.has_more()) {
        size_t read = s.next_line(line);
        client->send(line + "\n", 0);
        std::cout << fd << "(" << read << "): " << line << "\n";
        line.clear();
    }
    std::cout << "Connection " << fd << " closed\n";
}

int main(int argc, char *argv[])
{
    int port = 5005;
    try {
        if (argc == 2) {
            port = std::stoi(argv[1]);
        }
    } catch (std::exception &e) {
        std::cerr << "Usage: " << argv[0] << " [port]\n";
        exit(1);
    }

    std::signal(SIGPIPE, SIG_IGN);  // Ignore SIGPIPE

    try {
        // cmd::tls_server serv{"cert.pem", "key.pem"};
        cmd::tcp_server serv;
        serv.bind(port);
        serv.listen(64);
        std::cout << "Using port " << port << "\n";
        while (true) {
            std::thread t(handle_client, serv.accept());
            t.detach();
        }
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
        std::exit(1);
    }
}
