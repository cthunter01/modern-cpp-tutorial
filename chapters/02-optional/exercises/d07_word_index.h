#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>

#include "tut/todo.h"

namespace ch02
{

/// A text and the number of words in it. Counting is somebody else's job and is slow, so it is done
/// when the count is first asked for, and once.
class WordIndex
{
public:
    using Counter = std::function<std::size_t(std::string_view)>;

    WordIndex(std::string content, Counter countWords) { TUT_TODO(content, countWords); }

    /// The number of words, as the counter reported it.
    [[nodiscard]] std::size_t Count() { TUT_TODO(); }

private:
    // Your data members.
};

}  // namespace ch02
