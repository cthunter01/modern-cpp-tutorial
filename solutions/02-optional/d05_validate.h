#pragma once

#include <optional>
#include <string>

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
    constexpr int kLargest = 10'000;

    if (settings.name.empty())
    {
        return "name must not be empty";
    }
    if (settings.width < 1 || settings.width > kLargest)
    {
        return "width must be between 1 and 10000, not " + std::to_string(settings.width);
    }
    if (settings.height < 1 || settings.height > kLargest)
    {
        return "height must be between 1 and 10000, not " + std::to_string(settings.height);
    }
    if (settings.opacity < 0.0 || settings.opacity > 1.0)
    {
        return "opacity must be between 0 and 1, not " + std::to_string(settings.opacity);
    }
    return std::nullopt;  // here "nothing" is the good news
}

}  // namespace ch02
