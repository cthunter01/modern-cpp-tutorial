#include "d06_first_out_of_order.h"

#include <optional>
#include <vector>

#include <gtest/gtest.h>

namespace
{

TEST(FirstOutOfOrder, SamplesInOrderGiveNothing)
{
    const std::vector<ch03::Sample> samples{
        {.timestamp = 10, .value = 5.0},
        {.timestamp = 20, .value = 1.0},
        {.timestamp = 30, .value = 3.0},
    };
    EXPECT_EQ(ch03::first_out_of_order(samples), std::nullopt);
}

TEST(FirstOutOfOrder, ItIsTheTimestampsThatCount)
{
    // The values go down; the timestamps do not. That is in order.
    const std::vector<ch03::Sample> samples{
        {.timestamp = 1, .value = 9.0},
        {.timestamp = 2, .value = 8.0},
        {.timestamp = 3, .value = 7.0},
    };
    EXPECT_EQ(ch03::first_out_of_order(samples), std::nullopt);
}

TEST(FirstOutOfOrder, ReportsTheFirstSampleThatGoesBackInTime)
{
    const std::vector<ch03::Sample> samples{
        {.timestamp = 10, .value = 0.0},
        {.timestamp = 20, .value = 0.0},
        {.timestamp = 15, .value = 0.0},
        {.timestamp = 5, .value = 0.0},
    };
    EXPECT_EQ(ch03::first_out_of_order(samples), 2U);
}

TEST(FirstOutOfOrder, EqualTimestampsAreInOrder)
{
    const std::vector<ch03::Sample> samples{
        {.timestamp = 10, .value = 0.0},
        {.timestamp = 10, .value = 1.0},
        {.timestamp = 10, .value = 2.0},
    };
    EXPECT_EQ(ch03::first_out_of_order(samples), std::nullopt);
}

TEST(FirstOutOfOrder, NoneAndOneAreInOrder)
{
    const std::vector<ch03::Sample> one{{.timestamp = 10, .value = 0.0}};
    EXPECT_EQ(ch03::first_out_of_order({}), std::nullopt);
    EXPECT_EQ(ch03::first_out_of_order(one), std::nullopt);
}

}  // namespace
