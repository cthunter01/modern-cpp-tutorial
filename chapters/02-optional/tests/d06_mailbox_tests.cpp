#include "d06_mailbox.h"

#include <optional>
#include <string>

#include <gtest/gtest.h>

namespace
{

TEST(Mailbox, StartsEmpty)
{
    ch02::Mailbox mailbox;
    EXPECT_EQ(mailbox.Take(), std::nullopt);
}

TEST(Mailbox, HandsOverWhatWasPut)
{
    ch02::Mailbox mailbox;
    mailbox.Put("disk almost full");
    EXPECT_EQ(mailbox.Take(), "disk almost full");
}

TEST(Mailbox, AMessageIsCollectedOnce)
{
    ch02::Mailbox mailbox;
    mailbox.Put("a message long enough that it does not fit into the string object itself");

    const std::optional<std::string> first  = mailbox.Take();
    const std::optional<std::string> second = mailbox.Take();

    EXPECT_TRUE(first.has_value());
    // Not "an empty message": no message. A mailbox that was moved from, and not emptied, fails
    // here.
    EXPECT_FALSE(second.has_value());
}

TEST(Mailbox, ANewMessageReplacesAnUncollectedOne)
{
    ch02::Mailbox mailbox;
    mailbox.Put("first");
    mailbox.Put("second");
    EXPECT_EQ(mailbox.Take(), "second");
    EXPECT_EQ(mailbox.Take(), std::nullopt);
}

TEST(Mailbox, AnEmptyMessageIsAMessage)
{
    ch02::Mailbox mailbox;
    mailbox.Put("");

    const std::optional<std::string> taken = mailbox.Take();

    EXPECT_TRUE(taken.has_value());
    EXPECT_EQ(taken, "");
}

}  // namespace
