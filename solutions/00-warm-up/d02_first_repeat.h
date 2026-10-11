#pragma once

#include <set>
#include <vector>

namespace ch00
{

/// The first value that turns up for the second time, or -1 when no value repeats.
[[nodiscard]] inline int first_repeat(const std::vector<int>& values)
{
    std::set<int> seen;
    for (const int value : values)
    {
        // insert() returns {iterator, bool}. Both names exist only inside this if.
        if (const auto [position, isNew] = seen.insert(value); !isNew)
        {
            return *position;
        }
    }
    return -1;
}

}  // namespace ch00
