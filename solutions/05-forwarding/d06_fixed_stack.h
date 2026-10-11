#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <utility>

namespace ch05
{

/// A stack of at most N elements that never allocates: its elements live inside the object.
template <typename T, std::size_t N>
class FixedStack
{
public:
    //! [emplace]
    /// Constructs a new top element from @p args, in place, and returns it. The stack must not be
    /// full.
    template <typename... Args>
    T& Emplace(Args&&... args)
    {
        // The arguments travel on to T's constructor untouched; no T is built here and then moved.
        T& created = slots.at(count).emplace(std::forward<Args>(args)...);
        ++count;
        return created;
    }
    //! [emplace]

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
    std::array<std::optional<T>, N> slots;
    std::size_t                     count = 0;
};

}  // namespace ch05
