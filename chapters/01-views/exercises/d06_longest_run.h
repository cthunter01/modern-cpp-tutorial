#pragma once

#include <span>

#include "tut/todo.h"

namespace ch01
{

/// The longest stretch of equal neighbours in @p values; the first one when several are equally
/// long. The result is a part of @p values. Empty for an empty input.
[[nodiscard]] inline std::span<const int> longest_run(std::span<const int> values) noexcept
{
    TUT_TODO(values);
}

}  // namespace ch01
