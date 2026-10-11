#pragma once

#include <charconv>
#include <cstdint>
#include <expected>
#include <map>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace review_a
{

//! [result]
/// Why a getter has no value to give.
enum class Lookup : std::uint8_t
{
    Missing,    ///< the key is not in the settings: use your default
    Malformed,  ///< the key is there and its value cannot be read as the type asked for: tell the
                ///< user
};

template <typename T>
using Setting = std::expected<T, Lookup>;

/// Whether a getter came back empty because the key was not there (as opposed to being unreadable).
template <typename T>
[[nodiscard]] constexpr bool is_missing(const Setting<T>& setting) noexcept
{
    return !setting && setting.error() == Lookup::Missing;
}
//! [result]

/// Settings read from text of the form `key = value`, one per line. Blank lines and lines that
/// start with '#' are ignored, and so is a line without '='. A key given twice keeps its last
/// value.
class Settings
{
public:
    //! [parse]
    /// Reads @p text. Nothing of @p text is referred to afterwards: keys and values are copied.
    explicit Settings(std::string_view text)
    {
        while (!text.empty())
        {
            const std::size_t      newline = text.find('\n');
            const std::string_view line    = Trimmed(text.substr(0, newline));
            text.remove_prefix(newline == std::string_view::npos ? text.size() : newline + 1);

            const std::size_t equals = line.find('=');
            if (line.empty() || line.front() == '#' || equals == std::string_view::npos)
            {
                continue;
            }
            const std::string_view key = Trimmed(line.substr(0, equals));
            if (!key.empty())
            {
                values.insert_or_assign(std::string(key),
                                        std::string(Trimmed(line.substr(equals + 1))));
            }
        }
    }
    //! [parse]

    [[nodiscard]] Setting<std::string> Text(std::string_view key) const
    {
        const auto found = values.find(key);
        if (found == values.end())
        {
            return std::unexpected(Lookup::Missing);
        }
        return found->second;
    }

    //! [number]
    [[nodiscard]] Setting<int> Number(std::string_view key) const
    {
        const auto found = values.find(key);
        if (found == values.end())
        {
            return std::unexpected(Lookup::Missing);
        }
        const std::string_view text   = found->second;
        int                    value  = 0;
        const char* const      first  = text.data();
        const char* const      last   = first + text.size();  // NOLINT(*-pointer-arithmetic)
        const auto [stoppedAt, error] = std::from_chars(first, last, value);
        if (error != std::errc{} || stoppedAt != last)
        {
            return std::unexpected(Lookup::Malformed);
        }
        return value;
    }
    //! [number]

    [[nodiscard]] Setting<bool> Flag(std::string_view key) const
    {
        const auto found = values.find(key);
        if (found == values.end())
        {
            return std::unexpected(Lookup::Missing);
        }
        if (found->second == "true" || found->second == "on")
        {
            return true;
        }
        if (found->second == "false" || found->second == "off")
        {
            return false;
        }
        return std::unexpected(Lookup::Malformed);
    }

private:
    [[nodiscard]] static constexpr std::string_view Trimmed(std::string_view text) noexcept
    {
        constexpr std::string_view kBlanks = " \t\r";
        const std::size_t          first   = text.find_first_not_of(kBlanks);
        if (first == std::string_view::npos)
        {
            return {};
        }
        return text.substr(first, text.find_last_not_of(kBlanks) - first + 1);
    }

    // std::less<> lets find() take a string_view without building a std::string for the search.
    std::map<std::string, std::string, std::less<>> values;
};

}  // namespace review_a
