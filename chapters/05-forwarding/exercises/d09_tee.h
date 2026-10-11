#pragma once

#include <string_view>

#include "tut/todo.h"

namespace ch05
{

/// Writes every line to each of several sinks, in the order they were given. A sink is anything
/// with a Write(std::string_view). The sinks are not owned: they must outlive the Tee.
template <typename... Sinks>
class Tee
{
public:
    explicit Tee(Sinks&... targets) { TUT_TODO(targets...); }

    void Write(std::string_view line) { TUT_TODO(line); }

private:
    // Your data member: something that holds one reference per sink.
};

}  // namespace ch05
