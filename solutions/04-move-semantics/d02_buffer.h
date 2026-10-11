#pragma once

#include <algorithm>
#include <cstddef>
#include <utility>

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

    //! [copy]
    // Copying: new storage, and every element copied.
    Buffer(const Buffer& other)
      : data(other.size == 0 ? nullptr : new int[other.size]), size(other.size)
    {
        std::copy_n(other.data, size, data);
    }

    // Copy and swap: build the copy first, then exchange contents with it. If the allocation
    // throws, *this is untouched; and assigning an object to itself needs no special case.
    Buffer& operator=(const Buffer& other)
    {
        Buffer copy(other);
        Swap(copy);
        return *this;
    }
    //! [copy]

    //! [move]
    // Moving: take the pointer, and leave the source in a state its destructor can live with.
    Buffer(Buffer&& other) noexcept
      : data(std::exchange(other.data, nullptr)), size(std::exchange(other.size, 0))
    {
    }

    Buffer& operator=(Buffer&& other) noexcept
    {
        if (this != &other)
        {
            delete[] data;  // what *this owned is no longer needed
            data = std::exchange(other.data, nullptr);
            size = std::exchange(other.size, 0);
        }
        return *this;
    }
    //! [move]

    [[nodiscard]] std::size_t Size() const noexcept { return size; }
    [[nodiscard]] int*        Data() noexcept { return data; }
    [[nodiscard]] const int*  Data() const noexcept { return data; }

private:
    void Swap(Buffer& other) noexcept
    {
        std::swap(data, other.data);
        std::swap(size, other.size);
    }

    int*        data = nullptr;
    std::size_t size = 0;
};

// NOLINTEND(cppcoreguidelines-owning-memory)

}  // namespace ch04
