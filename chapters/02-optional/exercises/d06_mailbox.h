#pragma once

#include <optional>
#include <string>

#include "tut/todo.h"

namespace ch02
{

/// Holds at most one message until somebody collects it.
class Mailbox
{
public:
    /// Stores @p message, replacing one that was not collected.
    void Put(std::string message) { TUT_TODO(message); }

    /// The stored message, if there is one. The mailbox is empty afterwards.
    [[nodiscard]] std::optional<std::string> Take() noexcept { TUT_TODO(); }

private:
    // What does a mailbox have to remember? That is the first thing to write.
};

}  // namespace ch02
