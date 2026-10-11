#pragma once

#include <cstdint>
#include <string_view>

namespace tut
{

/// Where the characters of a text are stored, as a number. Two strings with the same storage_of()
/// are one block of characters: nothing was copied between them. Tests use it as evidence of a
/// move.
[[nodiscard]] inline std::uintptr_t storage_of(std::string_view text) noexcept
{
    // An address is wanted, as a number.   NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<std::uintptr_t>(text.data());
}

}  // namespace tut
