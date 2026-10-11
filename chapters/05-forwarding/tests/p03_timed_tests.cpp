#include "p03_timed.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

// The tests fix one call: ch05::timed(timings, label, function, arguments...). It calls
// function(arguments...) and returns exactly what that returns. Before the call it appends
// "<label>: begin" to timings, a std::vector<std::string>, and after it "<label>: end".

namespace
{

using Timings = std::vector<std::string>;

// What slot() returns.   NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
int g_slot = 0;

int& slot()
{
    return g_slot;
}

TEST(Timed, RecordsBeginAndEndAroundTheCall)
{
    Timings timings;

    const int result = ch05::timed(
        timings, "add",
        [&timings](int a, int b) {
            timings.emplace_back("working");
            return a + b;
        },
        2, 3);

    EXPECT_EQ(result, 5);
    EXPECT_EQ(timings, (Timings{"add: begin", "working", "add: end"}));
}

TEST(Timed, WorksForAFunctionThatReturnsNothing)
{
    Timings timings;
    int     calls = 0;

    ch05::timed(timings, "notify", [&calls] { ++calls; });

    EXPECT_EQ(calls, 1);
    EXPECT_EQ(timings, (Timings{"notify: begin", "notify: end"}));
}

TEST(Timed, AReferenceResultStaysAReference)
{
    Timings timings;
    g_slot = 1;

    static_assert(std::is_same_v<decltype(ch05::timed(timings, "slot", slot)), int&>);
    ch05::timed(timings, "slot", slot) = 42;

    EXPECT_EQ(g_slot, 42);
    EXPECT_EQ(timings.back(), "slot: end") << "the end has to be recorded before timed() returns";
}

TEST(Timed, PassesArgumentsOnAsTheyWere)
{
    Timings timings;
    int     counter = 0;

    ch05::timed(timings, "bump", [](int& value) { ++value; }, counter);
    const int owned = ch05::timed(
        timings, "own", [](std::unique_ptr<int> pointer) { return *pointer; },
        std::make_unique<int>(9));

    EXPECT_EQ(counter, 1);
    EXPECT_EQ(owned, 9);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): EXPECT_THROW expands to a good deal
TEST(Timed, RecordsTheEndAlsoWhenTheFunctionThrows)
{
    Timings timings;

    EXPECT_THROW(ch05::timed(timings, "fail", [] { throw std::runtime_error("no"); }),
                 std::runtime_error);

    EXPECT_EQ(timings, (Timings{"fail: begin", "fail: end"}));
}

}  // namespace
