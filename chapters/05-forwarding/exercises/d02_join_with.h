#pragma once

#include <string>
#include <string_view>

#include "tut/todo.h"

namespace ch05
{

/// The arguments written one after the other, as operator<< writes them, with @p separator between
/// them.
template <typename... Ts>
[[nodiscard]] std::string join_with(std::string_view separator, const Ts&... items)
{
    TUT_TODO(separator, items...);
}

}  // namespace ch05
