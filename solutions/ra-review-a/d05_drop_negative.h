#pragma once

#include <vector>

namespace review_a
{

/// Removes every negative number from @p values. The others keep their order.
inline void drop_negative(std::vector<int>& values)
{
    // The bug called erase() with one iterator, which erases one element. remove_if returns the
    // whole leftover part, and all of it has to go: erase(leftover.begin(), leftover.end()). Or let
    // the container do both steps.
    std::erase_if(values, [](int value) { return value < 0; });
}

}  // namespace review_a
