#pragma once

#include <string>
#include <string_view>

#include "tut/todo.h"

namespace ch00
{

struct Request
{
    std::string method = "GET";
    std::string path;
    int         timeoutSeconds = 30;
    bool        keepAlive      = true;
};

/// A request for @p path. Paths below /upload are sent with POST, get two minutes and are not kept
/// alive; everything else keeps the defaults of Request.
[[nodiscard]] inline Request make_request(std::string_view path)
{
    TUT_TODO(path);
}

}  // namespace ch00
