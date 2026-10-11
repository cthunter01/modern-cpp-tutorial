#pragma once

#include <cstddef>
#include <span>

namespace ch01
{

/// The longest stretch of equal neighbours in @p values; the first one when several are equally
/// long. The result is a part of @p values. Empty for an empty input.
[[nodiscard]] inline std::span<const int> longest_run(std::span<const int> values) noexcept
{
    std::size_t bestStart  = 0;
    std::size_t bestLength = 0;
    std::size_t start      = 0;
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (values[i] != values[start])
        {
            start = i;
        }
        const std::size_t length = i - start + 1;
        if (length >
            bestLength)  // > and not >=: an equally long later run does not replace the first
        {
            bestStart  = start;
            bestLength = length;
        }
    }
    return values.subspan(bestStart, bestLength);
}

}  // namespace ch01
