#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <gtest/gtest.h>

#include "base64.h"

TEST(base64, encode_hello_world)
{
    std::string encoded = cmd::base64::encode("Hello, world");
    ASSERT_EQ(encoded, "SGVsbG8sIHdvcmxk");
}

TEST(base64, encode_and_decode)
{
    std::string s{"Random text to be encoded and decoded"};
    std::string encoded = cmd::base64::encode(s);
    auto d = cmd::base64::decode(encoded);
    std::string decoded{d.begin(), d.end()};
    ASSERT_EQ(s.size(), decoded.size());
    ASSERT_EQ(s.size(), d.size());
    ASSERT_EQ(decoded, s);
}

TEST(base64, encode_and_decode_random)
{
    unsigned char rand[32];
    std::ifstream ifs;
    ifs.open("/dev/urandom");
    ifs.read((char*) rand, sizeof(rand));

    EXPECT_EQ(ifs.gcount(), sizeof(rand));

    auto encoded = cmd::base64::encode((char*) rand, sizeof(rand));
    auto decoded = cmd::base64::decode(encoded);

    ASSERT_EQ(sizeof(rand), decoded.size());
    for (int i = 0; i < sizeof(rand); i++) {
        ASSERT_EQ(rand[i], decoded[i]);
    }
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
