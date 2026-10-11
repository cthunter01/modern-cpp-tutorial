#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "tut/tracked.h"

namespace ch04
{

/// A vector of @p count items with the values 0, 1, 2, ...
[[nodiscard]] inline std::vector<tut::Tracked> make_items(std::size_t count)
{
    std::vector<tut::Tracked> items;
    items.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        items.emplace_back(static_cast<int>(i));
    }
    // Not std::move(items). A local returned by name is moved, or not even that: the compiler may
    // build it directly where the caller wants it. std::move here can only prevent the second.
    return items;
}

/// Collects pieces of text into one string.
class Builder
{
public:
    void Append(std::string_view piece) { text += piece; }

    /// The text so far.
    [[nodiscard]] std::string_view View() const noexcept { return text; }

    //! [take]
    /// The finished text. `&&` after the parameter list: this function can only be called on a
    /// Builder that is an rvalue, std::move(builder).Take(), which says at the call that the
    /// builder is being given up.
    [[nodiscard]] std::string Take() && noexcept { return std::move(text); }
    //! [take]

private:
    std::string text;
};

}  // namespace ch04
