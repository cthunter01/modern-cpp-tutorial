#pragma once

#include <string>
#include <utility>

namespace ch05
{

// This class works for what it was written for, and breaks something it was not meant to touch: a
// Label that is not const can no longer be copied. Find out why, and say in the constructor's
// declaration what the constructor is for.

/// A piece of text with a meaning. Constructible from anything a std::string is constructible from.
class Label
{
public:
    template <typename U>
    explicit Label(U&& value) : text(std::forward<U>(value))
    {
    }

    [[nodiscard]] const std::string& Text() const noexcept { return text; }

private:
    std::string text;
};

}  // namespace ch05
