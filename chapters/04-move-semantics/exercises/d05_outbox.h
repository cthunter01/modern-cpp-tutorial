#pragma once

#include <string>
#include <vector>

#include "tut/todo.h"

namespace ch04
{

/// Messages waiting to be sent.
class Outbox
{
public:
    /// Takes @p message over. The parameter is an rvalue reference, so a caller has to give the
    /// string up, visibly: Send(std::move(text)) or Send(make_text()).
    void Send(std::string&& message) { TUT_TODO(message); }

    /// Hands over everything that is waiting. The outbox is empty afterwards.
    [[nodiscard]] std::vector<std::string> Drain() noexcept { TUT_TODO(); }

private:
    std::vector<std::string> pending;
};

}  // namespace ch04
