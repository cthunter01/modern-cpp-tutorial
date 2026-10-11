#include "d01_trim.h"

#include <string>
#include <string_view>

#include <gtest/gtest.h>

namespace
{

TEST(Trim, RemovesBlanksAtBothEnds)
{
    EXPECT_EQ(ch01::trim("  hello  "), "hello");
    EXPECT_EQ(ch01::trim("\t hello\r\n"), "hello");
}

TEST(Trim, KeepsBlanksInTheMiddle)
{
    EXPECT_EQ(ch01::trim("  two words "), "two words");
}

TEST(Trim, LeavesATrimmedTextAsItIs)
{
    EXPECT_EQ(ch01::trim("hello"), "hello");
    EXPECT_EQ(ch01::trim("x"), "x");
}

TEST(Trim, NothingButBlanksGivesAnEmptyText)
{
    EXPECT_TRUE(ch01::trim("   \t ").empty());
    EXPECT_TRUE(ch01::trim("").empty());
}

TEST(Trim, TheResultIsAPartOfTheInput)
{
    const std::string      owner = "   padded text   ";
    const std::string_view text  = owner;

    const std::string_view trimmed = ch01::trim(text);

    // Not a copy somewhere else: the very characters of `owner`, three in from its start.
    EXPECT_EQ(trimmed.data(), &text[3]);
    EXPECT_EQ(trimmed.size(), 11U);
}

}  // namespace
