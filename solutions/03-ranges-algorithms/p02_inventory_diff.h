#pragma once

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

namespace ch03
{

struct Item
{
    std::string sku;
    int         quantity = 0;
};

struct InventoryDiff
{
    std::vector<std::string> added;    ///< in `after` only
    std::vector<std::string> removed;  ///< in `before` only
    std::vector<std::string> changed;  ///< in both, with different quantities
};

/// What changed between two inventories. Each list of SKUs is sorted. Neither inventory has to be
/// sorted, and neither contains a SKU twice.
[[nodiscard]] inline InventoryDiff diff(std::vector<Item> before, std::vector<Item> after)
{
    //! [diff]
    // Sorted by SKU, the two inventories can be compared in one pass each.
    std::ranges::sort(before, {}, &Item::sku);
    std::ranges::sort(after, {}, &Item::sku);

    InventoryDiff result;
    // "The elements of the first range that are not in the second", compared by SKU on both sides.
    // The output is a sequence of Items; the transform below keeps only the SKU of each.
    std::vector<Item> onlyAfter;
    std::vector<Item> onlyBefore;
    std::ranges::set_difference(after, before, std::back_inserter(onlyAfter), {}, &Item::sku,
                                &Item::sku);
    std::ranges::set_difference(before, after, std::back_inserter(onlyBefore), {}, &Item::sku,
                                &Item::sku);
    std::ranges::transform(onlyAfter, std::back_inserter(result.added), &Item::sku);
    std::ranges::transform(onlyBefore, std::back_inserter(result.removed), &Item::sku);
    //! [diff]

    //! [changed]
    // For "in both, but different" no algorithm fits exactly: walk the two sorted sequences
    // together.
    auto left  = before.begin();
    auto right = after.begin();
    while (left != before.end() && right != after.end())
    {
        if (left->sku < right->sku)
        {
            ++left;
        }
        else if (right->sku < left->sku)
        {
            ++right;
        }
        else
        {
            if (left->quantity != right->quantity)
            {
                result.changed.push_back(left->sku);
            }
            ++left;
            ++right;
        }
    }
    //! [changed]
    return result;
}

}  // namespace ch03
