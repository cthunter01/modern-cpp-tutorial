#include "d03_parse_bool.h"

#include <optional>

#include <gtest/gtest.h>

namespace
{

TEST(ParseBool, ReadsTrue)
{
    EXPECT_EQ(ch02::parse_bool("true"), true);
    EXPECT_EQ(ch02::parse_bool("on"), true);
}

TEST(ParseBool, FalseIsAnAnswer)
{
    for (const char* const text : {"false", "off"})
    {
        const std::optional<bool> parsed = ch02::parse_bool(text);

        // Two different questions. Is there an answer? Yes. What is it? false.
        EXPECT_TRUE(parsed.has_value()) << text;
        EXPECT_EQ(parsed, false) << text;
    }
}

TEST(ParseBool, AnythingElseIsNoAnswer)
{
    EXPECT_EQ(ch02::parse_bool("maybe"), std::nullopt);
    EXPECT_EQ(ch02::parse_bool(""), std::nullopt);
    EXPECT_EQ(ch02::parse_bool("TRUE"), std::nullopt);
    EXPECT_EQ(ch02::parse_bool("true "), std::nullopt);
}

}  // namespace
