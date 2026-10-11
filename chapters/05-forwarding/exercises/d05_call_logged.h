#pragma once

#include <string>
#include <vector>

#include "tut/todo.h"

namespace ch05
{

/// Notes the call in @p log (appends the word "call"), then calls @p function with @p args exactly
/// as the caller could have: every argument arrives as the lvalue or rvalue it was, and whatever
/// the function returns is returned, a reference as a reference.
///
/// The return type is part of the exercise. `void` is only what makes the stub compile.
template <typename F, typename... Args>
void call_logged(std::vector<std::string>& log, F&& function, Args&&... args)
{
    TUT_TODO(log, function, args...);
}

}  // namespace ch05
