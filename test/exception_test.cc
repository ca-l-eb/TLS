#include <gtest/gtest.h>

#include "exceptions.h"

TEST(Exception, HttpResponseCaughtByException)
{
    try {
        throw cmd::http_response_exception{"no http response"};
    } catch (std::exception &e) {
        ASSERT_STREQ("no http response", e.what());
    } catch (...) {
        FAIL();
    }
}

TEST(Exception, HttpResponseCaughtByRuntimeError)
{
    try {
        throw cmd::http_response_exception{"no http response"};
    } catch (std::runtime_error &e) {
        ASSERT_STREQ("no http response", e.what());
    } catch (...) {
        FAIL();
    }
}
TEST(Exception, HttpResponseCatchesItself)
{
    try {
        throw cmd::http_response_exception{"no http response"};
    } catch (cmd::http_response_exception &e) {
        ASSERT_STREQ("no http response", e.what());
    } catch (...) {
        FAIL();
    }
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
