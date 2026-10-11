#include "d05_drop_blank.h"

#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace
{

TEST(DropBlank, RemovesEmptyAndBlankLines)
{
    std::vector<std::string> lines{"first", "", "second", "   ", "\t", "third"};

    ch03::drop_blank(lines);

    EXPECT_EQ(lines, (std::vector<std::string>{"first", "second", "third"}));
}

TEST(DropBlank, TheVectorReallyGetsShorter)
{
    std::vector<std::string> lines{"", "kept", "", ""};

    ch03::drop_blank(lines);

    // Not four elements of which three are leftovers: one element.
    EXPECT_EQ(lines.size(), 1U);
}

TEST(DropBlank, ALineWithTextInItIsNotBlank)
{
    std::vector<std::string> lines{"  indented", "trailing  ", " x "};

    ch03::drop_blank(lines);

    EXPECT_EQ(lines.size(), 3U);
}

TEST(DropBlank, NothingButBlankLinesLeavesNothing)
{
    std::vector<std::string> lines{"", " ", "\t\t"};
    ch03::drop_blank(lines);
    EXPECT_TRUE(lines.empty());

    std::vector<std::string> none;
    ch03::drop_blank(none);
    EXPECT_TRUE(none.empty());
}

}  // namespace
