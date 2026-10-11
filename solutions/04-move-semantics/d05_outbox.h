#pragma once

#include <string>
#include <utility>
#include <vector>

namespace ch04
{

/// Messages waiting to be sent.
class Outbox
{
public:
    //! [send]
    /// Takes @p message over. The parameter is an rvalue reference, so a caller has to give the
    /// string up, visibly: Send(std::move(text)) or Send(make_text()).
    void Send(std::string&& message)
    {
        // `message` has a name, so it is an lvalue here, whatever its type. Without std::move this
        // line would copy.
        pending.push_back(std::move(message));
    }
    //! [send]

    //! [drain]
    /// Hands over everything that is waiting. The outbox is empty afterwards.
    [[nodiscard]] std::vector<std::string> Drain() noexcept
    {
        // exchange: the old content goes to the caller and `pending` is left in a state that is
        // known, not merely "valid but unspecified".
        return std::exchange(pending, {});
    }
    //! [drain]

private:
    std::vector<std::string> pending;
};

}  // namespace ch04
