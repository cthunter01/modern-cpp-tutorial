#pragma once

#include <cstddef>

#include "tut/todo.h"

namespace ch05
{

/// How many arguments were passed.
template <typename... Ts>
[[nodiscard]] constexpr std::size_t count_args(const Ts&... values) noexcept
{
    TUT_TODO(values...);
}

/// The sum of all arguments; 0 when there are none.
template <typename... Ts>
[[nodiscard]] constexpr auto sum_all(const Ts&... values) noexcept -> decltype((0 + ... + values))
{
    TUT_TODO(values...);
}

}  // namespace ch05
