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
        cmd::websocket sock{resource, false};
        sock.connect("demos.kaazing.com", 80);
        std::string s = "Hello, world!";
        sock.send(s);

        cmd::websock::frame f;
        do {
            f = sock.next_frame();

            int read = f.data.size();
            std::cout << "Read " << read << " bytes\n";
            for (int i = 0; i < read; i++) {
                std::cout << std::setw(3) << std::hex << (int) f.data[i];
            }
            std::cout << "\n";
        } while (!f.fin);  // Read until we get fin frame

    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
}
