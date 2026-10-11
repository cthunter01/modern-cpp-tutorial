#include "d09_tee.h"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace
{

/// Remembers what was written to it, and adds every write to a list shared by all sinks.
class Memory
{
public:
    Memory(std::string sinkName, std::vector<std::string>& sharedOrder)
      : name(std::move(sinkName)), order(&sharedOrder)
    {
    }

    void Write(std::string_view line)
    {
        lines.emplace_back(line);
        order->push_back(name);
    }

    [[nodiscard]] const std::vector<std::string>& Lines() const noexcept { return lines; }

private:
    std::string               name;
    std::vector<std::string>* order;
    std::vector<std::string>  lines;
};

/// A second kind of sink: it only counts.
class Counter
{
public:
    void              Write(std::string_view /*unused*/) { ++count; }
    [[nodiscard]] int Count() const noexcept { return count; }

private:
    int count = 0;
};

TEST(Tee, WritesEveryLineToEverySink)
{
    std::vector<std::string> order;
    Memory                   first("first", order);
    Memory                   second("second", order);
    ch05::Tee                tee(first, second);

    tee.Write("one");
    tee.Write("two");

    EXPECT_EQ(first.Lines(), (std::vector<std::string>{"one", "two"}));
    EXPECT_EQ(second.Lines(), (std::vector<std::string>{"one", "two"}));
}

TEST(Tee, CallsTheSinksInTheOrderTheyWereGiven)
{
    std::vector<std::string> order;
    Memory                   a("a", order);
    Memory                   b("b", order);
    Memory                   c("c", order);
    ch05::Tee                tee(a, b, c);

    tee.Write("line");

    EXPECT_EQ(order, (std::vector<std::string>{"a", "b", "c"}));
}

TEST(Tee, TheSinksMayBeOfDifferentTypes)
{
    std::vector<std::string> order;
    Memory                   memory("memory", order);
    Counter                  counter;
    ch05::Tee                tee(memory, counter);

    tee.Write("one");
    tee.Write("two");

    EXPECT_EQ(counter.Count(), 2);
    EXPECT_EQ(memory.Lines().size(), 2U);
}

TEST(Tee, OneSinkOrNoneIsFineToo)
{
    Counter   counter;
    ch05::Tee single(counter);
    single.Write("line");
    EXPECT_EQ(counter.Count(), 1);

    ch05::Tee<> none;
    none.Write("into the void");

    // The same tee as in the first test, once more, so that this case fails with the others on a
    // stub.
    std::vector<std::string> order;
    Memory                   memory("memory", order);
    ch05::Tee                tee(memory, counter);
    tee.Write("x");
    EXPECT_EQ(counter.Count(), 2);
}

}  // namespace
