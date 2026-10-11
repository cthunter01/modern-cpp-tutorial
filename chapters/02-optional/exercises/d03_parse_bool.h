#pragma once

#include <optional>
#include <string_view>

#include "tut/todo.h"

namespace ch02
{

/// "true" or "on" give true, "false" or "off" give false, exactly as written here. Anything else
/// gives nothing.
[[nodiscard]] constexpr std::optional<bool> parse_bool(std::string_view text) noexcept
{
    TUT_TODO(text);
}

}  // namespace ch02
