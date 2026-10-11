#include "d01_count_and_sum.h"

#include <string>

#include <gtest/gtest.h>

namespace
{

TEST(CountArgs, CountsWhateverItIsGiven)
{
    const std::string text = "text";
    EXPECT_EQ(ch05::count_args(), 0U);
    EXPECT_EQ(ch05::count_args(1), 1U);
    EXPECT_EQ(ch05::count_args(1, 2.5, 'c', text, nullptr), 5U);
}

TEST(SumAll, AddsThemUp)
{
    EXPECT_EQ(ch05::sum_all(1, 2, 3, 4), 10);
    EXPECT_EQ(ch05::sum_all(7), 7);
}

TEST(SumAll, OfNothingIsZero)
{
    EXPECT_EQ(ch05::sum_all(), 0);
}

TEST(SumAll, MixedTypesAddAsTheyWouldByHand)
{
    EXPECT_DOUBLE_EQ(ch05::sum_all(1, 2.5, 3), 6.5);
}

}  // namespace
