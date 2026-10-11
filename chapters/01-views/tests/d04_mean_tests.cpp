#include "d04_mean.h"

#include <array>
#include <span>
#include <vector>

#include <gtest/gtest.h>

// One function, one parameter type, and every way a program keeps doubles next to each other.

namespace
{

TEST(Mean, OfAVector)
{
    const std::vector<double> values{1.0, 2.0, 3.0, 6.0};
    EXPECT_DOUBLE_EQ(ch01::mean(values), 3.0);
}

TEST(Mean, OfAStdArray)
{
    const std::array<double, 3> values{10.0, 20.0, 60.0};
    EXPECT_DOUBLE_EQ(ch01::mean(values), 30.0);
}

TEST(Mean, OfABuiltInArray)
{
    const double values[] = {0.5, 1.5};  // NOLINT(*-avoid-c-arrays): the point of the test
    EXPECT_DOUBLE_EQ(ch01::mean(values), 1.0);
}

TEST(Mean, OfOneObject)
{
    const double single = 7.25;
    EXPECT_DOUBLE_EQ(ch01::mean({&single, 1}), 7.25);
}

TEST(Mean, OfAPartOfAVector)
{
    const std::vector<double> values{100.0, 1.0, 2.0, 3.0, 100.0};
    EXPECT_DOUBLE_EQ(ch01::mean(std::span(values).subspan(1, 3)), 2.0);
}

TEST(Mean, OfNothingIsZero)
{
    EXPECT_DOUBLE_EQ(ch01::mean({}), 0.0);
}

}  // namespace
