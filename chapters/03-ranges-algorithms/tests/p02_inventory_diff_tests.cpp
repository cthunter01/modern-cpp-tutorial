#include "p02_inventory_diff.h"

#include <string>
#include <vector>

#include <gtest/gtest.h>

// The tests fix: a struct ch03::Item with members sku and quantity, in that order; and a function
// ch03::diff(before, after), called with two std::vector<Item>, whose result has three members
// named added, removed and changed. Each of them can be iterated over and yields SKUs.

namespace
{

template <typename Skus>
std::vector<std::string> strings(const Skus& skus)
{
    std::vector<std::string> result;
    for (const auto& sku : skus)
    {
        result.emplace_back(sku);  // NOLINT(performance-inefficient-vector-operation)
    }
    return result;
}

using Strings = std::vector<std::string>;

TEST(InventoryDiff, ReportsAddedRemovedAndChanged)
{
    const std::vector<ch03::Item> before{
        {.sku = "pear", .quantity = 4},
        {.sku = "apple", .quantity = 10},
        {.sku = "plum", .quantity = 7},
        {.sku = "fig", .quantity = 1},
    };
    const std::vector<ch03::Item> after{
        {.sku = "kiwi", .quantity = 3},
        {.sku = "plum", .quantity = 9},
        {.sku = "apple", .quantity = 10},
        {.sku = "date", .quantity = 2},
    };

    const auto result = ch03::diff(before, after);

    EXPECT_EQ(strings(result.added), (Strings{"date", "kiwi"}));
    EXPECT_EQ(strings(result.removed), (Strings{"fig", "pear"}));
    EXPECT_EQ(strings(result.changed), (Strings{"plum"}));
}

TEST(InventoryDiff, IdenticalInventoriesInADifferentOrderDoNotDiffer)
{
    const std::vector<ch03::Item> before{{.sku = "b", .quantity = 2}, {.sku = "a", .quantity = 1}};
    const std::vector<ch03::Item> after{{.sku = "a", .quantity = 1}, {.sku = "b", .quantity = 2}};

    const auto result = ch03::diff(before, after);

    EXPECT_TRUE(strings(result.added).empty());
    EXPECT_TRUE(strings(result.removed).empty());
    EXPECT_TRUE(strings(result.changed).empty());
}

TEST(InventoryDiff, AgainstAnEmptyInventory)
{
    const std::vector<ch03::Item> stock{{.sku = "b", .quantity = 2}, {.sku = "a", .quantity = 1}};

    EXPECT_EQ(strings(ch03::diff({}, stock).added), (Strings{"a", "b"}));
    EXPECT_EQ(strings(ch03::diff(stock, {}).removed), (Strings{"a", "b"}));
}

TEST(InventoryDiff, IsFastEnoughForARealWarehouse)
{
    // 100 000 SKUs each, in opposite orders. A solution that searches one inventory for every item
    // of the other makes ten thousand million comparisons and runs into this test's time limit.
    constexpr int           kItems = 100'000;
    std::vector<ch03::Item> before;
    std::vector<ch03::Item> after;
    for (int i = 0; i < kItems; ++i)
    {
        before.push_back({.sku = "sku-" + std::to_string(i), .quantity = i});
        after.push_back({.sku = "sku-" + std::to_string(kItems - i), .quantity = kItems - i});
    }

    const auto result = ch03::diff(before, after);

    EXPECT_EQ(strings(result.added), (Strings{"sku-100000"}));
    EXPECT_EQ(strings(result.removed), (Strings{"sku-0"}));
    EXPECT_TRUE(strings(result.changed).empty());
}

}  // namespace
