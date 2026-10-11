#pragma once

#include <cstddef>
#include <vector>

#include "tut/tracked.h"

namespace ch04
{

// This class works, and copies every item it is given, including the ones the caller was giving up.
// Change Add so that such a caller pays for no copy, and a caller who keeps their item pays for
// one.

class Registry
{
public:
    /// Keeps @p item.
    void Add(const tut::Tracked& item) { items.push_back(item); }

    [[nodiscard]] std::size_t Size() const noexcept { return items.size(); }
    [[nodiscard]] int         ValueAt(std::size_t index) const { return items.at(index).Value(); }

private:
    std::vector<tut::Tracked> items;
};

}  // namespace ch04
