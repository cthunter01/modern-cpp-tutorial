#include "p01_section_body.h"

#include <cstddef>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "tut/allocation_count.h"

// The tests fix the call, section_body(text, name), and that the result can be looked at as a
// std::string_view. They also measure two things a correct answer could get wrong without anyone
// noticing: whether it allocates, and whether what it returns lives inside the caller's text.

namespace
{

constexpr std::string_view kConfig =
    "[server]\n"
    "port = 8080\n"
    "host = example.org\n"
    "\n"
    "[client]\n"
    "retries = 3\n"
    "[empty]\n"
    "[last]\n"
    "tail = yes";

TEST(SectionBody, ReturnsTheLinesOfTheSection)
{
    auto&& body = ch01::section_body(kConfig, "server");
    EXPECT_EQ(std::string_view(body), "port = 8080\nhost = example.org\n\n");
}

TEST(SectionBody, ASectionEndsWhereTheNextOneStarts)
{
    auto&& body = ch01::section_body(kConfig, "client");
    EXPECT_EQ(std::string_view(body), "retries = 3\n");
}

TEST(SectionBody, TheLastSectionRunsToTheEndOfTheText)
{
    auto&& body = ch01::section_body(kConfig, "last");
    EXPECT_EQ(std::string_view(body), "tail = yes");
}

TEST(SectionBody, ASectionCanBeEmpty)
{
    auto&& body = ch01::section_body(kConfig, "empty");
    EXPECT_TRUE(std::string_view(body).empty());
}

TEST(SectionBody, AMissingSectionGivesAnEmptyBody)
{
    auto&& missing = ch01::section_body(kConfig, "database");
    EXPECT_TRUE(std::string_view(missing).empty());

    // "serve" is the beginning of a section name, and "port" is a key: neither is a section.
    auto&& prefix = ch01::section_body(kConfig, "serve");
    EXPECT_TRUE(std::string_view(prefix).empty());
    auto&& key = ch01::section_body(kConfig, "port");
    EXPECT_TRUE(std::string_view(key).empty());
}

TEST(SectionBody, DoesNotAllocate)
{
    // A real file: big, and read into memory once. Looking a section up must not copy any of it.
    std::string file;
    for (int i = 0; i < 1000; ++i)
    {
        file += "[section" + std::to_string(i) + "]\nkey = a value long enough to need the heap\n";
    }
    const std::string_view text = file;

    const tut::AllocationCount allocations;
    auto&&                     body = ch01::section_body(text, "section999");
    const std::size_t          made = allocations.Count();

    EXPECT_EQ(made, 0U) << "section_body allocated memory " << made << " time(s)";
    EXPECT_EQ(std::string_view(body), "key = a value long enough to need the heap\n");
}

TEST(SectionBody, TheResultIsAPartOfTheCallersText)
{
    const std::string      file = std::string(kConfig);
    const std::string_view text = file;

    auto&&                 result = ch01::section_body(text, "client");
    const std::string_view body   = result;

    EXPECT_GE(body.begin(), text.begin());
    EXPECT_LE(body.end(), text.end());
}

}  // namespace
