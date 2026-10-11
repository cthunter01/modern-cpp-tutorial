#pragma once

#include <string_view>
#include <vector>

#include "d01_trim.h"
#include "d02_split.h"

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
    std::vector<KeyValue> pairs;
    for (const std::string_view piece : split(text, ';'))
    {
        const std::size_t equals = piece.find('=');
        if (equals == std::string_view::npos)
        {
            continue;
        }
        const std::string_view key = trim(piece.substr(0, equals));
        if (key.empty())
        {
            continue;
        }
        pairs.push_back({.key = key, .value = trim(piece.substr(equals + 1))});
    }
    return pairs;
}

}  // namespace ch01
