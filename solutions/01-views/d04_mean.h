#pragma once

#include <span>

namespace ch01
{

/// The arithmetic mean of @p values, 0.0 when there are none.
[[nodiscard]] inline double mean(std::span<const double> values) noexcept
{
    if (values.empty())
    {
        return 0.0;
    }
    double sum = 0.0;
    for (const double value : values)
    {
        sum += value;
    }
    return sum / static_cast<double>(values.size());
}

}  // namespace ch01
