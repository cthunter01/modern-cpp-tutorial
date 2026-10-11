#pragma once

#include <optional>

namespace review_a
{

/// Whether output should be coloured: what the user's setting says if there is one, otherwise
/// whether the terminal supports colour.
[[nodiscard]] constexpr bool use_colour(std::optional<bool> setting,
                                        bool                terminalSupportsColour) noexcept
{
    // The bug was `if (setting) { return true; }`: it answered "is there a setting?" where the
    // question was "what does it say?". A user who set colour to false got colour.
    return setting.value_or(terminalSupportsColour);
}

}  // namespace review_a
