#include "d04_is_one_of.h"

#include <cstdint>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

namespace
{

enum class State : std::uint8_t
{
    Stopped,
    Running,
    Draining,
    Exiting,
};

TEST(IsOneOf, FindsTheValueAmongTheCandidates)
{
    EXPECT_TRUE(ch05::is_one_of(3, 1, 2, 3));
    EXPECT_TRUE(ch05::is_one_of(1, 1));
    EXPECT_FALSE(ch05::is_one_of(4, 1, 2, 3));
}

TEST(IsOneOf, NothingIsOneOfNothing)
{
    EXPECT_FALSE(ch05::is_one_of(4));
}

TEST(IsOneOf, ReadsLikeTheConditionItReplaces)
{
    // state == State::Stopped || state == State::Exiting
    const State state = State::Exiting;
    EXPECT_TRUE(ch05::is_one_of(state, State::Stopped, State::Exiting));
    EXPECT_FALSE(ch05::is_one_of(state, State::Running, State::Draining));
}

TEST(IsOneOf, TheCandidatesMayHaveDifferentTypes)
{
    const std::string      word = "stop";
    const std::string_view view = "stop";
    EXPECT_TRUE(ch05::is_one_of(word, "start", view));
    EXPECT_FALSE(ch05::is_one_of(word, "start", "status"));
}

}  // namespace
