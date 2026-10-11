#pragma once

#include "tut/todo.h"

namespace ch04
{

/// Adds one to a counter for as long as it lives. Can be moved, so that it can be returned from a
/// function and kept in a container; cannot be copied, because two copies would subtract twice.
class ScopedCount
{
public:
    explicit ScopedCount(int& target) noexcept { TUT_TODO(target); }

    ~ScopedCount() { TUT_TODO(); }

    ScopedCount(ScopedCount&& other) noexcept { TUT_TODO(other); }
    ScopedCount& operator=(ScopedCount&& other) noexcept { TUT_TODO(other); }

private:
    // Your data member. What does a ScopedCount that has been moved from hold?
};

}  // namespace ch04
