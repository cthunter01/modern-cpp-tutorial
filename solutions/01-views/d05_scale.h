#pragma once

#include <span>

namespace ch01
{

/// Multiplies every element of @p values by @p factor, in place.
inline void scale(std::span<double> values, double factor) noexcept
{
    for (double& value : values)
    {
        value *= factor;
    }
}

}  // namespace ch01
