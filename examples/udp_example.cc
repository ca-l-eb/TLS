#include <netdb.h>
#include <chrono>
#include <csignal>
#include <iostream>

#include "udp_socket.h"

int main(int argc, char *argv[])
{
    try {
        char buffer[1024];
        ssize_t ret;

        if (argc == 1) {
            cmd::bound_udp_socket listener{cmd::inet_addr_list{5000}};
            auto other = listener.get_address();
            std::cout << "Connected at " << other.to_string() << " on port " << other.get_port()
                      << "\n";

            while (true) {
                ret = listener.recv(other, buffer, sizeof(buffer));
                std::cout << "Received " << ret << " bytes from " << other.to_string()
                          << " on port " << other.get_port() << "\n";

                if (ret == -1)
                    break;
                buffer[ret] = '\0';
                std::cout << buffer << "\n";
                ret = listener.send(other, buffer, ret);
                std::cout << "Echoed back : " << ret << " bytes\n";
            }

        } else {
            // Lookup methods for connecting to argv[1] on port 5000
            auto addresses = cmd::inet_addr_list{argv[1], 5000};
            auto address = cmd::inet_addr{*addresses.addrs};
            auto ignore = cmd::inet_addr{};
            cmd::udp_socket sender;

            std::string line;
            while (true) {
                std::getline(std::cin, line);
                if (std::cin.eof())
                    break;
                ret = sender.send(address, line);
                std::cout << "sent " << ret << " bytes\n";
                if (ret == -1)
                    break;
                ret = sender.recv(ignore, buffer, sizeof(buffer));
                std::cout << "read back " << ret << "bytes:\n";
                if (ret >= 0) {
                    buffer[ret] = '\0';
                    std::cout << buffer << "\n";
                }
            }
        }
    } catch (std::exception &e) {
        std::cerr << "Exception : " << e.what() << "\n";
    }
}
