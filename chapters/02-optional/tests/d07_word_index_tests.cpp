#include "d07_word_index.h"

#include <cstddef>
#include <functional>
#include <string_view>

#include <gtest/gtest.h>

namespace
{

/// Counts words (separated by single spaces), and counts how often it was asked to.
class SlowCounter
{
public:
    [[nodiscard]] std::size_t operator()(std::string_view text)
    {
        ++calls;
        if (text.empty())
        {
            return 0;
        }
        std::size_t words = 1;
        for (const char c : text)
        {
            words += (c == ' ') ? 1U : 0U;
        }
        return words;
    }

    [[nodiscard]] int Calls() const noexcept { return calls; }

private:
    int calls = 0;
};

TEST(WordIndex, ReportsWhatTheCounterSays)
{
    SlowCounter     counter;
    ch02::WordIndex index("one two three", std::ref(counter));

    EXPECT_EQ(index.Count(), 3U);
}

TEST(WordIndex, DoesNotCountUntilAsked)
{
    SlowCounter           counter;
    const ch02::WordIndex index("one two three", std::ref(counter));

    EXPECT_EQ(counter.Calls(), 0);
}

TEST(WordIndex, CountsOnlyOnce)
{
    SlowCounter     counter;
    ch02::WordIndex index("one two three", std::ref(counter));

    EXPECT_EQ(index.Count(), 3U);
    EXPECT_EQ(index.Count(), 3U);
    EXPECT_EQ(index.Count(), 3U);
    EXPECT_EQ(counter.Calls(), 1);
}

TEST(WordIndex, CountsAnEmptyTextOnlyOnceToo)
{
    SlowCounter     counter;
    ch02::WordIndex index("", std::ref(counter));

    EXPECT_EQ(index.Count(), 0U);
    EXPECT_EQ(index.Count(), 0U);
    EXPECT_EQ(index.Count(), 0U);
    // Zero words is a result. "Have I counted?" and "how many were there?" are two questions.
    EXPECT_EQ(counter.Calls(), 1);
}

}  // namespace
