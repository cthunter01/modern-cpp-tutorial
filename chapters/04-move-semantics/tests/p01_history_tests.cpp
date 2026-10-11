#include "p01_history.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>

#include "tut/storage.h"

// The tests fix a class ch04::History, built with the number of snapshots to keep, with
//   Store(snapshot)   takes a std::string the caller gives up
//   Undo()            answers `if (result)`; `*result` is the most recent snapshot, which is gone
//   from
//                     the history afterwards
// and they check what the statement demands and the types cannot show: that no snapshot is ever
// duplicated, and that a caller cannot have one copied without saying so.

namespace
{

/// Whether `history.Store(argument)` compiles when the argument has the type and value category
/// Arg.
template <typename Arg>
concept CanStore =
    requires(ch04::History history, Arg argument) { history.Store(std::forward<Arg>(argument)); };

// "A caller who wants to keep a copy must say so visibly." So `history.Store(document)`, with a
// named string, must not compile; giving the string up, or passing an explicit copy, must.
static_assert(!CanStore<std::string&>,
              "Store(document) compiles: megabytes would be copied without a trace at the call");
static_assert(!CanStore<const std::string&>);
static_assert(CanStore<std::string>);

/// A megabyte of one character: certainly on the heap, so that its address is evidence.
std::string megabyte(char fill)
{
    constexpr std::size_t kSize = 1'000'000;
    std::string           text(kSize, fill);
    return text;
}

TEST(History, UndoReturnsTheMostRecentSnapshot)
{
    ch04::History history(3);
    history.Store(megabyte('a'));
    history.Store(megabyte('b'));

    const auto latest = history.Undo();
    if (!latest)
    {
        FAIL() << "two snapshots were stored";
    }
    EXPECT_EQ(std::string_view(*latest).front(), 'b');

    const auto earlier = history.Undo();
    if (!earlier)
    {
        FAIL() << "one snapshot was left";
    }
    EXPECT_EQ(std::string_view(*earlier).front(), 'a');
}

TEST(History, UndoOnAnEmptyHistoryGivesNothing)
{
    ch04::History history(3);
    EXPECT_FALSE(history.Undo());

    history.Store(megabyte('a'));
    EXPECT_TRUE(history.Undo());
    EXPECT_FALSE(history.Undo());
}

TEST(History, KeepsOnlyTheNewestSnapshots)
{
    ch04::History history(2);
    history.Store(megabyte('a'));
    history.Store(megabyte('b'));
    history.Store(megabyte('c'));  // 'a' is the oldest and has to go

    const auto first  = history.Undo();
    const auto second = history.Undo();
    if (!first || !second)
    {
        FAIL() << "the history keeps two snapshots";
    }
    EXPECT_EQ(std::string_view(*first).front(), 'c');
    EXPECT_EQ(std::string_view(*second).front(), 'b');
    EXPECT_FALSE(history.Undo());
}

TEST(History, ASnapshotIsNeverDuplicated)
{
    ch04::History history(3);
    std::string   document   = megabyte('x');
    const auto    characters = tut::storage_of(document);

    history.Store(std::move(document));
    history.Store(megabyte('y'));  // enough activity for the container to move things around
    history.Store(megabyte('z'));
    static_cast<void>(history.Undo());
    static_cast<void>(history.Undo());

    const auto restored = history.Undo();
    if (!restored)
    {
        FAIL() << "the first snapshot should still be there";
    }
    EXPECT_EQ(tut::storage_of(*restored), characters)
        << "what came back is a copy: somewhere between Store and Undo a megabyte was duplicated";
}

}  // namespace
