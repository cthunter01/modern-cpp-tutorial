#include "d04_registry.h"

#include <utility>

#include <gtest/gtest.h>

#include "tut/tracked.h"

namespace
{

TEST(Registry, GivingUpAnItemCostsNoCopy)
{
    ch04::Registry registry;
    tut::Tracked   item{5};
    tut::Tracked::ResetCounts();

    registry.Add(std::move(item));

    EXPECT_EQ(tut::Tracked::Counts().copies, 0)
        << "the caller said std::move, and the item was copied";
    EXPECT_EQ(registry.ValueAt(0), 5);
}

TEST(Registry, ATemporaryCostsNoCopy)
{
    ch04::Registry registry;
    tut::Tracked::ResetCounts();

    registry.Add(tut::Tracked{9});

    EXPECT_EQ(tut::Tracked::Counts().copies, 0);
    EXPECT_EQ(registry.ValueAt(0), 9);
}

TEST(Registry, KeepingTheItemCostsExactlyOneCopy)
{
    ch04::Registry     registry;
    const tut::Tracked item{5};
    tut::Tracked::ResetCounts();

    registry.Add(item);

    EXPECT_EQ(tut::Tracked::Counts().copies, 1);
    EXPECT_EQ(item.Value(), 5) << "the caller's item must be untouched";
    EXPECT_EQ(registry.ValueAt(0), 5);

    // The same registry, and now the caller gives one up.
    tut::Tracked second{6};
    tut::Tracked::ResetCounts();
    registry.Add(std::move(second));
    EXPECT_EQ(tut::Tracked::Counts().copies, 0);
    EXPECT_EQ(registry.Size(), 2U);
}

}  // namespace
