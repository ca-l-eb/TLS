#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "http_request.h"
#include "http_response.h"
#include "socket.h"
#include "ssl_manager.h"
#include "tcp_socket.h"
#include "tls_socket.h"

int main(int argc, char *argv[])
{
    std::string host = "https://www.alucard.io:443";

    if (argc >= 2) {
        host = std::string(argv[1]);
    }
    try {
        cmd::http_request r{host};
        r.set_request_method("GET");
        r.connect();
        cmd::http_response response = r.response();

        std::cout << "Status: " << response.status_code() << "\n";
        std::cout << "-------------------------HEADERS-------------------------\n";
        for (std::string &s : response.headers()) {
            std::cout << s << "\n";
        }
        std::cout << "---------------------------------------------------------\n";

        std::cout << "---------------------------BODY--------------------------\n"
                  << response.body();
        std::cout << "\n---------------------------------------------------------\n";
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
        std::exit(1);
    }
}
