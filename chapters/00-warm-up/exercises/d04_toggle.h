#pragma once

#include "tut/todo.h"

namespace ch00
{

/// A flag that one part of a program sets and another part collects.
class Toggle
{
public:
    void Set() noexcept { on = true; }

    /// Whether the toggle was set since the last call. Clears it.
    [[nodiscard]] bool TakeAndReset() noexcept { TUT_TODO(); }

private:
    bool on = false;
};

}  // namespace ch00
