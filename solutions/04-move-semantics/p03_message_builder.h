#pragma once

#include <string>
#include <string_view>
#include <utility>

namespace ch04
{

//! [builder]
/// Builds a message piece by piece.
///
/// Finishing is Take(). It hands the text over without copying it and leaves the builder EMPTY,
/// which is a defined state: the builder can be used for the next message straight away.
class MessageBuilder
{
public:
    MessageBuilder& Add(std::string_view piece)
    {
        text += piece;
        return *this;
    }

    [[nodiscard]] bool Empty() const noexcept { return text.empty(); }

    /// The message so far. The builder is empty afterwards.
    [[nodiscard]] std::string Take() noexcept { return std::exchange(text, {}); }

private:
    std::string text;
};
//! [builder]

}  // namespace ch04
