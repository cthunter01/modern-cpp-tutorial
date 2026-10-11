#pragma once

#include <algorithm>
#include <vector>

namespace ch00
{

struct Range
{
    int min = 0;
    int max = 0;
};

/// The smallest and the largest value. For an empty vector both are 0.
[[nodiscard]] inline Range min_max(const std::vector<int>& values)
{
    if (values.empty())
    {
        return {};
    }
    // minmax_element returns both iterators in one object: a structured binding names them at once.
    // std::minmax_element(values.begin(), values.end()) does the same; the std::ranges form takes
    // the container itself (chapter 3).
    const auto [smallest, largest] = std::ranges::minmax_element(values);
    return {.min = *smallest, .max = *largest};
}

}  // namespace ch00
