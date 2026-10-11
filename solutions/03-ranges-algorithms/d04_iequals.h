#pragma once

#include <algorithm>
#include <string_view>

namespace ch03
{

/// @p c as a lower-case letter if it is an ASCII upper-case letter, otherwise @p c. (Given.)
[[nodiscard]] constexpr char to_lower_ascii(char c) noexcept
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

/// Whether @p a and @p b are the same text when the case of ASCII letters is ignored.
[[nodiscard]] constexpr bool iequals(std::string_view a, std::string_view b) noexcept
{
    // Two ranges, the default comparison (==), and one projection for each side. The range form of
    // equal compares the lengths as well: "war" is not equal to "warn".
    return std::ranges::equal(a, b, {}, to_lower_ascii, to_lower_ascii);
}

}  // namespace ch03
