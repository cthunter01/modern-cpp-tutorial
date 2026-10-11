#include "p03_resolve_command.h"

#include <array>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>

// The tests fix two functions. ch03::resolve(commands, typed), called with a std::array of
// std::string_view and what the user typed, returns a result that answers `if (result)` and, when
// that is true, gives the chosen command as `*result`. ch03::explain(result) turns a result that
// is not a command into "unknown command" or "ambiguous command".

namespace
{

constexpr std::array<std::string_view, 6> kCommands{
    "status", "start", "stop", "commit", "config", "co",
};

TEST(ResolveCommand, AFullNameIsThatCommand)
{
    const auto result = ch03::resolve(kCommands, "status");
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "status");
}

TEST(ResolveCommand, AnUnambiguousBeginningIsEnough)
{
    for (const auto& [typed, meant] : {
             std::pair<std::string_view, std::string_view>{"stat", "status"},
             std::pair<std::string_view, std::string_view>{"star", "start"},
             std::pair<std::string_view, std::string_view>{"sto", "stop"},
             std::pair<std::string_view, std::string_view>{"com", "commit"},
             std::pair<std::string_view, std::string_view>{"conf", "config"},
         })
    {
        const auto result = ch03::resolve(kCommands, typed);
        ASSERT_TRUE(result) << typed;
        EXPECT_EQ(*result, meant) << typed;
    }
}

TEST(ResolveCommand, LetterCaseDoesNotMatter)
{
    const auto result = ch03::resolve(kCommands, "STo");
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "stop");  // the command as it is spelt in the list, not as it was typed
}

TEST(ResolveCommand, ABeginningSharedBySeveralCommandsIsAmbiguous)
{
    for (const std::string_view typed : {"s", "st", "sta", "c", ""})
    {
        const auto result = ch03::resolve(kCommands, typed);
        ASSERT_FALSE(result) << typed;
        EXPECT_EQ(ch03::explain(result), "ambiguous command") << typed;
    }
}

TEST(ResolveCommand, AnExactNameWinsOverLongerCommandsThatStartWithIt)
{
    // "co" is a command, and also the beginning of "commit" and "config".
    const auto result = ch03::resolve(kCommands, "CO");
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "co");
}

TEST(ResolveCommand, SomethingNoCommandStartsWithIsUnknown)
{
    for (const std::string_view typed : {"x", "statuses", "stopp", "-s"})
    {
        const auto result = ch03::resolve(kCommands, typed);
        ASSERT_FALSE(result) << typed;
        EXPECT_EQ(ch03::explain(result), "unknown command") << typed;
    }
}

}  // namespace
