#pragma once

#include <string_view>

#include "tut/todo.h"

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
    TUT_TODO(a, b);
}

}  // namespace ch03
