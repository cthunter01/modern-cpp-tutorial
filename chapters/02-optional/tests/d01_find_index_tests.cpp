#include "d01_find_index.h"

#include <cstddef>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

namespace
{

TEST(FindIndex, FindsAnElement)
{
    const std::vector<int> values{4, 8, 15, 16, 23, 42};
    EXPECT_EQ(ch02::find_index(values, 15), 2U);
}

TEST(FindIndex, AHitAtTheFrontIsAHit)
{
    const std::vector<int> values{4, 8, 15};

    const std::optional<std::size_t> found = ch02::find_index(values, 4);

    // Index 0 is an answer. `if (found)` asks whether there is one, not whether it is non-zero.
    EXPECT_TRUE(found.has_value());
    EXPECT_EQ(found, 0U);
}

TEST(FindIndex, ReportsTheFirstOfSeveral)
{
    const std::vector<int> values{7, 3, 7, 7};
    EXPECT_EQ(ch02::find_index(values, 7), 0U);
    EXPECT_EQ(ch02::find_index(values, 3), 1U);
}

TEST(FindIndex, AMissGivesNothing)
{
    const std::vector<int> values{4, 8, 15};
    EXPECT_EQ(ch02::find_index(values, 99), std::nullopt);
    EXPECT_EQ(ch02::find_index({}, 0), std::nullopt);
}

}  // namespace
