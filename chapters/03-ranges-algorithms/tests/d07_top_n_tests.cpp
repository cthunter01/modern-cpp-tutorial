#include "d07_top_n.h"

#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace
{

std::vector<std::string> names_of(const std::vector<ch03::Score>& scores)
{
    std::vector<std::string> names;
    names.reserve(scores.size());
    for (const ch03::Score& score : scores)
    {
        names.push_back(score.name);
    }
    return names;
}

std::vector<ch03::Score> scores()
{
    return {
        {.name = "dave", .points = 70}, {.name = "alice", .points = 90},
        {.name = "erin", .points = 85}, {.name = "carol", .points = 90},
        {.name = "bob", .points = 85},  {.name = "frank", .points = 10},
    };
}

TEST(TopN, MostPointsFirst)
{
    EXPECT_EQ(names_of(ch03::top_n(scores(), 1)), (std::vector<std::string>{"alice"}));
}

TEST(TopN, EqualPointsAreOrderedByName)
{
    EXPECT_EQ(names_of(ch03::top_n(scores(), 4)),
              (std::vector<std::string>{"alice", "carol", "bob", "erin"}));
}

TEST(TopN, AskingForMoreThanThereAreGivesAll)
{
    EXPECT_EQ(names_of(ch03::top_n(scores(), 100)),
              (std::vector<std::string>{"alice", "carol", "bob", "erin", "dave", "frank"}));
}

TEST(TopN, AskingForNoneGivesNone)
{
    EXPECT_TRUE(ch03::top_n(scores(), 0).empty());
    EXPECT_TRUE(ch03::top_n({}, 3).empty());
}

TEST(TopN, KeepsThePointsWithTheNames)
{
    const std::vector<ch03::Score> best = ch03::top_n(scores(), 2);

    ASSERT_EQ(best.size(), 2U);
    EXPECT_EQ(best[0].points, 90);
    EXPECT_EQ(best[1].points, 90);
}

}  // namespace
