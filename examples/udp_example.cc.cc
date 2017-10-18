#include <chrono>
#include <thread>
#include <iostream>
#include <csignal>

#include "udp_socket.h"

int main(int argc, char *argv[]) {
    try {
        auto local = cmd::address_list{5000};
        cmd::bound_udp_socket listener{local};
        cmd::udp_socket sender;

        char buffer[1024];
        ssize_t ret;
        while (true) {
            ret = sender.send(listener.get_address(), "hello world", 0);
            std::cout << "Send: " << ret << "\n";
            if (ret < 0)
                break;
            ret = listener.recv(buffer, sizeof(buffer), 0);
            std::cout << "Received " << ret;
            if (ret > 0) {
                buffer[ret] = '\0';
                std::cout << " " << buffer;
            }
            std::cout << "\n";
            std::this_thread::sleep_for(std::chrono::seconds{5});
        }
    }
    catch (std::exception &e) {
        std::cerr << "Exception : " << e.what() << "\n";
    }
}
