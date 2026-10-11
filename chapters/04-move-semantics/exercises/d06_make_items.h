#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "tut/todo.h"
#include "tut/tracked.h"

namespace ch04
{

/// A vector of @p count items with the values 0, 1, 2, ...
[[nodiscard]] inline std::vector<tut::Tracked> make_items(std::size_t count)
{
    TUT_TODO(count);
}

/// Collects pieces of text into one string.
class Builder
{
public:
    void Append(std::string_view piece) { text += piece; }

    /// The text so far.
    [[nodiscard]] std::string_view View() const noexcept { return text; }

    /// The finished text. `&&` after the parameter list: this function can only be called on a
    /// Builder that is an rvalue, std::move(builder).Take(), which says at the call that the
    /// builder is being given up.
    [[nodiscard]] std::string Take() && noexcept { TUT_TODO(); }

private:
    std::string text;
};

}  // namespace ch04
