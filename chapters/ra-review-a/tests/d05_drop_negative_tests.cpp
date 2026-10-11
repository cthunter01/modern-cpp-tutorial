#include "d05_drop_negative.h"

#include <vector>

#include <gtest/gtest.h>

namespace
{

TEST(DropNegative, RemovesOneNegativeNumber)
{
    std::vector<int> values{1, -2, 3};
    review_a::drop_negative(values);
    EXPECT_EQ(values, (std::vector<int>{1, 3}));
}

TEST(DropNegative, RemovesAllOfThem)
{
    std::vector<int> values{1, -2, 3, -4, 5, -6};
    review_a::drop_negative(values);
    EXPECT_EQ(values, (std::vector<int>{1, 3, 5}));
}

TEST(DropNegative, NothingButNegativeNumbersLeavesNothing)
{
    std::vector<int> values{-1, -2, -3};
    review_a::drop_negative(values);
    EXPECT_TRUE(values.empty());
}

}  // namespace
