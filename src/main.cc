#include <iostream>
#include <string>
#include <cstdlib>
#include <memory>
#include <unistd.h>

#include "ssl_manager.h"
#include "socket.h"
#include "plain_socket.h"
#include "tls_socket.h"

int main(int argc, char *argv[]) {
    std::string host = "irc.chat.twitch.tv";
    std::string resource = "index.html";

    if (argc >= 2) {
        host = std::string(argv[1]);
    }
    if (argc >= 3) {
        uri = std::string(argv[2]);
    }
    try {

        cmd::ssl_manager manager;
        SSL_CTX *ctx = manager.get_context();
        cmd::tls_socket conn {ctx};
        conn.connect(host, 443);

        std::string request;
//        request += "GET /" + resource + " HTTP/1.1\r\n";
//        request += "Host: " + host + "\r\n";
//        request += "Connection: close\r\n";
//        request += "\r\n"; // End of header section


        request += "PASS oauth:u1vju09sa06pkijp49gnqni25lc7a9\n";
        request += "NICK exclamation_point_meme\n";
        request += "join #shroud\njoin #summit1g\n";

        conn.send(request);

        int recv = 0;
        char buffer[2048];
        std::string response;
        while ((recv = conn.recv(buffer, 2048)) > 0) {
            buffer[recv] = '\0';
            response += std::string(buffer, recv);
            std::cout << std::string(buffer, recv);
        }
        std::cout <<"\n\nComplete response:\n\n";
        std::cout << response << "\n";
    }
    catch(std::exception& e) {
        std::cerr << e.what() << "\n";
        std::exit(1);
    }
}
