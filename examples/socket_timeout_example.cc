#include <iostream>
#include "udp_socket.h"

int main(int argc, char *argv[])
{
    try {
        cmd::inet_addr addr;
        char buffer[256];
        cmd::udp_socket sock;
        // Try to read a udp packet, timeout after 1000 milliseconds
        auto read = sock.recv(buffer, sizeof(buffer), 0, 1000);

        if (read == 0)
            std::cout << "Recv timed out\n";
        else {
            std::cout << "Read " << read << " bytes from " << addr.to_string() << " "
                      << addr.get_port() << " before timeout\n";
            std::cout.write(buffer, read);
            std::cout << "\n";
        }
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}