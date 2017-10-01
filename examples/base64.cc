#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "base64.h"

int main(int argc, char *argv[])
{
    using namespace std::chrono;

    for (int i = 1; i < argc; i++) {
        auto t1 = high_resolution_clock::now();
        int len = std::strlen(argv[i]);
        auto t2 = high_resolution_clock::now();
        std::string encoded11 = cmd::base64::encode(argv[i], len);
        auto t3 = high_resolution_clock::now();
        std::string encoded12 = cmd::base64::encode(argv[i], len);
        auto t4 = high_resolution_clock::now();

        auto decode2 = cmd::base64::decode(encoded11);
        auto t5 = high_resolution_clock::now();

        auto decoded = cmd::base64::decode(encoded11);
        auto t7 = high_resolution_clock::now();
        std::string decoded_str{decoded.begin(), decoded.end()};
        auto t8 = high_resolution_clock::now();


        std::cout << encoded11 << "\n";
        std::cout << std::setw(10) << "strlen:" << std::setw(10)
                  << duration_cast<nanoseconds>(t2 - t1).count() << "ns\n";
        std::cout << std::setw(10) << "encode:" << std::setw(10)
                  << duration_cast<nanoseconds>(t3 - t2).count() << "ns\n";
        std::cout << std::setw(10) << "encode:" << std::setw(10)
                  << duration_cast<nanoseconds>(t4 - t3).count() << "ns\n";

        std::cout << std::setw(10) << "decode:" << std::setw(10)
                  << duration_cast<nanoseconds>(t5 - t4).count() << "ns\n";
        std::cout << std::setw(10) << "decode:" << std::setw(10)
                  << duration_cast<nanoseconds>(t7 - t5).count() << "ns\n";
        std::cout << std::setw(10) << "convert:" << std::setw(10)
                  << duration_cast<nanoseconds>(t8 - t7).count() << "ns\n";
        std::cout << std::setfill('-') << std::setw(25) << std::endl;
        std::cout << std::setfill(' ') << std::setw(0);
    }

    int len = 0;
    int read = 0;
    std::vector<int> read_len;
    while (std::cin.good()) {
        char buf[60];
        std::cin.read(buf, sizeof(buf));
        int good = std::cin.gcount();
        auto s = cmd::base64::encode(buf, good);
        std::cout << s;
        len += s.size();
        read += good;
        read_len.push_back(good);
    }
    std::cout << "\n";

    for (auto i : read_len)
        std::cout << i << " ";
    std::cout << "\n";
}
