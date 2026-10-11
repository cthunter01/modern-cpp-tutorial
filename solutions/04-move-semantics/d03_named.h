#pragma once

#include <string>
#include <utility>

#include "tut/tracked.h"

namespace ch04
{

/// Something with a name and a payload, which counts how many of its kind have been destroyed.
class Named
{
public:
    Named(std::string label, tut::Tracked content, int& destroyedCounter)
      : name(std::move(label)), payload(std::move(content)), destroyed(&destroyedCounter)
    {
    }

    ~Named() { ++*destroyed; }

    //! [five]
    // The destructor above is user-declared, and that alone stops the compiler from generating the
    // move operations: "moving" a Named then silently copies it. Asking for all four by name
    // restores them. This is the rule of five in its mildest form: nothing to write, four lines to
    // declare.
    Named(const Named&)                = default;
    Named& operator=(const Named&)     = default;
    Named(Named&&) noexcept            = default;
    Named& operator=(Named&&) noexcept = default;
    //! [five]

    [[nodiscard]] const std::string&  Name() const noexcept { return name; }
    [[nodiscard]] const tut::Tracked& Payload() const noexcept { return payload; }

private:
    std::string  name;
    tut::Tracked payload;
    int*         destroyed;
};

}  // namespace ch04
