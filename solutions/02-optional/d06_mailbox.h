#pragma once

#include <optional>
#include <string>
#include <utility>

namespace ch02
{

/// Holds at most one message until somebody collects it.
class Mailbox
{
public:
    /// Stores @p message, replacing one that was not collected.
    void Put(std::string message) { slot = std::move(message); }

    /// The stored message, if there is one. The mailbox is empty afterwards.
    [[nodiscard]] std::optional<std::string> Take() noexcept
    {
        // exchange, not std::move(slot): a moved-from optional is still engaged. It would hold an
        // empty string, and the next Take() would return that as if it were a message.
        return std::exchange(slot, std::nullopt);
    }

private:
    std::optional<std::string> slot;
};

}  // namespace ch02
