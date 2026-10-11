#pragma once

#include <vector>

#include "tut/todo.h"

namespace ch00
{

struct Range
{
    int min = 0;
    int max = 0;
};

/// The smallest and the largest value. For an empty vector both are 0.
inline Range min_max(const std::vector<int>& values)
{
    TUT_TODO(values);
}

}  // namespace ch00
