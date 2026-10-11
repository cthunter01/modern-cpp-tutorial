#pragma once

namespace ch05
{

/// Whether @p value equals at least one of @p candidates. False when there are none.
template <typename T, typename... Us>
[[nodiscard]] constexpr bool is_one_of(const T& value, const Us&... candidates) noexcept
{
    // A fold over ||. For an empty pack a fold over || is false, so no start value is needed.
    return ((value == candidates) || ...);
}

}  // namespace ch05
