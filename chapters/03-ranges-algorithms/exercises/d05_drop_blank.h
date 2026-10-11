#pragma once

#include <string>
#include <vector>

#include "tut/todo.h"

namespace ch03
{

/// Removes the lines that are empty or consist of spaces and tabs only. The others keep their
/// order.
inline void drop_blank(std::vector<std::string>& lines)
{
    TUT_TODO(lines);
}

}  // namespace ch03
