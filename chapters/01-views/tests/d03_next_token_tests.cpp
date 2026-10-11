#include "d03_next_token.h"

#include <string_view>

#include <gtest/gtest.h>

namespace
{

TEST(NextToken, TakesWordsOffTheFrontOneByOne)
{
    std::string_view rest = "  alpha beta\tgamma ";

    EXPECT_EQ(ch01::next_token(rest), "alpha");
    EXPECT_EQ(ch01::next_token(rest), "beta");
    EXPECT_EQ(ch01::next_token(rest), "gamma");
}

TEST(NextToken, LeavesTheRestDirectlyBehindTheWord)
{
    const std::string_view line = "copy  source target";
    std::string_view       rest = line;

    const std::string_view word = ch01::next_token(rest);

    EXPECT_EQ(word, "copy");
    EXPECT_EQ(rest, "  source target");
    EXPECT_EQ(rest.data(), &line[4]);
}

TEST(NextToken, GivesAnEmptyWordWhenNothingIsLeft)
{
    std::string_view rest = "only";

    EXPECT_EQ(ch01::next_token(rest), "only");
    EXPECT_TRUE(rest.empty());

    // Again and again: asking an empty text must stay harmless. (This build checks: a remove_prefix
    // with more than size() ends the program.)
    EXPECT_TRUE(ch01::next_token(rest).empty());
    EXPECT_TRUE(ch01::next_token(rest).empty());
    EXPECT_TRUE(rest.empty());
}

TEST(NextToken, BlanksOnlyGiveAnEmptyWordAndAnEmptyRest)
{
    std::string_view rest = " \t  ";

    EXPECT_TRUE(ch01::next_token(rest).empty());
    EXPECT_TRUE(rest.empty());
}

}  // namespace
