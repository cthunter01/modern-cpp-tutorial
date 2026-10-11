#include "d06_make_items.h"

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "tut/storage.h"
#include "tut/tracked.h"

namespace
{

// Take() gives the builder's text away, so it may only be called on a builder that is being given
// up.
static_assert(std::is_invocable_v<decltype(&ch04::Builder::Take), ch04::Builder>);
static_assert(!std::is_invocable_v<decltype(&ch04::Builder::Take), ch04::Builder&>);

TEST(MakeItems, ReturnsTheItemsWithoutCopyingAny)
{
    tut::Tracked::ResetCounts();

    const std::vector<tut::Tracked> items = ch04::make_items(100);

    ASSERT_EQ(items.size(), 100U);
    EXPECT_EQ(items.front().Value(), 0);
    EXPECT_EQ(items.back().Value(), 99);
    EXPECT_EQ(tut::Tracked::Counts().copies, 0);
}

TEST(MakeItems, NoneIsFine)
{
    EXPECT_TRUE(ch04::make_items(0).empty());
    EXPECT_EQ(ch04::make_items(1).size(), 1U);
}

TEST(Builder, TakeHandsTheTextOverWithoutCopyingIt)
{
    ch04::Builder builder;
    builder.Append(std::string(200, 'x'));  // long enough to live on the heap
    builder.Append("!");
    const auto characters = tut::storage_of(builder.View());

    const std::string text = std::move(builder).Take();

    EXPECT_EQ(text.size(), 201U);
    EXPECT_EQ(text.back(), '!');
    EXPECT_EQ(tut::storage_of(text), characters)
        << "Take() returned a copy of the text, not the text";
}

}  // namespace
