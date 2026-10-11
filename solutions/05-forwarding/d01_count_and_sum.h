#pragma once

#include <cstddef>

namespace ch05
{

/// How many arguments were passed.
template <typename... Ts>
[[nodiscard]] constexpr std::size_t count_args(const Ts&... /*unused*/) noexcept
{
    return sizeof...(Ts);  // the size of the pack: a number the compiler knows
}

/// The sum of all arguments; 0 when there are none.
template <typename... Ts>
[[nodiscard]] constexpr auto sum_all(const Ts&... values) noexcept
{
    // A fold: the pack, one operator, and a value to start from. For (a, b, c) this is
    // ((0 + a) + b) + c, and for an empty pack it is just the 0.
    return (0 + ... + values);
}

}  // namespace ch05
