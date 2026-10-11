#include "p01_retry.h"

#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

// Whether a variable is const decides what is deduced from it and whether it can be moved from, so
// the variables in this file are declared the way each test needs them.
//
// NOLINTBEGIN(misc-const-correctness)

#include "tut/tracked.h"

// The tests fix one call: ch05::retry(attempts, operation, arguments...). It calls
// operation(arguments...) until that returns true, at most `attempts` times, and returns whether it
// did. How retry is declared is yours to decide; the tests look at what happens to the arguments.

namespace
{

TEST(Retry, StopsAtTheFirstSuccess)
{
    int calls = 0;

    const bool succeeded = ch05::retry(5, [&calls] { return ++calls == 3; });

    EXPECT_TRUE(succeeded);
    EXPECT_EQ(calls, 3);
}

TEST(Retry, GivesUpAfterTheGivenNumberOfAttempts)
{
    int calls = 0;

    const bool succeeded = ch05::retry(4, [&calls] {
        ++calls;
        return false;
    });

    EXPECT_FALSE(succeeded);
    EXPECT_EQ(calls, 4);
}

TEST(Retry, PassesTheArgumentsToEveryAttempt)
{
    std::vector<int> seen;

    const bool succeeded = ch05::retry(
        3,
        [&seen](int port, const std::string& host) {
            seen.push_back(port);
            return host == "never";
        },
        8080, std::string("example.org"));

    EXPECT_FALSE(succeeded);
    EXPECT_EQ(seen, (std::vector<int>{8080, 8080, 8080}));
}

TEST(Retry, TheOperationCanChangeAnArgumentOfTheCaller)
{
    int progress = 0;

    const bool succeeded = ch05::retry(
        10,
        [](int& done) {
            done += 25;
            return done >= 100;
        },
        progress);

    EXPECT_TRUE(succeeded);
    EXPECT_EQ(progress, 100) << "each attempt has to work on the caller's variable, not on a copy";
}

TEST(Retry, AnArgumentTheCallerGaveUpIsStillWholeOnTheSecondAttempt)
{
    // The payload is expensive, and the caller hands it over with std::move. The first two attempts
    // fail. The third must still find the payload there: 7, not the remains of a moved-from object.
    tut::Tracked     payload{7};
    std::vector<int> seen;
    tut::Tracked::ResetCounts();

    const bool succeeded = ch05::retry(
        3,
        [&seen](const tut::Tracked& item) {
            seen.push_back(item.Value());
            return seen.size() == 3;
        },
        std::move(payload));

    EXPECT_TRUE(succeeded);
    EXPECT_EQ(seen, (std::vector<int>{7, 7, 7}));
    EXPECT_EQ(tut::Tracked::Counts().copies, 0) << "retry itself must not add copies";
}

TEST(Retry, AnOperationThatTakesItsArgumentByValueGetsACopyEachTime)
{
    // This operation consumes what it is given. Retrying it three times needs the payload three
    // times, so here copies are right, and a move would leave nothing for the second attempt.
    const tut::Tracked payload{7};
    std::vector<int>   seen;

    const bool succeeded = ch05::retry(
        3,
        [&seen](tut::Tracked item) {  // NOLINT(performance-unnecessary-value-param): the point
            seen.push_back(item.Value());
            return false;
        },
        payload);

    EXPECT_FALSE(succeeded);
    EXPECT_EQ(seen, (std::vector<int>{7, 7, 7}));
}

}  // namespace

// NOLINTEND(misc-const-correctness)
