#include <iostream>
#include <map>
#include "inet_addr.h"

static std::map<cmd::inet_family, std::string> family_to_string{
    {cmd::inet_family::unspecified, "unspecified"},
    {cmd::inet_family::ipv4, "ipv4"},
    {cmd::inet_family::ipv6, "ipv6"}};

static std::map<cmd::inet_proto, std::string> proto_to_string{{cmd::inet_proto::tcp, "tcp"},
                                                              {cmd::inet_proto::udp, "udp"}};

void print_info(cmd::inet_addr &addr)
{
    std::cout << "[ ] " << addr.to_string() << " ";
    auto family = family_to_string[addr.family()];
    auto proto = proto_to_string[addr.protocol()];
    if (!family.empty())
        std::cout << family << " ";

    if (!proto.empty())
        std::cout << proto << " ";
    std::cout << "\n";
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        std::cout << "[+] " << argv[i] << "\n";
        try {
            cmd::inet_resolver tcp_options{argv[i], 0, cmd::inet_proto::tcp,
                                           cmd::inet_family::unspecified};
            for (auto &addr : tcp_options.addresses)
                print_info(addr);

            cmd::inet_resolver udp_options{argv[i], 0, cmd::inet_proto::udp,
                                           cmd::inet_family::unspecified};
            for (auto &addr : udp_options.addresses)
                print_info(addr);

            std::cout << "\n";
        } catch (std::exception &e) {
            std::cerr << "Exception: " << e.what() << "\n";
        }
    }
}
