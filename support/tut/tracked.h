#pragma once

#include <utility>

namespace tut
{

/// What has happened to Tracked objects since the last Tracked::ResetCounts().
struct TrackedCounts
{
    int copies = 0;  ///< copy constructions and copy assignments
    int moves  = 0;  ///< move constructions and move assignments
};

/// A value that counts how often it is copied and how often it is moved. Tests use it to see what
/// the types alone do not show: whether a function copied its argument or moved it.
///
///     tut::Tracked::ResetCounts();
///     registry.Add(std::move(item));
///     EXPECT_EQ(tut::Tracked::Counts().copies, 0);
///
/// A Tracked that has been moved from says so (IsMovedFrom()) and has the value -1.
class Tracked
{
public:
    Tracked() noexcept = default;
    explicit Tracked(int number) noexcept : value(number) { }

    Tracked(const Tracked& other) noexcept : value(other.value) { ++counts.copies; }
    Tracked(Tracked&& other) noexcept : value(std::exchange(other.value, kMovedFrom))
    {
        ++counts.moves;
    }

    Tracked& operator=(const Tracked& other) noexcept
    {
        value = other.value;
        ++counts.copies;
        return *this;
    }
    Tracked& operator=(Tracked&& other) noexcept
    {
        if (this != &other)
        {
            value = std::exchange(other.value, kMovedFrom);
        }
        ++counts.moves;
        return *this;
    }

    ~Tracked() = default;

    [[nodiscard]] int  Value() const noexcept { return value; }
    [[nodiscard]] bool IsMovedFrom() const noexcept { return value == kMovedFrom; }

    [[nodiscard]] static TrackedCounts Counts() noexcept { return counts; }
    static void                        ResetCounts() noexcept { counts = {}; }

private:
    static constexpr int kMovedFrom = -1;

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables): counting is the point
    inline static TrackedCounts counts;

    int value = 0;
};

}  // namespace tut
