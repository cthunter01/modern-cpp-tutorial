#pragma once

#include <algorithm>
#include <vector>

#include "person.h"

namespace ch03
{

/// Sorts @p people by age, youngest first. People of the same age keep the order they had.
inline void sort_by_age(std::vector<Person>& people)
{
    // {} is the comparator: "the default one", std::ranges::less. &Person::age is the projection:
    // what to look at in each element. stable_sort, because equal ages must keep their order.
    std::ranges::stable_sort(people, {}, &Person::age);
}

}  // namespace ch03
