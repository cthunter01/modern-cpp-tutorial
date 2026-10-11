#pragma once

#include <cstdint>
#include <string_view>

// Given, for design problem 4: the logger's levels and their names, as in step 1 of the rebuild
// guide. Nothing to change here.

namespace ch03
{

/// Severity, in increasing verbosity.
enum class Level : std::uint8_t
{
    Off,
    Error,
    Warn,
    Info,
    Debug,
    Perf,
};

/// "OFF", "ERROR", "WARN", "INFO", "DEBUG" or "PERF".
[[nodiscard]] constexpr std::string_view level_name(Level level) noexcept
{
    switch (level)
    {
        case Level::Off:
            return "OFF";
        case Level::Error:
            return "ERROR";
        case Level::Warn:
            return "WARN";
        case Level::Info:
            return "INFO";
        case Level::Debug:
            return "DEBUG";
        case Level::Perf:
            return "PERF";
    }
    return "?";
}

}  // namespace ch03
