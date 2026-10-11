#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "tut/todo.h"

namespace ch03
{

struct Score
{
    std::string name;
    int         points = 0;
};

/// The @p n best of @p scores: most points first, and among equal points in alphabetical order of
/// the name. Fewer than @p n when there are not that many.
[[nodiscard]] inline std::vector<Score> top_n(std::vector<Score> scores, std::size_t n)
{
    TUT_TODO(scores, n);
}

}  // namespace ch03
