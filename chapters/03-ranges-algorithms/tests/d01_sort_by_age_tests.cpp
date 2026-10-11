#include "d01_sort_by_age.h"

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "person.h"

namespace
{

std::vector<std::string> names_of(const std::vector<ch03::Person>& people)
{
    std::vector<std::string> names;
    names.reserve(people.size());
    for (const ch03::Person& person : people)
    {
        names.push_back(person.name);
    }
    return names;
}

TEST(SortByAge, PutsTheYoungestFirst)
{
    std::vector<ch03::Person> people{
        {.name = "Grace", .age = 85, .id = 1},
        {.name = "Ada", .age = 36, .id = 2},
        {.name = "Linus", .age = 54, .id = 3},
    };

    ch03::sort_by_age(people);

    EXPECT_EQ(names_of(people), (std::vector<std::string>{"Ada", "Linus", "Grace"}));
}

TEST(SortByAge, PeopleOfTheSameAgeKeepTheirOrder)
{
    // A hundred people aged 30, in a known order, with older and younger people between them. A
    // sort that is not stable is free to shuffle the thirty-year-olds, and with this many elements
    // the usual implementations do.
    constexpr int             kGroups = 100;
    std::vector<ch03::Person> people;
    std::vector<std::string>  thirty;
    for (int i = 0; i < kGroups; ++i)
    {
        const std::string number = std::to_string(i);
        thirty.push_back("thirty" + number);
        people.push_back({.name = "older" + number, .age = 31 + ((i * 7) % 50), .id = 3 * i});
        people.push_back({.name = thirty.back(), .age = 30, .id = (3 * i) + 1});
        people.push_back(
            {.name = "younger" + number, .age = 29 - ((i * 3) % 20), .id = (3 * i) + 2});
    }

    ch03::sort_by_age(people);

    std::vector<std::string> sortedThirty;
    for (const ch03::Person& person : people)
    {
        if (person.age == 30)
        {
            sortedThirty.push_back(person.name);
        }
    }
    EXPECT_EQ(sortedThirty, thirty);
}

TEST(SortByAge, NobodyIsFine)
{
    std::vector<ch03::Person> people;
    ch03::sort_by_age(people);
    EXPECT_TRUE(people.empty());
}

}  // namespace
