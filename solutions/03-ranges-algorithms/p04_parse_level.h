#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <string_view>

#include "level.h"

namespace ch03
{

namespace detail
{

constexpr char to_lower_ascii(char c) noexcept
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

}  // namespace detail

//! [parse-level]
/// The level whose name is @p text, ignoring the case of ASCII letters, or nothing.
[[nodiscard]] constexpr std::optional<Level> parse_level(std::string_view text) noexcept
{
    constexpr std::array kLevels{
        Level::Off, Level::Error, Level::Warn, Level::Info, Level::Debug, Level::Perf,
    };
    for (const Level level : kLevels)
    {
        if (std::ranges::equal(text, level_name(level), {}, detail::to_lower_ascii,
                               detail::to_lower_ascii))
        {
            return level;
        }
    }
    return std::nullopt;
}
//! [parse-level]

}  // namespace ch03
