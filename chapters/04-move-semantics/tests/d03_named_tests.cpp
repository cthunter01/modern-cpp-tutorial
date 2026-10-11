#include "d03_named.h"

#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "tut/tracked.h"

namespace
{

TEST(Named, MovingANamedMovesItsPayload)
{
    int         destroyed = 0;
    ch04::Named original("first", tut::Tracked{7}, destroyed);
    tut::Tracked::ResetCounts();

    const ch04::Named moved(std::move(original));

    EXPECT_EQ(moved.Payload().Value(), 7);
    EXPECT_EQ(tut::Tracked::Counts().copies, 0) << "std::move was written, and a copy was made";
    EXPECT_EQ(tut::Tracked::Counts().moves, 1);
}

TEST(Named, MoveAssignmentMovesToo)
{
    int         destroyed = 0;
    ch04::Named source("source", tut::Tracked{1}, destroyed);
    ch04::Named target("target", tut::Tracked{2}, destroyed);
    tut::Tracked::ResetCounts();

    target = std::move(source);

    EXPECT_EQ(target.Payload().Value(), 1);
    EXPECT_EQ(tut::Tracked::Counts().copies, 0);
}

TEST(Named, AVectorOfNamedGrowsWithoutCopies)
{
    int                      destroyed = 0;
    std::vector<ch04::Named> all;
    tut::Tracked::ResetCounts();

    for (int i = 0; i < 50; ++i)
    {
        // NOLINTNEXTLINE(performance-inefficient-vector-operation): growing is what is tested
        all.emplace_back("item", tut::Tracked{i}, destroyed);
    }

    EXPECT_EQ(tut::Tracked::Counts().copies, 0);
    EXPECT_EQ(all.back().Payload().Value(), 49);
}

TEST(Named, TheDestructorStillCounts)
{
    int destroyed = 0;
    {
        const ch04::Named one("one", tut::Tracked{1}, destroyed);
        // Copying is still possible.   NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        const ch04::Named two(one);
        EXPECT_EQ(two.Name(), "one");
        EXPECT_EQ(tut::Tracked::Counts().copies >= 1, true);
    }
    EXPECT_EQ(destroyed, 2);

    // And the point of the drill, once more, so that this case cannot pass before the others do.
    ch04::Named original("x", tut::Tracked{1}, destroyed);
    tut::Tracked::ResetCounts();
    const ch04::Named moved(std::move(original));
    EXPECT_EQ(tut::Tracked::Counts().copies, 0);
    EXPECT_EQ(moved.Name(), "x");
}

}  // namespace
