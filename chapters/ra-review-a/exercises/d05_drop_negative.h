#pragma once

#include <algorithm>
#include <vector>

namespace review_a
{

/// Removes every negative number from @p values. The others keep their order.
inline void drop_negative(std::vector<int>& values)
{
    values.erase(std::ranges::remove_if(values, [](int value) { return value < 0; }).begin());
}

}  // namespace review_a
