#pragma once

#include <algorithm>
#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace ch03
{

struct Result
{
    std::string name;
    int         points        = 0;
    int         finishSeconds = 0;
};

/// The results of a match, in rank order: more points first, then the earlier finish, then the
/// name.
class Ranking
{
public:
    //! [sort]
    explicit Ranking(std::vector<Result> results) : table(std::move(results))
    {
        // The whole ranking rule is this one key. Tuples compare member by member, so the first
        // difference decides; negated points put the larger number first.
        std::ranges::sort(table, {}, [](const Result& result) {
            return std::tuple<int, int, std::string_view>(-result.points, result.finishSeconds,
                                                          result.name);
        });
    }
    //! [sort]

    /// Every result, best first.
    [[nodiscard]] std::span<const Result> Table() const noexcept { return table; }

    //! [rank]
    /// The rank of @p name, 1 for the winner, or nothing when @p name did not take part.
    [[nodiscard]] std::optional<std::size_t> RankOf(std::string_view name) const
    {
        const auto found = std::ranges::find(table, name, &Result::name);
        if (found == table.end())
        {
            return std::nullopt;
        }
        return static_cast<std::size_t>(found - table.begin()) + 1;
    }
    //! [rank]

    //! [within]
    /// The results that are at most @p points behind the leader, best first. A part of Table().
    [[nodiscard]] std::span<const Result> WithinOfLeader(int points) const
    {
        if (table.empty())
        {
            return {};
        }
        // The table is sorted by points, descending, so "close enough to the leader" is true for a
        // run of elements at the front and false for the rest. partition_point finds where the run
        // ends, by binary search.
        const int  threshold = table.front().points - points;
        const auto end       = std::ranges::partition_point(
            table, [threshold](int candidate) { return candidate >= threshold; }, &Result::points);
        return {table.begin(), end};
    }
    //! [within]

private:
    std::vector<Result> table;
};

}  // namespace ch03
