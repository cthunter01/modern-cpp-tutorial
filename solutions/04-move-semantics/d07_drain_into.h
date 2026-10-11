#pragma once

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

namespace ch04
{

/// Appends every string of @p from to @p to without copying its characters, and leaves @p from
/// empty.
inline void drain_into(std::vector<std::string>& from, std::vector<std::string>& to)
{
    to.reserve(to.size() + from.size());
    std::ranges::move(from,
                      std::back_inserter(to));  // the algorithm, not the cast: moves each element
    from.clear();                               // the strings are moved-from; now they are gone
}

}  // namespace ch04
