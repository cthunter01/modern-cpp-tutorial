#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ch05
{

namespace detail
{

/// Records "<label>: begin" when it is created and "<label>: end" when it is destroyed.
class Stamp
{
public:
    Stamp(std::vector<std::string>& target, std::string_view name) : timings(&target), label(name)
    {
        timings->push_back(label + ": begin");
    }
    ~Stamp() { timings->push_back(label + ": end"); }

    Stamp(const Stamp&)            = delete;
    Stamp& operator=(const Stamp&) = delete;
    Stamp(Stamp&&)                 = delete;
    Stamp& operator=(Stamp&&)      = delete;

private:
    std::vector<std::string>* timings;
    std::string               label;
};

}  // namespace detail

//! [timed]
/// Calls `function(args...)` and returns what it returns, a reference as a reference and nothing as
/// nothing. "<label>: begin" is appended to @p timings before the call and "<label>: end" after it,
/// also when the call throws.
template <typename F, typename... Args>
decltype(auto) timed(std::vector<std::string>& timings, std::string_view label, F&& function,
                     Args&&... args)
{
    const detail::Stamp stamp(timings, label);  // its destructor writes the "end" entry
    return std::invoke(std::forward<F>(function), std::forward<Args>(args)...);
}
//! [timed]

}  // namespace ch05
