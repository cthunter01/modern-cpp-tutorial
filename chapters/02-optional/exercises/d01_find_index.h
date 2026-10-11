#pragma once

#include <cstddef>
#include <optional>
#include <span>

#include "tut/todo.h"

namespace ch02
{

/// The position of the first element equal to @p target, or nothing when there is none.
[[nodiscard]] constexpr std::optional<std::size_t> find_index(std::span<const int> values,
                                                              int                  target) noexcept
{
    TUT_TODO(values, target);
}

}  // namespace ch02
