#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <string_view>

#include "d02_parse_uint.h"

namespace ch02
{

//! [types]
enum class PortError : std::uint8_t
{
    NotAPortSetting,  ///< the line is not `port = ...`
    NotANumber,       ///< the value is not an unsigned number
    OutOfRange,       ///< a number, but not between 1 and 65535
};

using PortResult = std::expected<unsigned, PortError>;
//! [types]

namespace detail
{

constexpr std::string_view trimmed(std::string_view text) noexcept
{
    constexpr std::string_view kBlanks = " \t";
    const std::size_t          first   = text.find_first_not_of(kBlanks);
    if (first == std::string_view::npos)
    {
        return {};
    }
    return text.substr(first, text.find_last_not_of(kBlanks) - first + 1);
}

}  // namespace detail

/// Reads a line of the form `port = N`. Blanks around the key and the number are ignored.
[[nodiscard]] inline PortResult load_port(std::string_view line) noexcept
{
    constexpr unsigned kLargestPort = 65'535;

    const std::size_t equals = line.find('=');
    if (equals == std::string_view::npos || detail::trimmed(line.substr(0, equals)) != "port")
    {
        return std::unexpected(PortError::NotAPortSetting);
    }
    const std::optional<unsigned> number = parse_uint(detail::trimmed(line.substr(equals + 1)));
    if (!number)
    {
        return std::unexpected(PortError::NotANumber);
    }
    if (*number == 0 || *number > kLargestPort)
    {
        return std::unexpected(PortError::OutOfRange);
    }
    return *number;
}

/// What to tell the user about a line that load_port() did not accept. Empty for a result that is a
/// port. The messages are literals, so a view of one is all that has to be returned.
[[nodiscard]] constexpr std::string_view describe_failure(const PortResult& result) noexcept
{
    if (result)
    {
        return {};
    }
    switch (result.error())
    {
        case PortError::NotAPortSetting:
            return "this line does not set the port";
        case PortError::NotANumber:
            return "the port must be a number";
        case PortError::OutOfRange:
            return "the port must be between 1 and 65535";
    }
    return {};
}

}  // namespace ch02
