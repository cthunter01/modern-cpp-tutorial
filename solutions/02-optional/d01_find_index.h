#pragma once

#include <cstddef>
#include <optional>
#include <span>

namespace ch02
{

/// The position of the first element equal to @p target, or nothing when there is none.
[[nodiscard]] constexpr std::optional<std::size_t> find_index(std::span<const int> values,
                                                              int                  target) noexcept
{
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (values[i] == target)
        {
            return i;  // an index converts to an engaged optional, also when it is 0
        }
    }
    return std::nullopt;
}

}  // namespace ch02
