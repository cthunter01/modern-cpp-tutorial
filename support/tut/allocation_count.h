#pragma once

// Include this header in exactly ONE source file of a program (a test file). It replaces the global
// operator new and operator delete of that program, which the language allows once.

#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <new>

namespace tut
{

namespace detail
{
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables): a counter is the point
inline std::atomic<std::size_t> g_allocations{0};
}  // namespace detail

/// Counts the heap allocations made, by anyone, while it is alive:
///
///     const tut::AllocationCount allocations;
///     const auto result = function_under_test(input);
///     const std::size_t made = allocations.Count();   // read it before the EXPECT_ macros run
///
/// It is how a test can require "no allocation" or "one allocation" of code whose types it does not
/// know.
class AllocationCount
{
public:
    AllocationCount() noexcept : start(detail::g_allocations.load()) { }

    [[nodiscard]] std::size_t Count() const noexcept
    {
        return detail::g_allocations.load() - start;
    }

private:
    std::size_t start;
};

}  // namespace tut

// The complete set, not just the first two: whatever is left out is supplied by the standard
// library, or under AddressSanitizer by the sanitizer, and memory from one family must not be freed
// by the other.
//
// NOLINTBEGIN(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory,
// misc-new-delete-overloads,
//             readability-inconsistent-declaration-parameter-name,
//             bugprone-unhandled-exception-at-new)
namespace tut::detail
{

[[nodiscard]] inline void* allocate(std::size_t size, std::size_t alignment) noexcept
{
    g_allocations.fetch_add(1);
    if (alignment <= alignof(std::max_align_t))
    {
        return std::malloc(size == 0 ? 1 : size);
    }
    // aligned_alloc wants a size that is a multiple of the alignment.
    const std::size_t rounded = (size + alignment - 1) / alignment * alignment;
    return std::aligned_alloc(alignment, rounded == 0 ? alignment : rounded);
}

[[nodiscard]] inline void* allocate_or_throw(std::size_t size, std::size_t alignment)
{
    if (void* memory = allocate(size, alignment))
    {
        return memory;
    }
    throw std::bad_alloc();
}

}  // namespace tut::detail

void* operator new(std::size_t size)
{
    return tut::detail::allocate_or_throw(size, alignof(std::max_align_t));
}
void* operator new[](std::size_t size)
{
    return tut::detail::allocate_or_throw(size, alignof(std::max_align_t));
}
void* operator new(std::size_t size, std::align_val_t alignment)
{
    return tut::detail::allocate_or_throw(size, static_cast<std::size_t>(alignment));
}
void* operator new[](std::size_t size, std::align_val_t alignment)
{
    return tut::detail::allocate_or_throw(size, static_cast<std::size_t>(alignment));
}
void* operator new(std::size_t size, const std::nothrow_t& /*tag*/) noexcept
{
    return tut::detail::allocate(size, alignof(std::max_align_t));
}
void* operator new[](std::size_t size, const std::nothrow_t& /*tag*/) noexcept
{
    return tut::detail::allocate(size, alignof(std::max_align_t));
}
void* operator new(std::size_t size, std::align_val_t alignment,
                   const std::nothrow_t& /*tag*/) noexcept
{
    return tut::detail::allocate(size, static_cast<std::size_t>(alignment));
}
void* operator new[](std::size_t size, std::align_val_t alignment,
                     const std::nothrow_t& /*tag*/) noexcept
{
    return tut::detail::allocate(size, static_cast<std::size_t>(alignment));
}

void operator delete(void* memory) noexcept
{
    std::free(memory);
}
void operator delete[](void* memory) noexcept
{
    std::free(memory);
}
void operator delete(void* memory, std::size_t /*size*/) noexcept
{
    std::free(memory);
}
void operator delete[](void* memory, std::size_t /*size*/) noexcept
{
    std::free(memory);
}
void operator delete(void* memory, std::align_val_t /*alignment*/) noexcept
{
    std::free(memory);
}
void operator delete[](void* memory, std::align_val_t /*alignment*/) noexcept
{
    std::free(memory);
}
void operator delete(void* memory, std::size_t /*size*/, std::align_val_t /*alignment*/) noexcept
{
    std::free(memory);
}
void operator delete[](void* memory, std::size_t /*size*/, std::align_val_t /*alignment*/) noexcept
{
    std::free(memory);
}
void operator delete(void* memory, const std::nothrow_t& /*tag*/) noexcept
{
    std::free(memory);
}
void operator delete[](void* memory, const std::nothrow_t& /*tag*/) noexcept
{
    std::free(memory);
}
void operator delete(void* memory, std::align_val_t /*alignment*/,
                     const std::nothrow_t& /*tag*/) noexcept
{
    std::free(memory);
}
void operator delete[](void* memory, std::align_val_t /*alignment*/,
                       const std::nothrow_t& /*tag*/) noexcept
{
    std::free(memory);
}
// NOLINTEND(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory,
// misc-new-delete-overloads,
//           readability-inconsistent-declaration-parameter-name,
//           bugprone-unhandled-exception-at-new)
