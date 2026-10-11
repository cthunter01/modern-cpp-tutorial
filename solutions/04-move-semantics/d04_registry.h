#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include "tut/tracked.h"

namespace ch04
{

class Registry
{
public:
    //! [add]
    /// Keeps @p item. By value: the caller decides what that costs. An lvalue argument is copied
    /// into `item`, an rvalue argument is moved into it, and from there it is moved into the
    /// vector.
    void Add(tut::Tracked item) { items.push_back(std::move(item)); }
    //! [add]

    [[nodiscard]] std::size_t Size() const noexcept { return items.size(); }
    [[nodiscard]] int         ValueAt(std::size_t index) const { return items.at(index).Value(); }

private:
    std::vector<tut::Tracked> items;
};

}  // namespace ch04
