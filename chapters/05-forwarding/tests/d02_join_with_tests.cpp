#include "d02_join_with.h"

#include <string>

#include <gtest/gtest.h>

namespace
{

TEST(JoinWith, PutsTheSeparatorBetweenTheItems)
{
    EXPECT_EQ(ch05::join_with(", ", "red", "green", "blue"), "red, green, blue");
}

TEST(JoinWith, AcceptsItemsOfDifferentTypes)
{
    const std::string name = "port";
    EXPECT_EQ(ch05::join_with(" ", name, '=', 8080, "ratio", 0.5), "port = 8080 ratio 0.5");
}

TEST(JoinWith, OneItemHasNoSeparator)
{
    EXPECT_EQ(ch05::join_with(", ", 42), "42");
}

TEST(JoinWith, NoItemsGiveAnEmptyText)
{
    EXPECT_EQ(ch05::join_with(", "), "");
}

TEST(JoinWith, AnEmptySeparatorIsFine)
{
    EXPECT_EQ(ch05::join_with("", 1, 2, 3), "123");
}

}  // namespace
