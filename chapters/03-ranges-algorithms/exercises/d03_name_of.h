#pragma once

#include <optional>
#include <span>
#include <string_view>

#include "person.h"
#include "tut/todo.h"

namespace ch03
{

/// The name of the person with @p id, or nothing. The name is a part of @p people.
[[nodiscard]] inline std::optional<std::string_view> name_of(std::span<const Person> people, int id)
{
    TUT_TODO(people, id);
}

}  // namespace ch03
