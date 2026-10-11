#pragma once

#include <cstddef>

#include "tut/todo.h"

namespace ch04
{

// This class manages memory by hand, which is what the rule of five is about. In real code a
// std::vector<int> member would do all of it, and that is the rule of zero.
// NOLINTBEGIN(cppcoreguidelines-owning-memory)

/// A block of ints of a fixed size, on the heap. A Buffer that has been moved from is empty: Size()
/// is 0 and Data() is nullptr. It can be destroyed or assigned to, like any other.
class Buffer
{
public:
    explicit Buffer(std::size_t count) : data(count == 0 ? nullptr : new int[count]{}), size(count)
    {
    }

    ~Buffer() { delete[] data; }

    // The four operations to write. As they stand, each of them refuses to work.
    Buffer(const Buffer& other) { TUT_TODO(other); }
    Buffer& operator=(const Buffer& other) { TUT_TODO(other); }
    Buffer(Buffer&& other) noexcept { TUT_TODO(other); }
    Buffer& operator=(Buffer&& other) noexcept { TUT_TODO(other); }

    [[nodiscard]] std::size_t Size() const noexcept { return size; }
    [[nodiscard]] int*        Data() noexcept { return data; }
    [[nodiscard]] const int*  Data() const noexcept { return data; }

private:
    int*        data = nullptr;
    std::size_t size = 0;
};

// NOLINTEND(cppcoreguidelines-owning-memory)

}  // namespace ch04
