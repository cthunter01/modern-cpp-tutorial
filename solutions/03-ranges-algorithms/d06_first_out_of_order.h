#pragma once

#include <algorithm>
#include <cstddef>
#include <optional>
#include <span>

namespace ch03
{

struct Sample
{
    int    timestamp = 0;
    double value     = 0.0;
};

/// The index of the first sample whose timestamp is smaller than that of the sample before it, or
/// nothing when the timestamps never go backwards. Equal timestamps are in order.
[[nodiscard]] inline std::optional<std::size_t> first_out_of_order(std::span<const Sample> samples)
{
    const auto firstBad = std::ranges::is_sorted_until(samples, {}, &Sample::timestamp);
    if (firstBad == samples.end())
    {
        return std::nullopt;
    }
    return static_cast<std::size_t>(firstBad - samples.begin());
}

}  // namespace ch03
