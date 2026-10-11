#include "p01_ranking.h"

#include <cstddef>
#include <string>
#include <vector>

#include <gtest/gtest.h>

// The tests fix: a struct ch03::Result with members name, points and finishSeconds, in that order;
// and a class ch03::Ranking built from a std::vector<Result>, with
//   Table()               something to iterate over, whose elements have a .name, best first
//   RankOf(name)          answers `if (rank)`, and `*rank` is the rank, 1 for the winner
//   WithinOfLeader(n)     like Table(), but only the results at most n points behind the leader

namespace
{

std::vector<ch03::Result> match()
{
    return {
        {.name = "dave", .points = 12, .finishSeconds = 300},
        {.name = "alice", .points = 30, .finishSeconds = 420},
        {.name = "erin", .points = 30, .finishSeconds = 400},
        {.name = "carol", .points = 25, .finishSeconds = 500},
        {.name = "bob", .points = 25, .finishSeconds = 500},
        {.name = "frank", .points = 19, .finishSeconds = 100},
    };
}

template <typename Entries>
std::vector<std::string> names_of(const Entries& entries)
{
    std::vector<std::string> names;
    for (const auto& entry : entries)
    {
        names.emplace_back(entry.name);
    }
    return names;
}

TEST(Ranking, MorePointsThenEarlierFinishThenName)
{
    const ch03::Ranking ranking(match());

    // erin and alice have 30 points; erin finished earlier. carol and bob are level on points and
    // time; bob comes first in the alphabet.
    EXPECT_EQ(names_of(ranking.Table()),
              (std::vector<std::string>{"erin", "alice", "bob", "carol", "frank", "dave"}));
}

TEST(Ranking, RankOfCountsFromOne)
{
    const ch03::Ranking ranking(match());

    const auto first = ranking.RankOf("erin");
    const auto last  = ranking.RankOf("dave");

    if (!first || !last)
    {
        FAIL() << "erin and dave took part, so they have a rank";
    }
    EXPECT_EQ(static_cast<std::size_t>(*first), 1U);
    EXPECT_EQ(static_cast<std::size_t>(*last), 6U);
}

TEST(Ranking, SomeoneWhoDidNotTakePartHasNoRank)
{
    const ch03::Ranking ranking(match());
    EXPECT_FALSE(ranking.RankOf("zoe"));
    EXPECT_FALSE(ranking.RankOf(""));
}

TEST(Ranking, WithinOfLeaderIsInclusive)
{
    const ch03::Ranking ranking(match());

    // The leader has 30. Within 5: 30, 30, 25, 25. Within 4: only the two with 30.
    EXPECT_EQ(names_of(ranking.WithinOfLeader(5)),
              (std::vector<std::string>{"erin", "alice", "bob", "carol"}));
    EXPECT_EQ(names_of(ranking.WithinOfLeader(4)), (std::vector<std::string>{"erin", "alice"}));
    EXPECT_EQ(names_of(ranking.WithinOfLeader(0)), (std::vector<std::string>{"erin", "alice"}));
    EXPECT_EQ(names_of(ranking.WithinOfLeader(1000)).size(), 6U);
}

TEST(Ranking, AnEmptyMatch)
{
    const ch03::Ranking ranking({});
    EXPECT_TRUE(names_of(ranking.Table()).empty());
    EXPECT_TRUE(names_of(ranking.WithinOfLeader(10)).empty());
    EXPECT_FALSE(ranking.RankOf("erin"));
}

}  // namespace
