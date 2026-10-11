#pragma once

#include <functional>

namespace ch05
{

//! [retry]
/// Calls `operation(args...)` up to @p attempts times, until it returns true. Returns whether it
/// did.
///
/// The arguments are passed to every attempt as lvalues, whatever the caller passed: an attempt may
/// fail, and the next one needs them intact. So nothing is forwarded here, on purpose.
template <typename F, typename... Args>
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward): on purpose, as the comment above says
bool retry(int attempts, F&& operation, Args&&... args)
{
    for (int attempt = 0; attempt < attempts; ++attempt)
    {
        if (std::invoke(operation, args...))  // not std::forward<Args>(args)...: see above
        {
            return true;
        }
    }
    return false;
}
//! [retry]

}  // namespace ch05
