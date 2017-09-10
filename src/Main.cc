#include <iostream>
#include <string>
#include <cstdlib>
#include <memory>
#include <tls.h>

#include "socket.h"
#include "plain_socket.h"
#include "tls_socket.h"

int main(int argc, char *argv[]) {
    if (tls_init() == -1) {
        std::cerr << "Could not initialize LibreSSL\n";
        exit(1);
    }
    struct tls_config *tls_conf = tls_config_new();
    if (tls_conf == NULL) {
        std::cerr << "TLS configuration error: ";
        std::cerr << tls_config_error(tls_conf) << "\n";
        exit(1);
    }

    std::string host = "www.google.com";
    std::string uri = "index.html";

    if (argc >= 2) {
        host = std::string(argv[1]);
    }
    if (argc >= 3) {
        uri = std::string(argv[2]);
    }
    try {

        std::shared_ptr<cmd::socket> conn {new cmd::tls_socket{tls_conf}};
//        std::shared_ptr<cmd::socket> conn {new cmd::plain_socket};
        conn->connect(host, 443);

        std::string request;
//        request += "GET /" + uri + " HTTP/1.1\r\n";
//        request += "Host: " + host + "\r\n";
//        request += "Connection: close\r\n";
//        request += "\r\n"; // End of header section


        request += "PASS oauth:u1vju09sa06pkijp49gnqni25lc7a9\n";
        request += "NICK exclamation_point_meme\n";
        request += "join #shroud\njoin #summit1g\n";

        conn->send(request);

        int recv = 0;
        char buffer[2048];
        std::string response;
        while ((recv = conn->recv(buffer, 2048)) > 0) {
            buffer[recv] = '\0';
            std::cout << "Got " << recv << " bytes" << "\n";
            response += std::string(buffer, recv);
            std::cout << std::string(buffer, recv);
        }
        std::cout <<"\n\nComplete response:\n\n";
        std::cout << response << "\n";
    }
    catch(std::exception& e) {
        std::cerr << e.what();
        std::exit(1);
    }
}
