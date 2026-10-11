#include "d07_drain_into.h"

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "tut/storage.h"

namespace
{

std::string long_text(char fill)
{
    std::string text(200, fill);
    return text;
}

TEST(DrainInto, MovesTheStringsAcross)
{
    std::vector<std::string> from{long_text('a'), long_text('b')};
    std::vector<std::string> to;
    const auto               first  = tut::storage_of(from[0]);
    const auto               second = tut::storage_of(from[1]);

    ch04::drain_into(from, to);

    ASSERT_EQ(to.size(), 2U);
    EXPECT_EQ(tut::storage_of(to[0]), first) << "the characters were copied";
    EXPECT_EQ(tut::storage_of(to[1]), second);
}

TEST(DrainInto, LeavesTheSourceEmpty)
{
    std::vector<std::string> from{long_text('a'), long_text('b'), long_text('c')};
    std::vector<std::string> to;

    ch04::drain_into(from, to);

    // Empty: not three strings that happen to have no characters left in them.
    EXPECT_TRUE(from.empty());
}

TEST(DrainInto, AppendsToWhatIsAlreadyThere)
{
    std::vector<std::string> from{long_text('c'), long_text('d')};
    std::vector<std::string> to{long_text('a'), long_text('b')};

    ch04::drain_into(from, to);

    ASSERT_EQ(to.size(), 4U);
    EXPECT_EQ(to[0].front(), 'a');
    EXPECT_EQ(to[1].front(), 'b');
    EXPECT_EQ(to[2].front(), 'c');
    EXPECT_EQ(to[3].front(), 'd');
}

TEST(DrainInto, NothingToDrain)
{
    std::vector<std::string> from;
    std::vector<std::string> to{long_text('a')};

    ch04::drain_into(from, to);

    EXPECT_EQ(to.size(), 1U);
    EXPECT_TRUE(from.empty());
}

}  // namespace
