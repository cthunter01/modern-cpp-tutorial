#pragma once

#include <string_view>

#include "tut/tracked.h"

// Given, for drill 1. Nothing to change here.

namespace ch04
{

/// Two overloads. Which one a call selects depends on the value category of the argument and on
/// nothing else.
[[nodiscard]] constexpr std::string_view which(const tut::Tracked& /*unused*/) noexcept
{
    return "lvalue";  // chosen for anything that may still be needed afterwards
}

// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved): it only reports the category
[[nodiscard]] constexpr std::string_view which(tut::Tracked&& /*unused*/) noexcept
{
    return "rvalue";  // chosen for anything that is about to go away
}

}  // namespace ch04
