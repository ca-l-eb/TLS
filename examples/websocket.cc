#include <exception>
#include <iomanip>
#include <iostream>

#include "socket.h"
#include "websocket.h"

int main(int argc, char *argv[])
{
    try {
        std::string resource{"/echo"};
        cmd::websocket::websock sock{resource, false};
        sock.connect("demos.kaazing.com", 80);
        std::string s = "Hello, world!";
        sock.send(s);

        std::vector<unsigned char> msg = sock.next_message();

        for (auto c : msg) {
            std::cout << std::setw(3) << std::hex << (int) c;
        }
        std::cout << "\n";

        char *str = reinterpret_cast<char *>(msg.data());
        std::cout.write(str, msg.size());
        std::cout << "\n";

    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
}
