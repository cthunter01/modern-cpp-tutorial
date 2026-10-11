#include "d04_use_colour.h"

#include <optional>

#include <gtest/gtest.h>

namespace
{

TEST(UseColour, WithoutASettingTheTerminalDecides)
{
    EXPECT_TRUE(review_a::use_colour(std::nullopt, true));
    EXPECT_FALSE(review_a::use_colour(std::nullopt, false));
}

TEST(UseColour, ASettingWinsOverTheTerminal)
{
    EXPECT_TRUE(review_a::use_colour(true, false));
    EXPECT_FALSE(review_a::use_colour(false, true)) << "the user switched colour off";
    EXPECT_FALSE(review_a::use_colour(false, false));
}

}  // namespace
