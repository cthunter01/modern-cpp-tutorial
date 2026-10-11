#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace review_a
{

/// A list of entries.
class Journal
{
public:
    /// Adds @p entry and returns the number of characters that were added.
    std::size_t Add(std::string entry)
    {
        entries.push_back(std::move(entry));
        return entry.size();
    }

    [[nodiscard]] std::size_t Count() const noexcept { return entries.size(); }

private:
    std::vector<std::string> entries;
};

}  // namespace review_a
