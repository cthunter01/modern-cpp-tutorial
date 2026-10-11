#pragma once

#include <algorithm>
#include <string_view>

namespace ch01
{

/// Takes the first word off the front of @p rest and returns it. Blanks (space, tab) before the
/// word are skipped; afterwards @p rest starts directly behind the word. When there is no word
/// left, the result is empty and so is @p rest.
[[nodiscard]] constexpr std::string_view next_token(std::string_view& rest) noexcept
{
    constexpr std::string_view kBlanks = " \t";

    // min(): find_first_not_of returns npos when only blanks are left, and remove_prefix with more
    // than size() is undefined behaviour, not an empty result.
    rest.remove_prefix(std::min(rest.find_first_not_of(kBlanks), rest.size()));

    const std::string_view token =
        rest.substr(0, rest.find_first_of(kBlanks));  // substr clamps the count
    rest.remove_prefix(token.size());
    return token;
}

}  // namespace ch01
