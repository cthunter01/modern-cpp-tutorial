#include "d02_split.h"

#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

namespace
{

using Pieces = std::vector<std::string_view>;

TEST(Split, SeparatesAtTheDelimiter)
{
    EXPECT_EQ(ch01::split("red,green,blue", ','), (Pieces{"red", "green", "blue"}));
}

TEST(Split, ATextWithoutDelimiterIsOnePiece)
{
    EXPECT_EQ(ch01::split("red", ','), (Pieces{"red"}));
}

TEST(Split, KeepsEmptyPieces)
{
    EXPECT_EQ(ch01::split("a,,b", ','), (Pieces{"a", "", "b"}));
    EXPECT_EQ(ch01::split(",a,", ','), (Pieces{"", "a", ""}));
    EXPECT_EQ(ch01::split("", ','), (Pieces{""}));
}

TEST(Split, ThePiecesArePartsOfTheInput)
{
    const std::string owner = "alpha;beta;gamma";

    const Pieces pieces = ch01::split(owner, ';');

    ASSERT_EQ(pieces.size(), 3U);
    EXPECT_EQ(pieces[0].data(), owner.data());
    EXPECT_EQ(pieces[1].data(), &owner[6]);
    EXPECT_EQ(pieces[2].data(), &owner[11]);
}

TEST(Split, WorksOnCharactersThatAreNotFollowedByATerminator)
{
    // Five characters and nothing behind them: no '\0'. Under the asan preset, a split() that walks
    // on until it finds one is reported.
    const std::vector<char> raw{'a', ',', 'b', ',', 'c'};
    const std::string_view  text(raw.data(), raw.size());

    EXPECT_EQ(ch01::split(text, ','), (Pieces{"a", "b", "c"}));
}

TEST(Split, WorksOnAPartOfALargerText)
{
    const std::string_view line = "key=a|b|c;rest";
    const std::string_view list = line.substr(4, 5);  // "a|b|c", followed by ";rest", not by '\0'

    EXPECT_EQ(ch01::split(list, '|'), (Pieces{"a", "b", "c"}));
}

}  // namespace
