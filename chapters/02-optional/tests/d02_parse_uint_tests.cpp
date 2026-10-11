#include "d02_parse_uint.h"

#include <optional>

#include <gtest/gtest.h>

namespace
{

TEST(ParseUint, ReadsDecimalNumbers)
{
    EXPECT_EQ(ch02::parse_uint("8080"), 8080U);
    EXPECT_EQ(ch02::parse_uint("7"), 7U);
    EXPECT_EQ(ch02::parse_uint("007"), 7U);
}

TEST(ParseUint, ZeroIsANumber)
{
    const std::optional<unsigned> zero = ch02::parse_uint("0");
    EXPECT_TRUE(zero.has_value());
    EXPECT_EQ(zero, 0U);
}

TEST(ParseUint, ReadsTheLargestValue)
{
    EXPECT_EQ(ch02::parse_uint("4294967295"), 4294967295U);
}

TEST(ParseUint, RejectsWhatIsNotANumber)
{
    EXPECT_EQ(ch02::parse_uint(""), std::nullopt);
    EXPECT_EQ(ch02::parse_uint("abc"), std::nullopt);
    EXPECT_EQ(ch02::parse_uint("-1"), std::nullopt);
    EXPECT_EQ(ch02::parse_uint("+1"), std::nullopt);
}

TEST(ParseUint, RejectsANumberWithSomethingAroundIt)
{
    EXPECT_EQ(ch02::parse_uint("12x"), std::nullopt);
    EXPECT_EQ(ch02::parse_uint(" 12"), std::nullopt);
    EXPECT_EQ(ch02::parse_uint("12 "), std::nullopt);
    EXPECT_EQ(ch02::parse_uint("1.5"), std::nullopt);
}

TEST(ParseUint, RejectsANumberThatDoesNotFit)
{
    EXPECT_EQ(ch02::parse_uint("4294967296"), std::nullopt);
    EXPECT_EQ(ch02::parse_uint("99999999999999999999"), std::nullopt);
}

}  // namespace
