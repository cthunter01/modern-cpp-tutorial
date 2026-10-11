#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>

#include "d02_parse_uint.h"

namespace ch02
{

struct Endpoint
{
    std::string host;
    unsigned    port = 0;
};

enum class ParseError : std::uint8_t
{
    MissingColon,  ///< no ':' between host and port
    EmptyHost,     ///< nothing in front of the ':'
    BadPort,       ///< the part behind the ':' is not a number between 1 and 65535
};

/// "host:port" taken apart, or the reason why it could not be. The ':' that counts is the last one.
[[nodiscard]] inline std::expected<Endpoint, ParseError> parse_endpoint(std::string_view text)
{
    constexpr unsigned kLargestPort = 65'535;

    const std::size_t colon = text.rfind(':');
    if (colon == std::string_view::npos)
    {
        return std::unexpected(ParseError::MissingColon);
    }
    const std::string_view host = text.substr(0, colon);
    if (host.empty())
    {
        return std::unexpected(ParseError::EmptyHost);
    }
    const std::optional<unsigned> port = parse_uint(text.substr(colon + 1));
    if (!port || *port == 0 || *port > kLargestPort)
    {
        return std::unexpected(ParseError::BadPort);
    }
    return Endpoint{.host = std::string(host), .port = *port};
}

}  // namespace ch02
