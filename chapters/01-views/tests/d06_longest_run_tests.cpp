#include "d06_longest_run.h"

#include <span>
#include <vector>

#include <gtest/gtest.h>

namespace
{

TEST(LongestRun, FindsTheLongestStretchOfEqualNeighbours)
{
    const std::vector<int> values{1, 1, 2, 2, 2, 3};

    const std::span<const int> run = ch01::longest_run(values);

    ASSERT_EQ(run.size(), 3U);
    EXPECT_EQ(run.front(), 2);
    EXPECT_EQ(run.data(), &values[2]);  // a part of `values`, not a copy
}

TEST(LongestRun, TheFirstOfEquallyLongRunsWins)
{
    const std::vector<int> values{5, 5, 7, 7, 9};

    const std::span<const int> run = ch01::longest_run(values);

    ASSERT_EQ(run.size(), 2U);
    EXPECT_EQ(run.data(), values.data());
}

TEST(LongestRun, ARunAtTheVeryEndIsFound)
{
    const std::vector<int> values{1, 2, 3, 3, 3, 3};

    const std::span<const int> run = ch01::longest_run(values);

    ASSERT_EQ(run.size(), 4U);
    EXPECT_EQ(run.data(), &values[2]);
}

TEST(LongestRun, ValuesThatAllDifferGiveARunOfOne)
{
    const std::vector<int> values{4, 5, 6};

    const std::span<const int> run = ch01::longest_run(values);

    ASSERT_EQ(run.size(), 1U);
    EXPECT_EQ(run.data(), values.data());
}

TEST(LongestRun, NothingInNothingOut)
{
    EXPECT_TRUE(ch01::longest_run({}).empty());
}

}  // namespace
