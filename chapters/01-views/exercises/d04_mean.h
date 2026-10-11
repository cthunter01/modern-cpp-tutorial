#pragma once

#include <span>

#include "tut/todo.h"

namespace ch01
{

/// The arithmetic mean of @p values, 0.0 when there are none.
[[nodiscard]] inline double mean(std::span<const double> values) noexcept
{
    TUT_TODO(values);
}

}  // namespace ch01
