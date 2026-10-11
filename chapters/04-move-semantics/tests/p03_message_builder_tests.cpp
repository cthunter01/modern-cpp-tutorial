#include "p03_message_builder.h"

#include <string>

#include <gtest/gtest.h>

// Problem 3 has no test for your answer: it asks for a decision and a reason. This file only checks
// that the reference does what its comment promises.

namespace
{

TEST(MessageBuilder, TakeHandsOverTheTextAndLeavesTheBuilderEmpty)
{
    ch04::MessageBuilder builder;
    builder.Add("disk ").Add("almost ").Add("full");

    const std::string message = builder.Take();

    EXPECT_EQ(message, "disk almost full");
    EXPECT_TRUE(builder.Empty());
}

TEST(MessageBuilder, CanBeUsedAgainAfterTake)
{
    ch04::MessageBuilder builder;
    builder.Add("first");
    EXPECT_EQ(builder.Take(), "first");

    builder.Add("second");
    EXPECT_EQ(builder.Take(), "second");
    EXPECT_EQ(builder.Take(), "");
}

}  // namespace
