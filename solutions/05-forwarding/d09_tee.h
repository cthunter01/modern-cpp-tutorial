#pragma once

#include <string_view>
#include <tuple>

namespace ch05
{

/// Writes every line to each of several sinks, in the order they were given. A sink is anything
/// with a Write(std::string_view). The sinks are not owned: they must outlive the Tee.
template <typename... Sinks>
class Tee
{
public:
    explicit Tee(Sinks&... targets) : sinks(targets...) { }

    void Write(std::string_view line)
    {
        // std::apply calls the lambda with the tuple's elements as separate arguments, which gives
        // a pack to fold over.
        std::apply([&](Sinks&... each) { (each.Write(line), ...); }, sinks);
    }

private:
    std::tuple<Sinks&...> sinks;
};

}  // namespace ch05
