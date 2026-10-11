#pragma once

#include <cstddef>
#include <optional>
#include <span>

#include "tut/todo.h"

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
    TUT_TODO(samples);
}

}  // namespace ch03
