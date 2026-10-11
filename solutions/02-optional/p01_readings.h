#pragma once

#include <cstddef>
#include <optional>

namespace ch02
{

/// The readings of one sensor over a period, some of which may be missing.
class Readings
{
public:
    /// A reading the sensor delivered.
    void Add(double value) noexcept
    {
        sum += value;
        ++valid;
    }

    /// A moment at which the sensor delivered nothing.
    void AddMissing() noexcept { ++missing; }

    [[nodiscard]] std::size_t Missing() const noexcept { return missing; }

    //! [average]
    /// The mean of the readings that were delivered, or nothing when none was: an average of no
    /// values does not exist, and 0.0 is a temperature.
    [[nodiscard]] std::optional<double> Average() const noexcept
    {
        if (valid == 0)
        {
            return std::nullopt;
        }
        return sum / static_cast<double>(valid);
    }
    //! [average]

private:
    double      sum     = 0.0;
    std::size_t valid   = 0;
    std::size_t missing = 0;
};

}  // namespace ch02
