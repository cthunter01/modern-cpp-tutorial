#include "d05_scale.h"

#include <array>
#include <span>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

namespace
{

// scale() changes the elements, so it must not accept elements it is not allowed to change. This is
// true of the signature you were given; it is here so that you notice if you loosen it.
static_assert(std::is_invocable_v<decltype(&ch01::scale), std::span<double>, double>);
static_assert(!std::is_invocable_v<decltype(&ch01::scale), std::span<const double>, double>);

TEST(Scale, ChangesTheCallersElements)
{
    std::vector<double> values{1.0, 2.0, 3.0};

    ch01::scale(values, 2.0);

    EXPECT_EQ(values, (std::vector<double>{2.0, 4.0, 6.0}));
}

TEST(Scale, WorksOnAPart)
{
    std::array<double, 4> values{1.0, 1.0, 1.0, 1.0};

    ch01::scale(std::span(values).first(2), 5.0);

    EXPECT_EQ(values, (std::array<double, 4>{5.0, 5.0, 1.0, 1.0}));
}

TEST(Scale, AConstSpanStillLetsTheElementsChange)
{
    std::vector<double>     values{1.0, 2.0};
    const std::span<double> view(
        values);  // the span cannot be re-pointed; its elements are not const

    ch01::scale(view, 10.0);

    EXPECT_EQ(values, (std::vector<double>{10.0, 20.0}));
}

TEST(Scale, NothingToScaleIsFine)
{
    ch01::scale({}, 3.0);
    SUCCEED();
}

}  // namespace
