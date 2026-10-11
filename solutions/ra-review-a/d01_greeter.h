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

    [[nodiscard]] std::string Greet() const { return "Hello, " + name + "!"; }

private:
    // The bug was the type of this member. A string_view member made the Greeter a view of whatever
    // text the caller happened to pass, valid only while that text stayed where and what it was. An
    // object that is kept has to own what it keeps.
    std::string name;
};

}  // namespace review_a
