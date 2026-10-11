#include "d07_make_drawn.h"

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

#include "shapes.h"
#include "tut/tracked.h"

namespace
{

TEST(MakeDrawn, ConstructsTheShapeFromTheGivenArguments)
{
    ch05::Canvas canvas;

    const auto circle    = ch05::make_drawn<ch05::Circle>(canvas, 2.0);
    const auto rectangle = ch05::make_drawn<ch05::Rectangle>(canvas, 3.0, 4.0);

    EXPECT_DOUBLE_EQ(circle->Radius(), 2.0);
    EXPECT_DOUBLE_EQ(rectangle->Area(), 12.0);
}

TEST(MakeDrawn, ReturnsTheExactTypeNotTheBase)
{
    ch05::Canvas canvas;

    // The caller asked for a Circle and gets a pointer to a Circle, with which Radius() can be
    // called.
    static_assert(std::is_same_v<decltype(ch05::make_drawn<ch05::Circle>(canvas, 1.0)),
                                 std::shared_ptr<ch05::Circle>>);
    EXPECT_DOUBLE_EQ(ch05::make_drawn<ch05::Circle>(canvas, 1.0)->Radius(), 1.0);
}

TEST(MakeDrawn, TheCanvasAndTheCallerShareOneObject)
{
    ch05::Canvas canvas;

    const std::shared_ptr<ch05::Circle> circle = ch05::make_drawn<ch05::Circle>(canvas, 2.0);

    ASSERT_EQ(canvas.shapes.size(), 1U);
    EXPECT_EQ(canvas.shapes.front().get(), circle.get());
    EXPECT_EQ(circle.use_count(), 2);
}

TEST(MakeDrawn, ConstructorArgumentsAreForwardedNotCopied)
{
    ch05::Canvas canvas;
    std::string  caption(200, 'c');
    tut::Tracked payload{7};
    tut::Tracked::ResetCounts();

    const auto shape =
        ch05::make_drawn<ch05::Labelled>(canvas, std::move(caption), std::move(payload));

    EXPECT_EQ(shape->Label().size(), 200U);
    EXPECT_EQ(shape->Payload().Value(), 7);
    EXPECT_EQ(tut::Tracked::Counts().copies, 0)
        << "the caller gave the payload up, and it was copied on the way";
}

TEST(MakeDrawn, AnArgumentTheCallerKeepsIsCopiedOnce)
{
    ch05::Canvas       canvas;
    const tut::Tracked payload{7};
    tut::Tracked::ResetCounts();

    const auto shape = ch05::make_drawn<ch05::Labelled>(canvas, "kept", payload);

    EXPECT_EQ(shape->Payload().Value(), 7);
    EXPECT_EQ(tut::Tracked::Counts().copies, 1);
    EXPECT_FALSE(payload.IsMovedFrom());
}

}  // namespace
