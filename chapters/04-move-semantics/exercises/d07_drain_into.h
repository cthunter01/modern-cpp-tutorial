#pragma once

#include <string>
#include <vector>

#include "tut/todo.h"

namespace ch04
{

/// Appends every string of @p from to @p to without copying its characters, and leaves @p from
/// empty.
inline void drain_into(std::vector<std::string>& from, std::vector<std::string>& to)
{
    TUT_TODO(from, to);
}

}  // namespace ch04
