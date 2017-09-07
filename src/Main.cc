#include <iostream>
#include <string>
#include <botan/pipe.h>
#include <botan/filters.h>
#include <cstdlib>

#include "socket.h"

int main(int argc, char *argv[]) {
    Botan::Pipe b64_pipe {new Botan::Base64_Encoder};
    b64_pipe.process_msg("Hello, world. This was encoded in base64");

    // process_msg is the same as start_msg, write, end_msg
    b64_pipe.start_msg();
    b64_pipe.write("This is the second message");
    b64_pipe.end_msg();

    b64_pipe.start_msg();
    b64_pipe.write("this is going to be a longer message to see how the lines are split internally in Botan");
    b64_pipe.write("I assume newline are not generate, but I guess we'll see");
    b64_pipe.end_msg();

    b64_pipe.process_msg("The final message");

    for (int i = 0; i < b64_pipe.message_count(); i++)
        std::cout << b64_pipe.read_all_as_string(i) << "\n";

    Botan::Pipe hex_pipe {new Botan::Hex_Encoder};

    hex_pipe.process_msg("This is going to be hex encoded");

    std::cout << hex_pipe.read_all_as_string(0) << "\n";

    std::string host = "www.google.com";
    std::string uri = "index.html";
    if (argc >= 2) {
        host = std::string(argv[1]);
    }
    if (argc >= 3) {
        uri = std::string(argv[2]);
    }
    try {
        cmd::socket conn {host, 80};
        conn.connect();

        std::string request;
        request += "GET /" + uri + " HTTP/1.1\r\n";
        request += "Host: " + host + "\r\n";
//        request += "Connection: close\r\n";
        request += "\r\n"; // End of header section

        conn.send(request);

        int recv = 0;
        char buffer[2048];
        std::string response;
        while ((recv = conn.recv(buffer, 2048)) > 0) {
            buffer[recv] = '\0';
            std::cout << "Got " << recv << " bytes" << "\n";
            response += std::string(buffer, recv);
            std::cout << std::string(buffer, recv);
            if (response == "0")
//                break;
        }
        std::cout <<"\n\nComplete response:\n\n";
        std::cout << response << "\n";
    }
    catch(std::exception& e) {
        std::cerr << e.what();
        std::exit(1);
    }
}
