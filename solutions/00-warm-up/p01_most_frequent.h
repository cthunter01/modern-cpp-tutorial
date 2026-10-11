#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace ch00
{

//! [result]
struct WordCount
{
    std::string word;
    std::size_t count = 0;
};
//! [result]

/// The word that occurs most often in @p words, and how often. Between words that are equally
/// frequent, the alphabetically first one wins. @p words must not be empty.
[[nodiscard]] inline WordCount most_frequent(const std::vector<std::string>& words)
{
    std::map<std::string, std::size_t> counts;
    for (const std::string& word : words)
    {
        ++counts[word];
    }

    WordCount best;
    // A map iterates in key order, so of two equally frequent words the alphabetically first is
    // seen first, and `>` (not `>=`) keeps it.
    for (const auto& [word, count] : counts)
    {
        if (count > best.count)
        {
            best = {.word = word, .count = count};
        }
    }
    return best;
}

}  // namespace ch00
