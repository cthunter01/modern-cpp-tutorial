#include "d05_outbox.h"

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "tut/storage.h"

namespace
{

// Send takes over a string the caller gives up. A caller who passes a name must not compile: the
// copy of a long message would be invisible at the call.
static_assert(std::is_invocable_v<decltype(&ch04::Outbox::Send), ch04::Outbox&, std::string>);
static_assert(!std::is_invocable_v<decltype(&ch04::Outbox::Send), ch04::Outbox&, std::string&>);
static_assert(
    !std::is_invocable_v<decltype(&ch04::Outbox::Send), ch04::Outbox&, const std::string&>);

/// A text that is too long to be stored inside the string object, so that its characters are on
/// the heap and their address tells whether they were copied.
std::string long_text(char fill)
{
    std::string text(200, fill);
    return text;
}

TEST(Outbox, TheCharactersAreNeverCopied)
{
    ch04::Outbox outbox;
    std::string  message    = long_text('a');
    const auto   characters = tut::storage_of(message);

    outbox.Send(std::move(message));
    const std::vector<std::string> sent = outbox.Drain();

    ASSERT_EQ(sent.size(), 1U);
    EXPECT_EQ(tut::storage_of(sent.front()), characters)
        << "the string that came out is not the one that went in: it was copied on the way";
}

TEST(Outbox, DrainHandsOverEverythingInOrder)
{
    ch04::Outbox outbox;
    outbox.Send(long_text('a'));
    outbox.Send(long_text('b'));
    outbox.Send(long_text('c'));

    const std::vector<std::string> sent = outbox.Drain();

    ASSERT_EQ(sent.size(), 3U);
    EXPECT_EQ(sent[0].front(), 'a');
    EXPECT_EQ(sent[2].front(), 'c');
}

TEST(Outbox, IsEmptyAfterDrain)
{
    ch04::Outbox outbox;
    outbox.Send(long_text('a'));

    EXPECT_EQ(outbox.Drain().size(), 1U);
    EXPECT_TRUE(outbox.Drain().empty());

    outbox.Send(long_text('b'));  // and usable again
    EXPECT_EQ(outbox.Drain().size(), 1U);
}

}  // namespace
