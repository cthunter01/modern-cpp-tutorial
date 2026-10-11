#include "p03_factory.h"

#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "widgets.h"

// The tests fix a class review_a::WidgetRegistry with
//   Register<T>(name, arguments...)   T is one of the widget classes of widgets.h
//   Create(name)                      answers `if (widget)`; widget->Describe() is the new widget's
// and nothing about how the registry remembers what it was told.

namespace
{

TEST(WidgetRegistry, CreatesWhatWasRegistered)
{
    review_a::WidgetRegistry registry;
    registry.Register<review_a::Button>("ok", "OK");
    registry.Register<review_a::Slider>("volume", 0, 11);

    const auto button = registry.Create("ok");
    const auto slider = registry.Create("volume");

    ASSERT_TRUE(button);
    ASSERT_TRUE(slider);
    EXPECT_EQ(button->Describe(), "button 'OK'");
    EXPECT_EQ(slider->Describe(), "slider 0..11");
}

TEST(WidgetRegistry, AnUnknownNameCreatesNothing)
{
    review_a::WidgetRegistry registry;
    registry.Register<review_a::Button>("ok", "OK");

    EXPECT_FALSE(registry.Create("cancel"));
    EXPECT_FALSE(registry.Create(""));
}

TEST(WidgetRegistry, EveryCreateMakesANewWidgetFromTheSameArguments)
{
    review_a::WidgetRegistry registry;
    std::string              caption(100, 'x');
    registry.Register<review_a::Button>("wide", std::move(caption));  // given up by the caller

    const auto first  = registry.Create("wide");
    const auto second = registry.Create("wide");
    const auto third  = registry.Create("wide");

    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(third);
    EXPECT_NE(first.get(), second.get());
    EXPECT_EQ(first->Describe().size(), 109U);
    EXPECT_EQ(second->Describe().size(), 109U)
        << "the second widget did not get the caption the first one got";
    EXPECT_EQ(third->Describe().size(), 109U);
}

TEST(WidgetRegistry, RemembersTheArgumentsAsTheyWereWhenRegistered)
{
    review_a::WidgetRegistry registry;
    std::string              caption = "Save";
    registry.Register<review_a::Button>("save", caption);  // kept by the caller...

    caption = "Something else entirely";  // ...and changed afterwards

    const auto button = registry.Create("save");
    ASSERT_TRUE(button);
    EXPECT_EQ(button->Describe(), "button 'Save'");
}

TEST(WidgetRegistry, RegisteringANameAgainReplacesIt)
{
    review_a::WidgetRegistry registry;
    registry.Register<review_a::Button>("control", "old");
    registry.Register<review_a::Slider>("control", 1, 5);

    const auto control = registry.Create("control");
    ASSERT_TRUE(control);
    EXPECT_EQ(control->Describe(), "slider 1..5");
}

}  // namespace
