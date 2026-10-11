#pragma once

#include <optional>
#include <string>

#include "tut/todo.h"

namespace ch02
{

struct Settings
{
    std::string name;
    int         width   = 0;
    int         height  = 0;
    double      opacity = 1.0;
};

/// What is wrong with @p settings, or nothing when they are valid. Of several problems the first is
/// reported, in the order name, width, height, opacity. The name must not be empty, width and
/// height must be between 1 and 10000, the opacity between 0 and 1. Each message names the member.
[[nodiscard]] inline std::optional<std::string> validate(const Settings& settings)
{
    TUT_TODO(settings);
}

}  // namespace ch02
