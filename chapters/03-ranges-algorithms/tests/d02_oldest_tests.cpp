#include "d02_oldest.h"

#include <vector>

#include <gtest/gtest.h>

#include "person.h"

namespace
{

TEST(Oldest, FindsThePersonNotTheAge)
{
    const std::vector<ch03::Person> people{
        {.name = "Ada", .age = 36, .id = 1},
        {.name = "Grace", .age = 85, .id = 2},
        {.name = "Linus", .age = 54, .id = 3},
    };

    const ch03::Person* const found = ch03::oldest(people);

    // The projection chose what to compare. What comes back is still the element: the very
    // object in `people`, not a copy and not the number 85.
    EXPECT_EQ(found, &people[1]);
}

TEST(Oldest, TheFirstOfSeveralEquallyOldWins)
{
    const std::vector<ch03::Person> people{
        {.name = "a", .age = 40, .id = 1},
        {.name = "b", .age = 70, .id = 2},
        {.name = "c", .age = 70, .id = 3},
    };

    EXPECT_EQ(ch03::oldest(people), &people[1]);
}

TEST(Oldest, NobodyHasNoOldest)
{
    EXPECT_EQ(ch03::oldest({}), nullptr);
}

}  // namespace
