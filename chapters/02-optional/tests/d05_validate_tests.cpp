#include "d05_validate.h"

#include <optional>
#include <string>

#include <gtest/gtest.h>

namespace
{

ch02::Settings valid()
{
    return {.name = "main window", .width = 800, .height = 600, .opacity = 0.9};
}

TEST(Validate, ValidSettingsGiveNothing)
{
    // For this function "nothing" is the good outcome: there is no complaint.
    EXPECT_EQ(ch02::validate(valid()), std::nullopt);
}

TEST(Validate, TheLimitsThemselvesAreValid)
{
    ch02::Settings settings = valid();
    settings.width          = 1;
    settings.height         = 10'000;
    settings.opacity        = 0.0;
    EXPECT_EQ(ch02::validate(settings), std::nullopt);
}

TEST(Validate, ComplainsAboutAnEmptyName)
{
    ch02::Settings settings = valid();
    settings.name.clear();

    const std::optional<std::string> complaint = ch02::validate(settings);

    EXPECT_TRUE(complaint.has_value());
    const std::string text = complaint.value_or("");
    EXPECT_TRUE(text.contains("name")) << text;
}

TEST(Validate, ComplainsAboutTheWidth)
{
    ch02::Settings settings = valid();
    settings.width          = 0;

    const std::optional<std::string> complaint = ch02::validate(settings);

    EXPECT_TRUE(complaint.has_value());
    const std::string text = complaint.value_or("");
    EXPECT_TRUE(text.contains("width")) << text;
}

TEST(Validate, ComplainsAboutTheHeight)
{
    ch02::Settings settings = valid();
    settings.height         = 10'001;

    const std::optional<std::string> complaint = ch02::validate(settings);

    EXPECT_TRUE(complaint.has_value());
    const std::string text = complaint.value_or("");
    EXPECT_TRUE(text.contains("height")) << text;
}

TEST(Validate, ComplainsAboutTheOpacity)
{
    ch02::Settings settings = valid();
    settings.opacity        = 1.5;

    const std::optional<std::string> complaint = ch02::validate(settings);

    EXPECT_TRUE(complaint.has_value());
    const std::string text = complaint.value_or("");
    EXPECT_TRUE(text.contains("opacity")) << text;
}

TEST(Validate, ReportsTheFirstOfSeveralProblems)
{
    ch02::Settings settings = valid();
    settings.height         = -5;
    settings.width          = -5;
    settings.opacity        = 7.0;

    const std::optional<std::string> complaint = ch02::validate(settings);

    EXPECT_TRUE(complaint.has_value());
    const std::string text = complaint.value_or("");
    EXPECT_TRUE(text.contains("width")) << text;
    EXPECT_FALSE(text.contains("height")) << text;
}

}  // namespace
