#include "d02_first_repeat.h"

#include <gtest/gtest.h>

namespace
{

TEST(FirstRepeat, ReportsTheValueThatRepeatsFirst)
{
    // 1 is seen for the second time before 3 is.
    EXPECT_EQ(ch00::first_repeat({3, 1, 4, 1, 5, 3}), 1);
}

TEST(FirstRepeat, NeighboursCount)
{
    EXPECT_EQ(ch00::first_repeat({7, 7}), 7);
}

TEST(FirstRepeat, NoRepeatGivesMinusOne)
{
    EXPECT_EQ(ch00::first_repeat({1, 2, 3}), -1);
    EXPECT_EQ(ch00::first_repeat({}), -1);
}

}  // namespace
