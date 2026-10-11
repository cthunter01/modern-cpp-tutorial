#pragma once

#include <string_view>
#include <vector>

// Your own trim() and split() from drills 1 and 2. Use them.
#include "d01_trim.h"
#include "d02_split.h"
#include "tut/todo.h"

namespace ch01
{

struct KeyValue
{
    std::string_view key;
    std::string_view value;
};

/// The key=value pairs of a text such as "a=1; b = 2", separated by ';'. Keys and values are
/// trimmed. A piece without '=' or with an empty key is skipped; an empty value is allowed. Keys
/// and values are parts of @p text.
[[nodiscard]] inline std::vector<KeyValue> parse_pairs(std::string_view text)
{
    TUT_TODO(text);
}

}  // namespace ch01
