#pragma once

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

namespace review_a
{

struct Event
{
    int         id = 0;
    std::string payload;
};

//! [merge]
/// All events of @p first and @p second, sorted by id, each id once. Both inputs must be sorted by
/// id. They are taken by value: the caller hands them over with std::move, and no payload is
/// copied.
[[nodiscard]] inline std::vector<Event> merge_events(std::vector<Event> first,
                                                     std::vector<Event> second)
{
    std::vector<Event> merged;
    merged.reserve(first.size() + second.size());

    // A move iterator hands out rvalues, so the algorithm moves each event to its place in `merged`
    // where it would otherwise copy it.
    std::merge(std::make_move_iterator(first.begin()), std::make_move_iterator(first.end()),
               std::make_move_iterator(second.begin()), std::make_move_iterator(second.end()),
               std::back_inserter(merged),
               [](const Event& a, const Event& b) { return a.id < b.id; });

    // Equal ids are neighbours now. unique() keeps the first of each run and returns the leftover
    // part.
    const auto leftover = std::ranges::unique(merged, {}, &Event::id);
    merged.erase(leftover.begin(), leftover.end());
    return merged;
}
//! [merge]

}  // namespace review_a
