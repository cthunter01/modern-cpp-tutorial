#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <span>
#include <string>
#include <vector>

namespace review_a
{

struct Measurement
{
    std::string      station;
    double           value = 0.0;
    std::vector<int> samples;
};

//! [largest]
/// The @p count measurements with the highest values, highest first (fewer when there are not that
/// many). The result points into @p measurements: it is valid while that storage is.
[[nodiscard]] inline std::vector<const Measurement*> largest(
    std::span<const Measurement> measurements, std::size_t count)
{
    std::vector<const Measurement*> order;
    order.reserve(measurements.size());
    for (const Measurement& measurement : measurements)
    {
        order.push_back(&measurement);
    }

    // Sort pointers, compare what they point at. partial_sort puts the `count` best in front, in
    // order, and does not bother to sort the rest.
    count             = std::min(count, order.size());
    const auto middle = order.begin() + static_cast<std::ptrdiff_t>(count);
    std::ranges::partial_sort(order, middle, std::ranges::greater{},
                              [](const Measurement* measurement) { return measurement->value; });
    order.resize(count);
    return order;
}
//! [largest]

}  // namespace review_a
