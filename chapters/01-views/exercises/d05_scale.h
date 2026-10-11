#pragma once

#include <span>

#include "tut/todo.h"

namespace ch01
{

/// Multiplies every element of @p values by @p factor, in place.
inline void scale(std::span<double> values, double factor) noexcept
{
    TUT_TODO(values, factor);
}

}  // namespace ch01
