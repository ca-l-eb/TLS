#include <exception>
#include <iomanip>
#include <iostream>
#include <string>

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

        // Make sure null terminated so we can cast to char*
        msg.push_back('\0');
        char *str = reinterpret_cast<char *>(msg.data());

        std::cout << str << "\n";

    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
}
