#pragma once

#include <string_view>
#include <vector>

namespace ch01
{

/// The pieces of @p text between occurrences of @p delimiter. Empty pieces are kept, so "a,,b" has
/// three pieces and "" has one. Every piece is a part of @p text.
[[nodiscard]] inline std::vector<std::string_view> split(std::string_view text, char delimiter)
{
    std::vector<std::string_view> pieces;
    while (true)
    {
        const std::size_t at = text.find(delimiter);
        if (at == std::string_view::npos)
        {
            pieces.push_back(text);
            return pieces;
        }
        pieces.push_back(text.substr(0, at));
        text.remove_prefix(at + 1);  // at < size(), so at + 1 <= size(): always in range
    }
}

}  // namespace ch01
