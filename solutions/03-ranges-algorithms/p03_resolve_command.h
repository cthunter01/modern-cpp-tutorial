#pragma once

#include <algorithm>
#include <cstdint>
#include <expected>
#include <span>
#include <string_view>

namespace ch03
{

enum class ResolveError : std::uint8_t
{
    Unknown,    ///< no command starts with what was typed
    Ambiguous,  ///< more than one does, and none is an exact match
};

using Resolved = std::expected<std::string_view, ResolveError>;

namespace detail
{

constexpr char lower(char c) noexcept
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

}  // namespace detail

/// The one of @p commands that @p typed stands for: the command that equals it, or else the only
/// command that starts with it, both ignoring the case of ASCII letters. The result is one of the
/// elements of @p commands.
[[nodiscard]] constexpr Resolved resolve(std::span<const std::string_view> commands,
                                         std::string_view                  typed) noexcept
{
    //! [resolve]
    const auto isExactly = [typed](std::string_view command) {
        return std::ranges::equal(command, typed, {}, detail::lower, detail::lower);
    };
    const auto startsWith = [typed](std::string_view command) {
        return std::ranges::starts_with(command, typed, {}, detail::lower, detail::lower);
    };

    // An exact match wins, even when it is also the beginning of a longer command.
    if (const auto exact = std::ranges::find_if(commands, isExactly); exact != commands.end())
    {
        return *exact;
    }
    switch (std::ranges::count_if(commands, startsWith))
    {
        case 0:
            return std::unexpected(ResolveError::Unknown);
        case 1:
            return *std::ranges::find_if(commands, startsWith);
        default:
            return std::unexpected(ResolveError::Ambiguous);
    }
    //! [resolve]
}

/// What to tell the user when resolve() chose no command. Empty for a result that is a command.
[[nodiscard]] constexpr std::string_view explain(const Resolved& result) noexcept
{
    if (result)
    {
        return {};
    }
    return result.error() == ResolveError::Unknown ? "unknown command" : "ambiguous command";
}

}  // namespace ch03
