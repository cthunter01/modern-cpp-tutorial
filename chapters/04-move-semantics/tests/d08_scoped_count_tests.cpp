#include "d08_scoped_count.h"

#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace
{

ch04::ScopedCount enter(int& counter)
{
    return ch04::ScopedCount(counter);
}

TEST(ScopedCount, CountsWhileItLives)
{
    int active = 0;
    {
        const ch04::ScopedCount one(active);
        EXPECT_EQ(active, 1);
        {
            const ch04::ScopedCount two(active);
            EXPECT_EQ(active, 2);
        }
        EXPECT_EQ(active, 1);
    }
    EXPECT_EQ(active, 0);
}

TEST(ScopedCount, MovingDoesNotCountTwiceOrSubtractTwice)
{
    int active = 0;
    {
        ch04::ScopedCount original(active);
        EXPECT_EQ(active, 1);

        const ch04::ScopedCount moved(std::move(original));
        EXPECT_EQ(active, 1) << "a move hands the count on; it does not add one";
    }
    EXPECT_EQ(active, 0) << "two objects were destroyed, and exactly one of them may subtract";
}

TEST(ScopedCount, CanBeReturnedFromAFunction)
{
    int active = 0;
    {
        const ch04::ScopedCount held = enter(active);
        EXPECT_EQ(active, 1);
    }
    EXPECT_EQ(active, 0);
}

TEST(ScopedCount, MoveAssignmentReleasesWhatTheTargetHeld)
{
    int first  = 0;
    int second = 0;
    {
        ch04::ScopedCount target(first);
        ch04::ScopedCount source(second);

        target = std::move(source);

        EXPECT_EQ(first, 0) << "the target stopped counting `first` when it took over `second`";
        EXPECT_EQ(second, 1);
    }
    EXPECT_EQ(first, 0);
    EXPECT_EQ(second, 0);
}

TEST(ScopedCount, SurvivesAGrowingVector)
{
    int active = 0;
    {
        std::vector<ch04::ScopedCount> held;
        for (int i = 0; i < 100; ++i)
        {
            // NOLINTNEXTLINE(performance-inefficient-vector-operation): growing is what is tested
            held.emplace_back(active);
        }
        EXPECT_EQ(active, 100);
        // Copying it would be a bug by construction, so it must not be possible at all.
        EXPECT_FALSE(std::is_copy_constructible_v<ch04::ScopedCount>);
        EXPECT_FALSE(std::is_copy_assignable_v<ch04::ScopedCount>);
    }
    EXPECT_EQ(active, 0);
}

}  // namespace
