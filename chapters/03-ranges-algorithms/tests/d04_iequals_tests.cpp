#include "d04_iequals.h"

#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

namespace
{

TEST(IEquals, IgnoresTheCaseOfLetters)
{
    EXPECT_TRUE(ch03::iequals("warn", "WARN"));
    EXPECT_TRUE(ch03::iequals("WaRn", "wArN"));
    EXPECT_TRUE(ch03::iequals("", ""));
}

TEST(IEquals, DifferentTextsAreDifferent)
{
    EXPECT_FALSE(ch03::iequals("warn", "info"));
    EXPECT_FALSE(ch03::iequals("a-b", "a_b"));
}

TEST(IEquals, ABeginningIsNotTheWholeText)
{
    EXPECT_FALSE(ch03::iequals("war", "warn"));
    EXPECT_FALSE(ch03::iequals("warning", "warn"));
    EXPECT_FALSE(ch03::iequals("", "x"));
    EXPECT_FALSE(ch03::iequals("x", ""));
}

TEST(IEquals, DoesNotReadPastTheShorterText)
{
    // Three characters with nothing behind them. Under the asan preset, an iequals() that compares
    // as many characters as the longer text has is reported here.
    const std::vector<char> raw{'w', 'a', 'r'};
    const std::string_view  shorter(raw.data(), raw.size());

    EXPECT_FALSE(ch03::iequals("warning", shorter));
    EXPECT_FALSE(ch03::iequals(shorter, "warning"));
}

}  // namespace
