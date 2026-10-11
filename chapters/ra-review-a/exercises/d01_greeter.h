#pragma once

#include <string>
#include <string_view>

namespace review_a
{

/// Remembers a name and greets it.
class Greeter
{
public:
    explicit Greeter(std::string_view who) : name(who) { }

    [[nodiscard]] std::string Greet() const { return "Hello, " + std::string(name) + "!"; }

private:
    std::string_view name;
};

}  // namespace review_a
