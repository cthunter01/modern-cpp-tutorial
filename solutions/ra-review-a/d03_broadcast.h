#pragma once

#include <functional>
#include <vector>

namespace review_a
{

/// Calls every one of @p listeners with @p args.
template <typename... Params, typename... Args>
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward): on purpose, see below
void broadcast(const std::vector<std::function<void(Params...)>>& listeners, Args&&... args)
{
    for (const auto& listener : listeners)
    {
        // The bug was std::forward<Args>(args)... here. Forwarding hands an rvalue argument to the
        // first listener to be moved from, and the second listener gets what is left. Arguments
        // that are used more than once are passed as lvalues every time.
        listener(args...);
    }
}

}  // namespace review_a
