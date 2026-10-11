#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace ch05
{

/// Notes the call in @p log, then calls @p function with @p args exactly as the caller could have:
/// every argument arrives as the lvalue or rvalue it was, and whatever the function returns is
/// returned, a reference as a reference.
template <typename F, typename... Args>
decltype(auto) call_logged(std::vector<std::string>& log, F&& function, Args&&... args)
{
    log.emplace_back("call");
    // std::forward<Args>(args)... expands to std::forward<A1>(a1), std::forward<A2>(a2), ...
    // decltype(auto) keeps the exact return type; plain `auto` would turn an int& into an int.
    return std::invoke(std::forward<F>(function), std::forward<Args>(args)...);
}

}  // namespace ch05
