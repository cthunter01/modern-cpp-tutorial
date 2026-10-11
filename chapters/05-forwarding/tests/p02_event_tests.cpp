#include "p02_event.h"

#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

// The tests fix a class template ch05::Event<Ts...>, where Ts are the types of the values an event
// carries, with
//   Subscribe(listener)    listener is anything callable with the values
//   Emit(values...)        calls every listener, in the order they subscribed

namespace
{

TEST(Event, CarriesItsValuesToAListener)
{
    ch05::Event<int, int> clicked;
    int                   sum = 0;
    clicked.Subscribe([&sum](int x, int y) { sum = x + y; });

    clicked.Emit(3, 4);

    EXPECT_EQ(sum, 7);
}

TEST(Event, CallsEveryListenerInOrder)
{
    ch05::Event<int>         tick;
    std::vector<std::string> order;
    tick.Subscribe([&order](int n) { order.push_back("first " + std::to_string(n)); });
    tick.Subscribe([&order](int n) { order.push_back("second " + std::to_string(n)); });

    tick.Emit(1);
    tick.Emit(2);

    EXPECT_EQ(order, (std::vector<std::string>{"first 1", "second 1", "first 2", "second 2"}));
}

TEST(Event, EveryListenerSeesTheWholeValue)
{
    ch05::Event<std::string> changed;
    std::vector<std::string> received;
    // Both listeners take the text by value: each wants a string of its own to keep.
    changed.Subscribe([&received](std::string text) { received.push_back(std::move(text)); });
    changed.Subscribe([&received](std::string text) { received.push_back(std::move(text)); });

    changed.Emit(std::string(100, 'x'));  // a temporary: tempting to hand on with a move

    ASSERT_EQ(received.size(), 2U);
    EXPECT_EQ(received[0].size(), 100U);
    EXPECT_EQ(received[1].size(), 100U)
        << "the first listener was given the text to keep; the second got what was left";
}

TEST(Event, AnEventWithoutValuesAndAnEventWithoutListeners)
{
    ch05::Event<> closed;
    int           calls = 0;
    closed.Subscribe([&calls] { ++calls; });
    closed.Emit();
    closed.Emit();
    EXPECT_EQ(calls, 2);

    const ch05::Event<int, std::string> nobodyListens;
    nobodyListens.Emit(1, "unheard");
}

TEST(Event, TheCallersVariableIsNotConsumed)
{
    ch05::Event<std::string> changed;
    std::string              last;
    changed.Subscribe([&last](const std::string& text) { last = text; });
    const std::string title = "a title long enough to live on the heap, to be sure";

    changed.Emit(title);

    EXPECT_EQ(last, title);
}

}  // namespace
