#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace ch02
{

/// A text and the number of words in it. Counting is somebody else's job and is slow, so it is done
/// when the count is first asked for, and once.
class WordIndex
{
public:
    using Counter = std::function<std::size_t(std::string_view)>;

    WordIndex(std::string content, Counter countWords)
      : text(std::move(content)), counter(std::move(countWords))
    {
    }

    /// The number of words, as @p counter reported it.
    [[nodiscard]] std::size_t Count()
    {
        // "Not counted yet" is a state of its own. A plain std::size_t would have to borrow one of
        // its values for it, and 0 is taken: an empty text has 0 words and would be counted again
        // and again.
        if (!cached)
        {
            cached = counter(text);
        }
        return *cached;
    }

private:
    std::string                text;
    Counter                    counter;
    std::optional<std::size_t> cached;
};

}  // namespace ch02
