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
            // Create and bind udp socket to port 5000
            cmd::bound_udp_socket listener{5000, cmd::inet_family::ipv4};
            auto at = listener.get_address();
            cmd::inet_addr client;

            std::cout << "Bound at " << at.to_string() << " on port " << at.get_port() << "\n";

            while (true) {
                ret = listener.recv(client, buffer, sizeof(buffer));
                std::cout << "Received " << ret << " bytes from " << client.to_string()
                          << " on port " << client.get_port() << "\n";

                if (ret == -1) {
                    perror("recv failed");
                    break;
                }
                std::cout.write(buffer, ret);
                std::cout << "\n";
                ret = listener.send(client, buffer, ret);
                std::cout << "Echoed back : " << ret << " bytes\n";
                if (ret == -1) {
                    perror("echo back failed");
                }
            }

        } else {
            // Lookup methods for connecting to argv[1] on port 5000 using udp
            cmd::inet_resolver options{argv[1], 5000, cmd::inet_proto::udp, cmd::inet_family::ipv4};

            // Let's just take the first of the available methods
            cmd::inet_addr address = options.addresses.front();
            for (auto &a : options.addresses) {
                std::cout << a.to_string() << " port " << a.get_port() << " ";
                switch (a.protocol()) {
                    case cmd::inet_proto::udp:
                        std::cout << "udp ";
                        break;
                    case cmd::inet_proto::tcp:
                        std::cout << "tcp ";
                        break;
                    default:
                        std::cout << "unknown type: " << (int) a.protocol();
                }
                switch (a.family()) {
                    case cmd::inet_family::unspecified:
                        std::cout << "unspecified protocol ";
                        break;
                    case cmd::inet_family::ipv4:
                        std::cout << "ipv4 ";
                        break;
                    case cmd::inet_family::ipv6:
                        std::cout << "ipv6 ";
                        break;
                    default:
                        std::cout << "unknown protocol: " << (int) a.family();
                }
                std::cout << "\n";
            }

            std::cout << "Sending datagrams to " << address.to_string() << " on port "
                      << address.get_port() << "\n";

            // Create an unbound udp socket using ipv4. it can send and receive messages, but you
            // shouldn't really expect to read any messages until you send some because the port is
            // not bound (outside world doesn't know where to find it).
            cmd::udp_socket sender{cmd::inet_family::ipv4};

            std::string line;
            while (true) {
                std::getline(std::cin, line);
                if (std::cin.eof())
                    break;
                ret = sender.send(address, line);
                std::cout << "sent " << ret << " bytes\n";
                if (ret == -1) {
                    perror("send failed");
                    break;
                }
                // Read and ignore who sent it
                ret = sender.recv(buffer, sizeof(buffer));
                std::cout << "read back " << ret << " bytes:\n";
                if (ret >= 0) {
                    std::cout.write(buffer, ret);
                    std::cout << "\n";
                } else {
                    perror("read failed");
                }
            }
        }
    } catch (std::exception &e) {
        std::cerr << "Exception : " << e.what() << "\n";
    }
}
