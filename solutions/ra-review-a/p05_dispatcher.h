#pragma once

#include <algorithm>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace review_a
{

/// Finds the handler for the first word of a line and calls it with the remaining words.
class Dispatcher
{
public:
    //! [types]
    /// What a handler is given: the words after the verb. It may look at them and may not keep
    /// them: they are parts of the line that was passed to Run().
    using Words = std::span<const std::string_view>;

    /// A handler answers with a reply for the player, or with nothing.
    using Handler = std::function<std::optional<std::string>(Words)>;
    //! [types]

    void On(std::string verb, Handler handler)
    {
        verbs.push_back({.verb = std::move(verb), .handler = std::move(handler)});
    }

    //! [run]
    /// The reply of the handler whose verb the first word of @p line stands for: the verb that
    /// equals it, or else the only verb that begins with it, ignoring letter case. Nothing when the
    /// line is empty, when no verb or more than one fits, or when the handler has nothing to say.
    [[nodiscard]] std::optional<std::string> Run(std::string_view line) const
    {
        // The words are views into `line`: nothing is copied to take the line apart.
        std::vector<std::string_view> words;
        while (true)
        {
            line.remove_prefix(std::min(line.find_first_not_of(' '), line.size()));
            if (line.empty())
            {
                break;
            }
            words.push_back(line.substr(0, line.find(' ')));
            line.remove_prefix(words.back().size());
        }
        if (words.empty())
        {
            return std::nullopt;
        }

        const Entry* const entry = Find(words.front());
        if (entry == nullptr)
        {
            return std::nullopt;
        }
        return entry->handler(Words(words).subspan(1));
    }
    //! [run]

private:
    struct Entry
    {
        std::string verb;
        Handler     handler;
    };

    [[nodiscard]] static constexpr char Lower(char c) noexcept
    {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
    }

    //! [find]
    /// The entry @p typed stands for, or nullptr.
    [[nodiscard]] const Entry* Find(std::string_view typed) const
    {
        const auto isExactly = [typed](const Entry& entry) {
            return std::ranges::equal(entry.verb, typed, {}, Lower, Lower);
        };
        const auto startsWith = [typed](const Entry& entry) {
            return std::ranges::starts_with(entry.verb, typed, {}, Lower, Lower);
        };
        if (const auto exact = std::ranges::find_if(verbs, isExactly); exact != verbs.end())
        {
            return &*exact;
        }
        if (std::ranges::count_if(verbs, startsWith) != 1)
        {
            return nullptr;
        }
        return &*std::ranges::find_if(verbs, startsWith);
    }
    //! [find]

    std::vector<Entry> verbs;
};

}  // namespace review_a
