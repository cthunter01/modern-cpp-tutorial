#pragma once

#include <array>
#include <cstddef>
#include <optional>

#include "tut/todo.h"

namespace ch05
{

/// A stack of at most N elements that never allocates: its elements live inside the object.
template <typename T, std::size_t N>
class FixedStack
{
public:
    /// Constructs a new top element from @p args, in place, and returns it. The stack must not be
    /// full.
    template <typename... Args>
    T& Emplace(Args&&... args)
    {
        TUT_TODO(args...);
    }

    /// Removes the top element. The stack must not be empty.
    void Pop()
    {
        slots.at(count - 1).reset();
        --count;
    }

    // Every slot below `count` holds an element.   NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    [[nodiscard]] T&          Top() { return slots.at(count - 1).value(); }
    [[nodiscard]] std::size_t Size() const noexcept { return count; }

private:
    // Each slot either holds a T or is empty; std::optional has an emplace() of its own.
    std::array<std::optional<T>, N> slots;
    std::size_t                     count = 0;
};

}  // namespace ch05
