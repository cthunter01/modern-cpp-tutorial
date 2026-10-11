#pragma once

#include "tut/todo.h"

namespace ch05
{

/// Whether @p value equals at least one of @p candidates. False when there are none.
template <typename T, typename... Us>
[[nodiscard]] constexpr bool is_one_of(const T& value, const Us&... candidates) noexcept
{
    TUT_TODO(value, candidates...);
}

}  // namespace ch05
