#include "d08_parse_endpoint.h"

#include <gtest/gtest.h>

namespace
{

using ch02::ParseError;

TEST(ParseEndpoint, ReadsHostAndPort)
{
    const auto endpoint = ch02::parse_endpoint("example.org:8080");

    ASSERT_TRUE(endpoint.has_value());
    EXPECT_EQ(endpoint->host, "example.org");
    EXPECT_EQ(endpoint->port, 8080U);
}

TEST(ParseEndpoint, AcceptsTheLimitsOfThePortRange)
{
    EXPECT_EQ(ch02::parse_endpoint("a:1").value().port, 1U);
    EXPECT_EQ(ch02::parse_endpoint("a:65535").value().port, 65535U);
}

TEST(ParseEndpoint, SaysWhenTheColonIsMissing)
{
    const auto endpoint = ch02::parse_endpoint("example.org");

    ASSERT_FALSE(endpoint.has_value());
    EXPECT_EQ(endpoint.error(), ParseError::MissingColon);
}

TEST(ParseEndpoint, SaysWhenTheHostIsEmpty)
{
    const auto endpoint = ch02::parse_endpoint(":8080");

    ASSERT_FALSE(endpoint.has_value());
    EXPECT_EQ(endpoint.error(), ParseError::EmptyHost);
}

TEST(ParseEndpoint, SaysWhenThePortIsBad)
{
    for (const char* const text : {
             "example.org:",
             "example.org:http",
             "example.org:0",
             "example.org:65536",
             "example.org:-1",
             "example.org:80 ",
         })
    {
        const auto endpoint = ch02::parse_endpoint(text);

        ASSERT_FALSE(endpoint.has_value()) << text;
        EXPECT_EQ(endpoint.error(), ParseError::BadPort) << text;
    }
}

TEST(ParseEndpoint, TheLastColonSeparatesThePort)
{
    const auto endpoint = ch02::parse_endpoint("::1:443");

    ASSERT_TRUE(endpoint.has_value());
    EXPECT_EQ(endpoint->host, "::1");
    EXPECT_EQ(endpoint->port, 443U);
}

}  // namespace
