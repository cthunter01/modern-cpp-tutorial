#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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
    // A projection may compute something. This one makes a sort key out of each score: a pair,
    // compared member by member. Negating the points turns "smaller first" into "more points
    // first"; the name is a view, so that no string is copied to be compared.
    const auto key = [](const Score& score) {
        return std::pair<int, std::string_view>(-score.points, score.name);
    };
    std::ranges::sort(scores, {}, key);
    scores.resize(std::min(n, scores.size()));
    return scores;
}

}  // namespace ch03
