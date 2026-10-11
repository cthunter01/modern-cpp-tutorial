#pragma once

#include <algorithm>
#include <span>

#include "person.h"

namespace ch03
{

/// The oldest of @p people, the first one when several are equally old; nullptr when there is
/// nobody. The result points into @p people.
[[nodiscard]] inline const Person* oldest(std::span<const Person> people) noexcept
{
    // max_element compares ages (the projection) and returns an iterator to a Person (the element).
    const auto found = std::ranges::max_element(people, {}, &Person::age);
    return found == people.end() ? nullptr : &*found;
}

}  // namespace ch03
