#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace ch03
{

/// Removes the lines that are empty or consist of spaces and tabs only. The others keep their
/// order.
inline void drop_blank(std::vector<std::string>& lines)
{
    const auto isBlank = [](std::string_view line) {
        return line.find_first_not_of(" \t") == std::string_view::npos;
    };
    // std::erase_if (C++20) is remove_if and erase in one call. The two-step form is:
    //     const auto leftover = std::ranges::remove_if(lines, isBlank);
    //     lines.erase(leftover.begin(), leftover.end());
    std::erase_if(lines, isBlank);
}

}  // namespace ch03
