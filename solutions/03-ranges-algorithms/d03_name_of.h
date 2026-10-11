#pragma once

#include <algorithm>
#include <optional>
#include <span>
#include <string_view>

#include "person.h"

namespace ch03
{

/// The name of the person with @p id, or nothing. The name is a part of @p people.
[[nodiscard]] inline std::optional<std::string_view> name_of(std::span<const Person> people, int id)
{
    // "Find the element whose id equals `id`": a value to look for and a projection, no lambda.
    const auto found = std::ranges::find(people, id, &Person::id);
    if (found == people.end())
    {
        return std::nullopt;
    }
    return found->name;
}

}  // namespace ch03
