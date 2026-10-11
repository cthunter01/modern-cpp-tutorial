#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "d02_parse_uint.h"

namespace ch02
{

/// Settings as they come out of a file: keys and values, both text.
struct Config
{
    std::vector<std::pair<std::string, std::string>> entries;
};

/// The value stored under @p key, or nothing. The result is a part of @p config.
[[nodiscard]] inline std::optional<std::string_view> lookup(const Config&    config,
                                                            std::string_view key)
{
    for (const auto& [name, value] : config.entries)
    {
        if (name == key)
        {
            return value;
        }
    }
    return std::nullopt;
}

/// The port to listen on: the entry "port" when there is one and it is a number, otherwise what
/// @p fallback returns. Calling @p fallback is expensive (think of a request to another machine):
/// it must be called only when its answer is needed.
[[nodiscard]] inline unsigned port_or(const Config&                    config,
                                      const std::function<unsigned()>& fallback)
{
    // Each step may have nothing to give; and_then passes a value on and lets "nothing" fall
    // through.
    const std::optional<unsigned> port = lookup(config, "port").and_then(parse_uint);

    // Not port.value_or(fallback()): the argument of value_or is computed before value_or looks at
    // the optional, so the fallback would be called every time.
    return port ? *port : fallback();
}

}  // namespace ch02
