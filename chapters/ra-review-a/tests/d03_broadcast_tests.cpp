#include "d03_broadcast.h"

#include <functional>
#include <vector>

#include <gtest/gtest.h>

#include "tut/tracked.h"

namespace
{

TEST(Broadcast, EveryListenerReceivesTheValue)
{
    std::vector<int> received;
    // Each listener takes the item by value: it wants one of its own.   NOLINTBEGIN(performance-unnecessary-value-param)
    const std::vector<std::function<void(tut::Tracked)>> listeners{
        [&received](tut::Tracked item) { received.push_back(item.Value()); },
        [&received](tut::Tracked item) { received.push_back(item.Value()); },
        [&received](tut::Tracked item) { received.push_back(item.Value()); },
    };
    // NOLINTEND(performance-unnecessary-value-param)

    review_a::broadcast(listeners, tut::Tracked{7});  // a temporary

    EXPECT_EQ(received, (std::vector<int>{7, 7, 7}));
}

TEST(Broadcast, PassesSeveralArguments)
{
    int                                              total = 0;
    const std::vector<std::function<void(int, int)>> listeners{
        [&total](int a, int b) { total += a + b; },
        [&total](int a, int b) { total += a * b; },
    };

    review_a::broadcast(listeners, 3, 4);

    EXPECT_EQ(total, 7 + 12);
}

}  // namespace
