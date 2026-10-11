#pragma once

#include <set>

// Given, for design problem 2: a stand-in for an operating-system API that hands out numbered
// resources. Nothing to change here.

namespace ch04::os
{

namespace detail
{

struct Table
{
    std::set<int> open;
    int           next        = 100;
    int           badReleases = 0;
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables): it plays the operating system
inline Table g_table;

}  // namespace detail

/// Opens a resource and returns its handle. Every handle must be given back with release(), once.
[[nodiscard]] inline int acquire()
{
    const int handle = detail::g_table.next++;
    detail::g_table.open.insert(handle);
    return handle;
}

/// Gives a handle back. Releasing a handle that is not open (never acquired, or released before) is
/// the kind of bug that corrupts a real system; here it is counted.
inline void release(int handle)
{
    if (detail::g_table.open.erase(handle) == 0)
    {
        ++detail::g_table.badReleases;
    }
}

/// For the tests: how many handles are open, how many releases were wrong, and a fresh start.
[[nodiscard]] inline int open_handles()
{
    return static_cast<int>(detail::g_table.open.size());
}
[[nodiscard]] inline int bad_releases()
{
    return detail::g_table.badReleases;
}
inline void reset()
{
    detail::g_table = {};
}

}  // namespace ch04::os
