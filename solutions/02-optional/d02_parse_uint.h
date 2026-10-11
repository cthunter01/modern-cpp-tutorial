#pragma once

#include <charconv>
#include <optional>
#include <string_view>
#include <system_error>

namespace ch02
{

/// @p text as an unsigned decimal number. Nothing unless all of @p text is digits and the value
/// fits: no sign, no blanks, no trailing characters.
[[nodiscard]] inline std::optional<unsigned> parse_uint(std::string_view text) noexcept
{
    unsigned          value = 0;
    const char* const first = text.data();
    // from_chars takes two pointers: where the characters start and where they end.
    const char* const last        = first + text.size();  // NOLINT(*-pointer-arithmetic)
    const auto [stoppedAt, error] = std::from_chars(first, last, value);
    // from_chars stops at the first character that is not part of a number and calls that success:
    // "12x" gives 12. Demanding that it stopped at the end is what rejects it. An empty text is an
    // error on its own account (invalid_argument), and so is a value that does not fit.
    if (error != std::errc{} || stoppedAt != last)
    {
        return std::nullopt;
    }
    return value;
}

}  // namespace ch02
