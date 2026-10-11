#pragma once

#include <utility>

namespace ch00
{

/// A flag that one part of a program sets and another part collects.
class Toggle
{
public:
    void Set() noexcept { on = true; }

    /// Whether the toggle was set since the last call. Clears it.
    [[nodiscard]] bool TakeAndReset() noexcept { return std::exchange(on, false); }

private:
    bool on = false;
};

}  // namespace ch00
