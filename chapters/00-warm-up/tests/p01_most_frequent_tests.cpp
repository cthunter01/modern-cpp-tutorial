#include "p01_most_frequent.h"

#include <cstddef>
#include <string>
#include <vector>

#include <gtest/gtest.h>

// The tests fix only how most_frequent is called and that its result can be taken apart into a word
// and a count. Which types those are is yours to decide.

namespace
{

TEST(MostFrequent, FindsTheWordAndItsCount)
{
    const std::vector<std::string> words{"pear", "fig", "pear", "plum", "fig", "pear"};
    const auto [word, count] = ch00::most_frequent(words);
    EXPECT_EQ(word, "pear");
    EXPECT_EQ(static_cast<std::size_t>(count), 3U);
}

TEST(MostFrequent, TiesGoToTheAlphabeticallyFirstWord)
{
    const std::vector<std::string> words{"plum", "fig", "plum", "fig", "kiwi"};
    const auto [word, count] = ch00::most_frequent(words);
    EXPECT_EQ(word, "fig");
    EXPECT_EQ(static_cast<std::size_t>(count), 2U);
}

TEST(MostFrequent, OneWord)
{
    const std::vector<std::string> words{"fig"};
    const auto [word, count] = ch00::most_frequent(words);
    EXPECT_EQ(word, "fig");
    EXPECT_EQ(static_cast<std::size_t>(count), 1U);
}

}  // namespace
