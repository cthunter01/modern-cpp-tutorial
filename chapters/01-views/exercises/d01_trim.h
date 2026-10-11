#pragma once

#include <string_view>

#include "tut/todo.h"

namespace ch01
{

/// @p text without the blanks (space, tab, carriage return, newline) at its two ends. The result is
/// a part of @p text: it is valid for as long as the characters @p text refers to are.
[[nodiscard]] constexpr std::string_view trim(std::string_view text) noexcept
{
    TUT_TODO(text);
}

}  // namespace ch01
