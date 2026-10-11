#pragma once

#include <string_view>

namespace ch01
{

//! [contract]
/// The body of section @p name in an INI-like @p text: everything after the line "[name]" up to the
/// next line that starts with '[', or to the end. Empty when there is no such section.
///
/// LIFETIME: the result is a part of @p text. It is valid only while the characters @p text refers
/// to exist and do not move: do not let it outlive the buffer the file was read into.
[[nodiscard]] constexpr std::string_view section_body(std::string_view text,
                                                      std::string_view name) noexcept
//! [contract]
{
    while (!text.empty())
    {
        // One line, without its newline; `text` moves on to the start of the next line.
        const std::size_t      newline = text.find('\n');
        const std::string_view line    = text.substr(0, newline);
        text.remove_prefix(newline == std::string_view::npos ? text.size() : newline + 1);

        if (line.size() == name.size() + 2 && line.front() == '[' && line.back() == ']' &&
            line.substr(1, name.size()) == name)
        {
            // The body runs to the next header: a '[' at the very start, or directly after a
            // newline.
            if (text.starts_with('['))
            {
                return text.substr(0, 0);
            }
            const std::size_t next = text.find("\n[");
            return text.substr(0, next == std::string_view::npos ? next : next + 1);
        }
    }
    return {};
}

}  // namespace ch01
