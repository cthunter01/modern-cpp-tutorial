#include "d01_min_max.h"

#include <vector>

#include <gtest/gtest.h>

namespace
{

TEST(MinMax, FindsBothEndsInOnePass)
{
    const auto [low, high] = ch00::min_max({3, -7, 12, 0, 5});
    EXPECT_EQ(low, -7);
    EXPECT_EQ(high, 12);
}

TEST(MinMax, OneValueIsBothEnds)
{
    const auto [low, high] = ch00::min_max({42});
    EXPECT_EQ(low, 42);
    EXPECT_EQ(high, 42);
}

TEST(MinMax, EmptyGivesZeroes)
{
    const ch00::Range range = ch00::min_max({});
    EXPECT_EQ(range.min, 0);
    EXPECT_EQ(range.max, 0);
}

}  // namespace
