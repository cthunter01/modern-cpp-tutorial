#pragma once

#include <optional>
#include <string_view>

namespace ch02
{

/// "true" or "on" give true, "false" or "off" give false, exactly as written here. Anything else
/// gives nothing.
[[nodiscard]] constexpr std::optional<bool> parse_bool(std::string_view text) noexcept
{
    if (text == "true" || text == "on")
    {
        return true;
    }
    if (text == "false" || text == "off")
    {
        return false;  // an answer: engaged, and the answer is false
    }
    return std::nullopt;  // no answer
}

}  // namespace ch02
