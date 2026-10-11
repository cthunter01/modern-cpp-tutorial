#include "p02_merge_events.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "tut/storage.h"

// The tests fix: a struct review_a::Event with members id and payload, in that order; and a
// function review_a::merge_events(first, second), called with two std::vector<Event> that the
// caller gives up with std::move. It returns something to iterate over, whose elements have an .id
// and a .payload.

namespace
{

/// A payload long enough to live on the heap, so that its address shows whether it was copied.
std::string payload(char fill)
{
    std::string text(300, fill);
    return text;
}

template <typename Events>
std::vector<int> ids_of(const Events& events)
{
    std::vector<int> ids;
    for (const auto& event : events)
    {
        // NOLINTNEXTLINE(performance-inefficient-vector-operation): the size is not known
        ids.push_back(event.id);
    }
    return ids;
}

/// Events with the given ids and short payloads.
std::vector<review_a::Event> events_with(const std::vector<int>& ids)
{
    std::vector<review_a::Event> events;
    events.reserve(ids.size());
    for (const int id : ids)
    {
        events.push_back({.id = id, .payload = "event " + std::to_string(id)});
    }
    return events;
}

TEST(MergeEvents, InterleavesTheTwoStreamsById)
{
    std::vector<review_a::Event> first  = events_with({1, 4, 9});
    std::vector<review_a::Event> second = events_with({2, 3, 10});

    const auto merged = review_a::merge_events(std::move(first), std::move(second));

    EXPECT_EQ(ids_of(merged), (std::vector<int>{1, 2, 3, 4, 9, 10}));
}

TEST(MergeEvents, AnEventThatCameFromBothServicesAppearsOnce)
{
    std::vector<review_a::Event> first  = events_with({1, 2, 5});
    std::vector<review_a::Event> second = events_with({2, 3, 5});

    const auto merged = review_a::merge_events(std::move(first), std::move(second));

    EXPECT_EQ(ids_of(merged), (std::vector<int>{1, 2, 3, 5}));
}

TEST(MergeEvents, OneEmptyStream)
{
    std::vector<review_a::Event> events = events_with({1, 2});

    EXPECT_EQ(ids_of(review_a::merge_events(std::move(events), {})), (std::vector<int>{1, 2}));
    EXPECT_TRUE(ids_of(review_a::merge_events({}, {})).empty());
}

TEST(MergeEvents, NoPayloadIsCopied)
{
    std::vector<review_a::Event> first;
    std::vector<review_a::Event> second;
    first.push_back({.id = 1, .payload = payload('a')});
    first.push_back({.id = 3, .payload = payload('c')});
    second.push_back({.id = 2, .payload = payload('b')});
    second.push_back({.id = 4, .payload = payload('d')});
    const std::vector<std::uintptr_t> blocks{
        tut::storage_of(first[0].payload),
        tut::storage_of(second[0].payload),
        tut::storage_of(first[1].payload),
        tut::storage_of(second[1].payload),
    };

    const auto merged = review_a::merge_events(std::move(first), std::move(second));

    std::vector<std::uintptr_t> after;
    for (const auto& event : merged)
    {
        // NOLINTNEXTLINE(performance-inefficient-vector-operation): the size is not known
        after.push_back(tut::storage_of(event.payload));
    }
    EXPECT_EQ(after, blocks) << "a payload in the result is not the block the caller handed over";
}

}  // namespace
