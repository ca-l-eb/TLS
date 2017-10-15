#include <exception>
#include <iomanip>
#include <iostream>

#include "socket.h"
#include "websocket.h"
#include "http_pool.h"


int main(int argc, char *argv[])
{
    try {
        std::string resource{"/echo"};
        auto connection = cmd::http_pool::get_connection("demos.kaazing.com", 80, false);
        auto stream = cmd::stream{connection};

        cmd::websocket::socket sock{resource, stream};
        sock.connect();
        std::string s = "Hello, world!";
        sock.send(s);
        sock.close();
        std::vector<unsigned char> msg = sock.next_message();

        std::cout << "Received:\n";
        std::cout.write(reinterpret_cast<char *>(msg.data()), msg.size());
        std::cout << "\n";

        cmd::websocket::frame f = sock.next_frame();

        std::cout << "Closing message:\n";
        for (auto c : f.data) {
            std::cout << std::setw(3) << std::hex << (int) c;
        }
        std::cout << "\n";
    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
}
