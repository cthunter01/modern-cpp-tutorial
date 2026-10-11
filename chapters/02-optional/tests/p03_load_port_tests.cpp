#include "p03_load_port.h"

#include <gtest/gtest.h>

// The tests fix two functions. ch02::load_port(line) returns a result that answers `if (result)`
// and, when that is true, gives the port as `*result`. ch02::describe_failure(result) turns a
// result that is not a port into the message the user is shown. What the result type is, is yours
// to decide.

namespace
{

TEST(LoadPort, ReadsThePort)
{
    const auto result = ch02::load_port("port = 8080");

    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 8080U);
}

TEST(LoadPort, IgnoresBlanksAroundKeyAndValue)
{
    const auto tight = ch02::load_port("port=1");
    ASSERT_TRUE(tight);
    EXPECT_EQ(*tight, 1U);

    const auto loose = ch02::load_port("  port\t=   65535  ");
    ASSERT_TRUE(loose);
    EXPECT_EQ(*loose, 65535U);
}

TEST(LoadPort, SaysWhenTheLineIsNotAboutThePort)
{
    for (const char* const line : {"host = example.org", "port", "", "ports = 80", "# port = 80"})
    {
        const auto result = ch02::load_port(line);

        ASSERT_FALSE(result) << line;
        EXPECT_EQ(ch02::describe_failure(result), "this line does not set the port") << line;
    }
}

TEST(LoadPort, SaysWhenThePortIsNotANumber)
{
    for (const char* const line :
         {"port = http", "port =", "port = 80 81", "port = -1", "port = 8O80"})
    {
        const auto result = ch02::load_port(line);

        ASSERT_FALSE(result) << line;
        EXPECT_EQ(ch02::describe_failure(result), "the port must be a number") << line;
    }
}

TEST(LoadPort, SaysWhenThePortIsOutOfRange)
{
    for (const char* const line : {"port = 0", "port = 65536", "port = 4000000000"})
    {
        const auto result = ch02::load_port(line);

        ASSERT_FALSE(result) << line;
        EXPECT_EQ(ch02::describe_failure(result), "the port must be between 1 and 65535") << line;
    }
}

}  // namespace
