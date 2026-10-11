#include "p04_parse_level.h"

#include <optional>

#include <gtest/gtest.h>

#include "level.h"

// These are the logger's own tests of parse_level (tests/LevelTests.cpp), unchanged except for the
// namespace.

namespace
{

using ch03::Level;

TEST(Level, ParseIsCaseInsensitive)
{
    EXPECT_EQ(ch03::parse_level("off"), Level::Off);
    EXPECT_EQ(ch03::parse_level("ERROR"), Level::Error);
    EXPECT_EQ(ch03::parse_level("Warn"), Level::Warn);
    EXPECT_EQ(ch03::parse_level("iNfO"), Level::Info);
    EXPECT_EQ(ch03::parse_level("debug"), Level::Debug);
    EXPECT_EQ(ch03::parse_level("PERF"), Level::Perf);
}

TEST(Level, ParseRejectsUnknownAndEmpty)
{
    EXPECT_EQ(ch03::parse_level(""), std::nullopt);
    EXPECT_EQ(ch03::parse_level("verbose"), std::nullopt);
    EXPECT_EQ(ch03::parse_level("warning"), std::nullopt);
    EXPECT_EQ(ch03::parse_level(" info"), std::nullopt);
    EXPECT_EQ(ch03::parse_level("3"), std::nullopt);
}

TEST(Level, ParseRejectsABeginningOfAName)
{
    EXPECT_EQ(ch03::parse_level("war"), std::nullopt);
    EXPECT_EQ(ch03::parse_level("o"), std::nullopt);
}

}  // namespace
