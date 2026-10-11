#pragma once

#include <span>

#include "person.h"
#include "tut/todo.h"

namespace ch03
{

/// The oldest of @p people, the first one when several are equally old; nullptr when there is
/// nobody. The result points into @p people.
[[nodiscard]] inline const Person* oldest(std::span<const Person> people) noexcept
{
    TUT_TODO(people);
}

}  // namespace ch03
