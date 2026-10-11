#pragma once

#include <utility>

namespace ch04
{

/// Adds one to a counter for as long as it lives. Can be moved, so that it can be returned from a
/// function and kept in a container; cannot be copied, because two copies would subtract twice.
class ScopedCount
{
public:
    explicit ScopedCount(int& target) noexcept : counter(&target) { ++*counter; }

    ~ScopedCount() { Release(); }

    ScopedCount(const ScopedCount&)            = delete;
    ScopedCount& operator=(const ScopedCount&) = delete;

    //! [move]
    // A moved-from ScopedCount holds nullptr: it has handed its duty on, and its destructor does
    // nothing.
    ScopedCount(ScopedCount&& other) noexcept : counter(std::exchange(other.counter, nullptr)) { }

    ScopedCount& operator=(ScopedCount&& other) noexcept
    {
        if (this != &other)
        {
            Release();  // whatever *this was counting stops being counted
            counter = std::exchange(other.counter, nullptr);
        }
        return *this;
    }
    //! [move]

private:
    void Release() noexcept
    {
        if (counter != nullptr)
        {
            --*counter;
            counter = nullptr;
        }
    }

    int* counter;
};

}  // namespace ch04
