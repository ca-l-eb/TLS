#include <iostream>
#include <string>
#include <cstdlib>
#include <memory>
#include <unistd.h>

#include "ssl_manager.h"
#include "socket.h"
#include "plain_socket.h"
#include "tls_socket.h"
#include "tokenizer.h"
#include "http_request.h"

int main(int argc, char *argv[]) {
    std::string host = "irc.chat.twitch.tv";
    std::string resource = "index.html";



    if (argc >= 2) {
        host = std::string(argv[1]);
    }
    if (argc >= 3) {
        resource = std::string(argv[2]);
    }
    try {

        cmd::ssl_manager manager;
        SSL_CTX* context = manager.get_context();

        cmd::http_request r ("https://api.twitch.tv/kraken/streams/?stream_type=live&limit=5", context);
        r.set_request_method("GET");
        r.set_header("Connection", "close");
        r.set_header("Client-ID", "mptmk1yiiqjdz560cy354ehkrh4xpo");
        r.connect();
        r.response();
        int s = r.status_code();
        std::cout << "Status: " << s << "\n";

        exit(1);




        cmd::tls_socket conn {context};

        conn.connect(host, 443);
        std::string request;
        request += "GET /" + resource + " HTTP/1.1\r\n";
        request += "Host: " + host + "\r\n";
        request += "Client-ID: mptmk1yiiqjdz560cy354ehkrh4xpo\r\n";
        request += "Connection: close\r\n";
        request += "\r\n"; // End of header section


//        request += "PASS oauth:u1vju09sa06pkijp49gnqni25lc7a9\n";
//        request += "NICK exclamation_point_meme\n";
//        request += "join #shroud\njoin #summit1g\n";

        conn.send(request);

        int recv = 0;
        char buffer[2048];
        std::string complete;

        std::string line;
        cmd::tokenizer t;
        cmd::line_token tok;

        while ((recv = conn.recv(buffer, 2048)) > 0) {
            int remaining = recv;
//            std::cout << "\nRead " << recv << " bytes\n";
            char *buf_ptr = buffer; 
            while (remaining > 0) {
                char *next = t.get_line(buf_ptr, remaining, line, tok);
                if (tok == cmd::line_token::COMPLETE) {
                    complete += line + "\n";
//                    std::cout << "C: '" << line << "'\n";
                    line.clear();
                }
                remaining -= next - buf_ptr;
                buf_ptr = next;
            }

        }
        std::cout <<"\n\nComplete response:\n\n";
        std::cout << complete << "\n";
    }
    catch(std::exception& e) {
        std::cerr << e.what() << "\n";
        std::exit(1);
    }
}
