#pragma once

#include <sstream>
#include <string>
#include <string_view>

namespace ch05
{

/// With nothing to join, the result is empty.
[[nodiscard]] inline std::string join_with(std::string_view /*separator*/)
{
    return {};
}

/// The arguments written one after the other, as operator<< writes them, with @p separator between
/// them.
template <typename First, typename... Rest>
[[nodiscard]] std::string join_with([[maybe_unused]] std::string_view separator, const First& first,
                                    const Rest&... rest)
{
    std::ostringstream out;
    out << first;
    // A fold over the comma operator: "do this for each element of the pack, in order". Each of the
    // remaining items is written with the separator in front of it. With one item there is no rest,
    // the fold expands to nothing, and `separator` is not used: hence the attribute.
    ((out << separator << rest), ...);
    return out.str();
}

}  // namespace ch05
