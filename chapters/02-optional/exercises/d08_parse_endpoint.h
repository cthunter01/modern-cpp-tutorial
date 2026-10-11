#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

// Your own parse_uint() from drill 2.
#include "d02_parse_uint.h"
#include "tut/todo.h"

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
    TUT_TODO(text);
}

}  // namespace ch02
