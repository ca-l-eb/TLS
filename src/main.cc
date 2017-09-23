#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "http_request.h"
#include "http_response.h"
#include "server_socket.h"
#include "socket.h"
#include "ssl_manager.h"
#include "tcp_server.h"
#include "tcp_socket.h"
#include "tls_server.h"
#include "tls_socket.h"
#include "tokenizer.h"

int main(int argc, char *argv[])
{
    std::string host = "https://www.alucard.io:443";

    if (argc >= 2) {
        host = std::string(argv[1]);
    }
    try {
        if (argc == 1) {
            cmd::tls_server serv{"cert.pem", "key.pem"};
            serv.bind(5005);
            serv.listen(1);
            cmd::socket::ptr client = serv.accept();
            cmd::stream stream{client};

            std::string line;
            while (stream.has_more()) {
                stream.next_line(line);
                std::cout << line << "\n";
                line.clear();
            }

            exit(1);
        }

        {
            cmd::socket::ptr s = cmd::ssl_manager::get_socket_ptr();
            s->connect(host, 5005);
            char buf[] = "Hello world\r\n";
            char buf2[] = "Goodbye world\r\n";
            s->send(buf, sizeof(buf), 0);
            s->send(buf2, sizeof(buf2), 0);
            usleep(1000);
            s->close();
        }

        exit(1);
        cmd::http_request r{host};
        r.set_request_method("GET");
        r.connect();
        cmd::http_response response = r.response();

        r = cmd::http_request{host};
        r.set_resource("/fail.html");
        r.connect();

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
        std::cerr << e.what() << "\n";
        std::exit(1);
    }
}
