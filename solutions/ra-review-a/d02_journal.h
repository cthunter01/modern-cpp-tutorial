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
        // The bug read entry.size() after the std::move: the size of a moved-from string, which
        // with this library is 0. Whatever is needed from an object has to be taken before it is
        // given away.
        const std::size_t added = entry.size();
        entries.push_back(std::move(entry));
        return added;
    }

    [[nodiscard]] std::size_t Count() const noexcept { return entries.size(); }

private:
    std::vector<std::string> entries;
};

}  // namespace review_a
