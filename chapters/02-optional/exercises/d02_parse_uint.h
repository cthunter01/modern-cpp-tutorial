#pragma once

#include <optional>
#include <string_view>

#include "tut/todo.h"

namespace ch02
{

/// @p text as an unsigned decimal number. Nothing unless all of @p text is digits and the value
/// fits: no sign, no blanks, no trailing characters.
[[nodiscard]] inline std::optional<unsigned> parse_uint(std::string_view text) noexcept
{
    TUT_TODO(text);
}

}  // namespace ch02
