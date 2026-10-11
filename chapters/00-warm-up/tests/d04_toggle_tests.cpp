#include "d04_toggle.h"

#include <gtest/gtest.h>

namespace
{

TEST(Toggle, StartsCleared)
{
    ch00::Toggle toggle;
    EXPECT_FALSE(toggle.TakeAndReset());
}

TEST(Toggle, IsCollectedExactlyOnce)
{
    ch00::Toggle toggle;
    toggle.Set();
    EXPECT_TRUE(toggle.TakeAndReset());
    EXPECT_FALSE(toggle.TakeAndReset());
}

TEST(Toggle, SettingTwiceIsCollectedOnce)
{
    ch00::Toggle toggle;
    toggle.Set();
    toggle.Set();
    EXPECT_TRUE(toggle.TakeAndReset());
    EXPECT_FALSE(toggle.TakeAndReset());
}

}  // namespace
