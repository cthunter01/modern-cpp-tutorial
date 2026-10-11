#pragma once

#include <string_view>

#include "tut/todo.h"

namespace ch01
{

/// Takes the first word off the front of @p rest and returns it. Blanks (space, tab) before the
/// word are skipped; afterwards @p rest starts directly behind the word. When there is no word
/// left, the result is empty and so is @p rest.
[[nodiscard]] constexpr std::string_view next_token(std::string_view& rest) noexcept
{
    TUT_TODO(rest);
}

}  // namespace ch01
