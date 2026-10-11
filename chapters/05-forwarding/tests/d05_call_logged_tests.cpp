#include "d05_call_logged.h"

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

// Whether a variable is const decides what is deduced from it and whether it can be moved from, so
// the variables in this file are declared the way each test needs them.
//
// NOLINTBEGIN(misc-const-correctness)

#include "tut/tracked.h"

namespace
{

// What stored() returns.   NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
int g_storage = 0;

int& stored()
{
    return g_storage;
}

TEST(CallLogged, NotesTheCallAndReturnsTheResult)
{
    std::vector<std::string> log;

    const int sum = ch05::call_logged(log, [](int a, int b) { return a + b; }, 2, 3);

    EXPECT_EQ(sum, 5);
    EXPECT_EQ(log.size(), 1U);
}

TEST(CallLogged, AnLvalueArgumentArrivesAsTheCallersObject)
{
    std::vector<std::string> log;
    int                      counter = 0;

    ch05::call_logged(log, [](int& value) { value += 10; }, counter);

    EXPECT_EQ(counter, 10) << "the function changed a copy, not the caller's variable";
}

TEST(CallLogged, AnLvalueArgumentIsNotMovedFrom)
{
    std::vector<std::string> log;
    tut::Tracked             kept{7};
    tut::Tracked::ResetCounts();

    ch05::call_logged(log, [](const tut::Tracked& item) { return item.Value(); }, kept);

    EXPECT_FALSE(kept.IsMovedFrom());
    EXPECT_EQ(tut::Tracked::Counts().copies, 0);
    EXPECT_EQ(tut::Tracked::Counts().moves, 0);
}

TEST(CallLogged, AnRvalueArgumentIsMovedExactlyOnce)
{
    std::vector<std::string> log;
    tut::Tracked             given{7};
    tut::Tracked::ResetCounts();

    // The function takes its argument by value: it is a sink.
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    const auto consume = [](tut::Tracked item) { return item.Value(); };
    const int  value   = ch05::call_logged(log, consume, std::move(given));

    EXPECT_EQ(value, 7);
    EXPECT_EQ(tut::Tracked::Counts().copies, 0)
        << "the caller gave the argument up, and it was copied";
    EXPECT_EQ(tut::Tracked::Counts().moves, 1);
}

TEST(CallLogged, AnArgumentThatCannotBeCopiedGetsThrough)
{
    std::vector<std::string> log;

    const int value = ch05::call_logged(
        log, [](std::unique_ptr<int> owned) { return *owned; }, std::make_unique<int>(9));

    EXPECT_EQ(value, 9);
}

TEST(CallLogged, AReferenceResultStaysAReference)
{
    std::vector<std::string> log;
    g_storage = 1;

    // If call_logged returned a copy of the int, this would not compile, or would change the copy.
    static_assert(std::is_same_v<decltype(ch05::call_logged(log, stored)), int&>);
    ch05::call_logged(log, stored) = 42;

    EXPECT_EQ(g_storage, 42);
}

}  // namespace

// NOLINTEND(misc-const-correctness)
