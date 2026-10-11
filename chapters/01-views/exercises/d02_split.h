#pragma once

#include <string_view>
#include <vector>

#include "tut/todo.h"

namespace ch01
{

/// The pieces of @p text between occurrences of @p delimiter. Empty pieces are kept, so "a,,b" has
/// three pieces and "" has one. Every piece is a part of @p text.
[[nodiscard]] inline std::vector<std::string_view> split(std::string_view text, char delimiter)
{
    TUT_TODO(text, delimiter);
}

}  // namespace ch01
