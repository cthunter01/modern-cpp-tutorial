#pragma once

#include <concepts>
#include <string>
#include <utility>

namespace ch05
{

/// A piece of text with a meaning. Constructible from anything a std::string is constructible from.
class Label
{
public:
    //! [constrained]
    // Without the requires clause this constructor accepts *everything*, including another Label,
    // and for a non-const Label it is a better match than the copy constructor. The clause says
    // what the constructor is for, and the compiler then leaves it out of every other case.
    template <typename U>
        requires std::constructible_from<std::string, U>
    explicit Label(U&& value) : text(std::forward<U>(value))
    {
    }
    //! [constrained]

    [[nodiscard]] const std::string& Text() const noexcept { return text; }

private:
    std::string text;
};

}  // namespace ch05
