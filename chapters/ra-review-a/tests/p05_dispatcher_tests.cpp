#include "p05_dispatcher.h"

#include <optional>
#include <string>

#include <gtest/gtest.h>

// The tests fix a class review_a::Dispatcher with
//   On(verb, handler)   handler is called with the words that follow the verb, as something with
//                       size() and [] whose elements are texts; it returns
//                       std::optional<std::string>
//   Run(line)           answers `if (reply)`; `*reply` is what the handler returned
// The handlers below take `const auto& words`, so that they fit whatever you pass them.

namespace
{

/// What the reply says, or a text that no handler returns when there is none. The tests read
/// replies through this, so that a missing reply is a failed comparison and never a dereference of
/// nothing.
template <typename Reply>
std::string text_of(const Reply& reply)
{
    if (!reply)
    {
        return "<no reply>";
    }
    return std::string(*reply);
}

review_a::Dispatcher game()
{
    review_a::Dispatcher dispatcher;
    dispatcher.On("take", [](const auto& words) -> std::optional<std::string> {
        if (words.size() != 1)
        {
            return std::nullopt;  // "take" what?
        }
        return "taken: " + std::string(words[0]);
    });
    dispatcher.On("talk", [](const auto& words) -> std::optional<std::string> {
        return "you talk to " + std::to_string(words.size()) + " people";
    });
    dispatcher.On("go", [](const auto& words) -> std::optional<std::string> {
        std::string route = "going";
        for (const auto& word : words)
        {
            route += ' ';
            route += std::string(word);
        }
        return route;
    });
    dispatcher.On("inventory",
                  [](const auto& /*unused*/) -> std::optional<std::string> { return "a lamp"; });
    return dispatcher;
}

TEST(Dispatcher, CallsTheHandlerOfTheVerbWithTheRemainingWords)
{
    const auto reply = game().Run("take lamp");
    EXPECT_EQ(text_of(reply), "taken: lamp");
}

TEST(Dispatcher, PassesAllTheWordsInOrder)
{
    const auto reply = game().Run("go north by northwest");
    EXPECT_EQ(text_of(reply), "going north by northwest");
}

TEST(Dispatcher, IgnoresExtraBlanks)
{
    const auto reply = game().Run("   go    north   ");
    EXPECT_EQ(text_of(reply), "going north");
}

TEST(Dispatcher, AVerbMayBeAbbreviatedAndInAnyCase)
{
    const auto short1 = game().Run("i");
    const auto short2 = game().Run("INV");
    const auto short3 = game().Run("Tak lamp");

    EXPECT_EQ(text_of(short1), "a lamp");
    EXPECT_EQ(text_of(short2), "a lamp");
    EXPECT_EQ(text_of(short3), "taken: lamp");
}

TEST(Dispatcher, AnAmbiguousOrUnknownVerbGivesNothing)
{
    EXPECT_FALSE(game().Run("t lamp")) << "take or talk?";
    EXPECT_FALSE(game().Run("ta lamp"));
    EXPECT_FALSE(game().Run("jump"));
    EXPECT_FALSE(game().Run("golf"));
}

TEST(Dispatcher, AnEmptyLineGivesNothing)
{
    EXPECT_FALSE(game().Run(""));
    EXPECT_FALSE(game().Run("     "));
}

TEST(Dispatcher, AHandlerMayHaveNothingToSay)
{
    EXPECT_FALSE(game().Run("take"));
    EXPECT_FALSE(game().Run("take lamp sword"));
}

TEST(Dispatcher, AVerbWithoutArgumentsGetsAnEmptyList)
{
    const auto reply = game().Run("talk");
    EXPECT_EQ(text_of(reply), "you talk to 0 people");
}

}  // namespace
