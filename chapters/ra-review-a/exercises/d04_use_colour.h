#pragma once

#include <optional>

namespace review_a
{

/// Whether output should be coloured: what the user's setting says if there is one, otherwise
/// whether the terminal supports colour.
[[nodiscard]] constexpr bool use_colour(std::optional<bool> setting,
                                        bool                terminalSupportsColour) noexcept
{
    if (setting)
    {
        return true;
    }
    return terminalSupportsColour;
}

}  // namespace review_a
