#pragma once

#include <functional>
#include <utility>
#include <vector>

namespace review_a
{

/// Calls every one of @p listeners with @p args.
template <typename... Params, typename... Args>
void broadcast(const std::vector<std::function<void(Params...)>>& listeners, Args&&... args)
{
    for (const auto& listener : listeners)
    {
        listener(std::forward<Args>(args)...);
    }
}

}  // namespace review_a
