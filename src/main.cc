#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "http_request.h"
#include "http_response.h"
#include "plain_socket.h"
#include "socket.h"
#include "ssl_manager.h"
#include "tls_socket.h"
#include "tokenizer.h"

int main(int argc, char *argv[])
{
    std::string host = "https://www.alucard.io:443";

    if (argc >= 2) {
        host = std::string(argv[1]);
    }
    try {
        cmd::ssl_manager manager;
        SSL_CTX *context = manager.get_context();

        cmd::http_request r{host, context};
        r.set_request_method("GET");
        r.connect();

        r.set_resource("/fail.html");
        r.connect();
        sleep(1);

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

        response = r.response();

        std::cout << "Status: " << response.status_code() << "\n";
        std::cout << "-------------------------HEADERS-------------------------\n";
        for (std::string &s : response.headers()) {
            std::cout << s << "\n";
        }
        std::cout << "---------------------------------------------------------\n";

        std::cout << "---------------------------BODY--------------------------\n"
                  << response.body();
        std::cout << "\n---------------------------------------------------------\n";

        exit(0);

        cmd::socket::ptr conn = std::make_shared<cmd::tls_socket>(context);
        conn->connect("irc.chat.twitch.tv", 443);

        std::string request;
        request += "PASS oauth:u1vju09sa06pkijp49gnqni25lc7a9\n";
        request += "NICK exclamation_point_meme\n";
        request += "join #shroud\njoin #summit1g\n";
        conn->send(request);

        cmd::stream stream{conn};
        std::string line;
        while (stream.has_more()) {
            line.clear();
            stream.next_line(line);
            std::cout << line << "\n";
        }

    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        std::exit(1);
    }
}
