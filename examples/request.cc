#include <http_pool.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>

#include "http_request.h"
#include "ssl_manager.h"
#include "tcp_socket.h"
#include "tls_socket.h"

int main(int argc, char *argv[])
{
    try {
        auto sock = cmd::http_pool::get_connection("www.alucard.io", 443, true);
        cmd::stream stream{sock};
        cmd::http_request r{stream};
        r.set_request_method("GET");
        r.set_resource("/index.html");
        r.connect();
        cmd::http_response response = r.response();

        std::cout << "Status: " << response.status_code() << "\n";
        std::cout << "-------------------------HEADERS-------------------------\n";
        for (auto &s : response.headers()) {
            std::cout << s.first << ": " << s.second << "\n";
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
