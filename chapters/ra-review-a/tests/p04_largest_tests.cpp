#include "p04_largest.h"

#include <string>
#include <vector>

#include <gtest/gtest.h>

// The tests fix: a struct review_a::Measurement with members station, value and samples, in that
// order; and a function review_a::largest(measurements, count), called with a
// std::vector<Measurement>. It returns something to iterate over. Each element refers to one of the
// caller's measurements, and the tests reach it either as `element->station` or as
// `element.get().station`, whichever your element type offers.

namespace
{

template <typename Element>
const review_a::Measurement& measurement_of(const Element& element)
{
    if constexpr (requires { element->station; })
    {
        return *element;
    }
    else
    {
        return element.get();
    }
}

std::vector<review_a::Measurement> measurements()
{
    std::vector<review_a::Measurement> all;
    all.push_back({.station = "north", .value = 3.5, .samples = std::vector<int>(10'000, 1)});
    all.push_back({.station = "south", .value = 9.0, .samples = std::vector<int>(10'000, 2)});
    all.push_back({.station = "east", .value = 1.25, .samples = std::vector<int>(10'000, 3)});
    all.push_back({.station = "west", .value = 7.75, .samples = std::vector<int>(10'000, 4)});
    all.push_back({.station = "centre", .value = 5.0, .samples = std::vector<int>(10'000, 5)});
    return all;
}

template <typename Result>
std::vector<std::string> stations_of(const Result& result)
{
    std::vector<std::string> stations;
    for (const auto& element : result)
    {
        // NOLINTNEXTLINE(performance-inefficient-vector-operation): the size is not known
        stations.push_back(measurement_of(element).station);
    }
    return stations;
}

TEST(Largest, ReturnsTheHighestValuesHighestFirst)
{
    const std::vector<review_a::Measurement> all = measurements();

    EXPECT_EQ(stations_of(review_a::largest(all, 3)),
              (std::vector<std::string>{"south", "west", "centre"}));
    EXPECT_EQ(stations_of(review_a::largest(all, 1)), (std::vector<std::string>{"south"}));
}

TEST(Largest, AskingForMoreThanThereAreGivesAllInOrder)
{
    const std::vector<review_a::Measurement> all = measurements();

    EXPECT_EQ(stations_of(review_a::largest(all, 99)),
              (std::vector<std::string>{"south", "west", "centre", "north", "east"}));
    EXPECT_TRUE(stations_of(review_a::largest(all, 0)).empty());
}

TEST(Largest, LeavesTheCallersListAsItWas)
{
    const std::vector<review_a::Measurement> all = measurements();

    static_cast<void>(review_a::largest(all, 3));

    EXPECT_EQ(all.front().station, "north");
    EXPECT_EQ(all.back().station, "centre");
}

TEST(Largest, RefersToTheCallersMeasurementsAndCopiesNone)
{
    const std::vector<review_a::Measurement> all = measurements();

    const auto top = review_a::largest(all, 2);

    std::vector<const review_a::Measurement*> where;
    for (const auto& element : top)
    {
        // NOLINTNEXTLINE(performance-inefficient-vector-operation): the size is not known
        where.push_back(&measurement_of(element));
    }
    // The very objects in `all`: south is its second element, west its fourth.
    EXPECT_EQ(where, (std::vector<const review_a::Measurement*>{&all[1], &all[3]}));
}

}  // namespace
