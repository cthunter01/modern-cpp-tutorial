#pragma once

#include <string>
#include <string_view>

namespace ch00
{

struct Request
{
    std::string method = "GET";
    std::string path;
    int         timeoutSeconds = 30;
    bool        keepAlive      = true;
};

/// A request for @p path. Paths below /upload are sent with POST and get two minutes; everything
/// else keeps the defaults of Request.
[[nodiscard]] inline Request make_request(std::string_view path)
{
    if (path.starts_with("/upload"))
    {
        return {
            .method         = "POST",
            .path           = std::string(path),
            .timeoutSeconds = 120,
            .keepAlive      = false,
        };
    }
    return {.path = std::string(path)};
}

}  // namespace ch00
