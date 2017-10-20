#include <exception>
#include <iomanip>
#include <iostream>

#include "http_pool.h"
#include "websocket.h"

int main(int argc, char *argv[])
{
    try {
        cmd::websocket sock{"ws://demos.kaazing.com/echo"};
        sock.connect();
        std::string s = "Hello, world!";
        sock.send(s);
        sock.close();
        std::vector<unsigned char> msg = sock.next_message();

        std::cout << "Received:\n";
        std::cout.write(reinterpret_cast<char *>(msg.data()), msg.size());
        std::cout << "\n";

        cmd::websocket_frame f = sock.next_frame();

        std::cout << "Closing message:\n";
        for (auto c : f.data) {
            std::cout << std::setw(3) << std::hex << (int) c;
        }
        std::cout << "\n";
    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
}
