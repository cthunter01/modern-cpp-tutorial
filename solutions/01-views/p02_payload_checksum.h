#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace ch01
{

inline constexpr std::size_t kHeaderBytes  = 4;
inline constexpr std::size_t kTrailerBytes = 2;

/// The sum, modulo 256, of the payload bytes of @p frame: everything between its 4-byte header and
/// its 2-byte trailer. 0 for a frame too short to have a payload.
[[nodiscard]] constexpr std::uint8_t payload_checksum(std::span<const std::uint8_t> frame) noexcept
{
    if (frame.size() <= kHeaderBytes + kTrailerBytes)
    {
        return 0;
    }
    const std::span<const std::uint8_t> payload =
        frame.subspan(kHeaderBytes, frame.size() - kHeaderBytes - kTrailerBytes);

    unsigned sum = 0;
    for (const std::uint8_t byte : payload)
    {
        sum += byte;
    }
    return static_cast<std::uint8_t>(sum % 256U);
}

}  // namespace ch01
