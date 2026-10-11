#pragma once

#include <utility>

#include "handles.h"

namespace ch04
{

/// Owns one handle of the os API and gives it back when it is destroyed. Exactly one Handle owns a
/// given handle at any time: it can be moved, not copied.
class Handle
{
public:
    /// Acquires a handle.
    [[nodiscard]] static Handle Open() { return Handle(os::acquire()); }

    ~Handle() { Close(); }

    Handle(const Handle&)            = delete;
    Handle& operator=(const Handle&) = delete;

    //! [move]
    Handle(Handle&& other) noexcept : id(std::exchange(other.id, kNone)) { }

    Handle& operator=(Handle&& other) noexcept
    {
        if (this != &other)
        {
            Close();
            id = std::exchange(other.id, kNone);
        }
        return *this;
    }
    //! [move]

    /// The number the os API knows this handle by.
    [[nodiscard]] int Id() const noexcept { return id; }

private:
    static constexpr int kNone = -1;  // what a moved-from Handle holds; the API never returns it

    explicit Handle(int acquired) noexcept : id(acquired) { }

    void Close() noexcept
    {
        if (id != kNone)
        {
            os::release(id);
            id = kNone;
        }
    }

    int id;
};

}  // namespace ch04
