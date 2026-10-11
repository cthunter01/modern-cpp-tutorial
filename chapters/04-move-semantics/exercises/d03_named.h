#pragma once

#include <string>
#include <utility>

#include "tut/tracked.h"

namespace ch04
{

// This class works. It is also slower than it looks: find out why moving a Named copies its
// payload, and repair it. The destructor has a job and has to stay.

/// Something with a name and a payload, which counts how many of its kind have been destroyed.
class Named
{
public:
    Named(std::string label, tut::Tracked content, int& destroyedCounter)
      : name(std::move(label)), payload(std::move(content)), destroyed(&destroyedCounter)
    {
    }

    ~Named() { ++*destroyed; }

    [[nodiscard]] const std::string&  Name() const noexcept { return name; }
    [[nodiscard]] const tut::Tracked& Payload() const noexcept { return payload; }

private:
    std::string  name;
    tut::Tracked payload;
    int*         destroyed;
};

}  // namespace ch04
