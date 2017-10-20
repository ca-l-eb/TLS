#include <gtest/gtest.h>

#include "resource_parser.h"

TEST(ResourceParser, NormalUrl)
{
    std::string url{"http://www.example.com/index.html"};
    std::string proto, host, resource;
    int port;
    try {
        std::tie(proto, host, port, resource) = cmd::resource_parser::parse(url);
        EXPECT_EQ(proto, "http");
        EXPECT_EQ(host, "www.example.com");
        EXPECT_EQ(port, 80);
        EXPECT_EQ(resource, "/index.html");
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        FAIL();
    }
}

TEST(ResourceParser, NoProtocol)
{
    std::string url{"www.example.com/index.html"};
    std::string proto, host, resource;
    int port;
    try {
        std::tie(proto, host, port, resource) = cmd::resource_parser::parse(url);
        EXPECT_EQ(proto, "");
        EXPECT_EQ(host, "www.example.com");
        EXPECT_EQ(port, -1);
        EXPECT_EQ(resource, "/index.html");
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        FAIL();
    }
}

TEST(ResourceParser, NoResource)
{
    std::string url{"www.example.com"};
    std::string proto, host, resource;
    int port;
    try {
        std::tie(proto, host, port, resource) = cmd::resource_parser::parse(url);
        EXPECT_EQ(proto, "");
        EXPECT_EQ(host, "www.example.com");
        EXPECT_EQ(port, -1);
        EXPECT_EQ(resource, "/");
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        FAIL();
    }
}

TEST(ResourceParser, SpecificPort)
{
    std::string url{"www.example.com:443"};
    std::string proto, host, resource;
    int port;
    try {
        std::tie(proto, host, port, resource) = cmd::resource_parser::parse(url);
        EXPECT_EQ(proto, "");
        EXPECT_EQ(host, "www.example.com");
        EXPECT_EQ(port, 443);
        EXPECT_EQ(resource, "/");
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        FAIL();
    }
}

TEST(ResourceParser, PortWithResource)
{
    std::string url{"www.example.com:533/this-is-a%20random%20url"};
    std::string proto, host, resource;
    int port;
    try {
        std::tie(proto, host, port, resource) = cmd::resource_parser::parse(url);
        EXPECT_EQ(proto, "");
        EXPECT_EQ(host, "www.example.com");
        EXPECT_EQ(port, 533);
        EXPECT_EQ(resource, "/this-is-a%20random%20url");
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        FAIL();
    }
}

TEST(ResourceParser, WebSocketPort)
{
    std::string url{"ws://www.example.com/"};
    std::string proto, host, resource;
    int port;
    try {
        std::tie(proto, host, port, resource) = cmd::resource_parser::parse(url);
        EXPECT_EQ(proto, "ws");
        EXPECT_EQ(host, "www.example.com");
        EXPECT_EQ(port, 80);
        EXPECT_EQ(resource, "/");
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        FAIL();
    }
}

TEST(ResourceParser, WebSocketPortWithPortOverride)
{
    std::string url{"ws://www.example.com:783/"};
    std::string proto, host, resource;
    int port;
    try {
        std::tie(proto, host, port, resource) = cmd::resource_parser::parse(url);
        EXPECT_EQ(proto, "ws");
        EXPECT_EQ(host, "www.example.com");
        EXPECT_EQ(port, 783);
        EXPECT_EQ(resource, "/");
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        FAIL();
    }
}

TEST(ResourceParser, WebSocketSecure)
{
    std::string url{"wss://www.example.com/echo"};
    std::string proto, host, resource;
    int port;
    try {
        std::tie(proto, host, port, resource) = cmd::resource_parser::parse(url);
        EXPECT_EQ(proto, "wss");
        EXPECT_EQ(host, "www.example.com");
        EXPECT_EQ(port, 443);
        EXPECT_EQ(resource, "/echo");
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        FAIL();
    }
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
