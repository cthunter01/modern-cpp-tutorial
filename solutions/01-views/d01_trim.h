#pragma once

#include <string_view>

namespace ch01
{

/// @p text without the blanks (space, tab, carriage return, newline) at its two ends. The result is
/// a part of @p text: it is valid for as long as the characters @p text refers to are.
[[nodiscard]] constexpr std::string_view trim(std::string_view text) noexcept
{
    constexpr std::string_view kBlanks = " \t\r\n";

    const std::size_t first = text.find_first_not_of(kBlanks);
    if (first == std::string_view::npos)
    {
        return {};  // nothing but blanks
    }
    const std::size_t last = text.find_last_not_of(kBlanks);
    return text.substr(first, last - first + 1);
}

}  // namespace ch01
