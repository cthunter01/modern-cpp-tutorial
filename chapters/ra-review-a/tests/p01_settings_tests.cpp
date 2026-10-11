#include "p01_settings.h"

#include <string>
#include <string_view>

#include <gtest/gtest.h>

// The tests fix a class review_a::Settings, built from the text of a settings file, with three
// getters: Text(key), Number(key) and Flag(key). What a getter returns answers `if (result)`; when
// that is true, `*result` is the value. When it is false, review_a::is_missing(result) tells the
// two reasons apart: true if the key is not there, false if it is there and cannot be read.

namespace
{

constexpr std::string_view kFile =
    "# demo settings\n"
    "name = demo server\n"
    "port = 8080\n"
    "\n"
    "  retries=3  \n"
    "verbose = on\n"
    "colour = false\n"
    "timeout = soon\n"
    "debug = maybe\n"
    "motto =\n"
    "port-backup = 80 81\n";

TEST(Settings, ReadsTextNumbersAndFlags)
{
    const review_a::Settings settings(kFile);

    const auto name    = settings.Text("name");
    const auto port    = settings.Number("port");
    const auto retries = settings.Number("retries");
    const auto verbose = settings.Flag("verbose");
    const auto colour  = settings.Flag("colour");

    if (!name || !port || !retries || !verbose || !colour)
    {
        FAIL() << "all five are in the file and well formed";
    }
    EXPECT_EQ(*name, "demo server");
    EXPECT_EQ(*port, 8080);
    EXPECT_EQ(*retries, 3);
    EXPECT_TRUE(*verbose);
    EXPECT_FALSE(*colour) << "false is a value: the setting is there, and it says no";
}

TEST(Settings, AKeyThatIsNotThereIsMissing)
{
    const review_a::Settings settings(kFile);

    const auto host = settings.Text("host");
    const auto size = settings.Number("size");
    const auto fast = settings.Flag("fast");

    EXPECT_FALSE(host);
    EXPECT_FALSE(size);
    EXPECT_FALSE(fast);
    EXPECT_TRUE(review_a::is_missing(host));
    EXPECT_TRUE(review_a::is_missing(size));
    EXPECT_TRUE(review_a::is_missing(fast));
}

TEST(Settings, AValueOfTheWrongKindIsMalformedNotMissing)
{
    const review_a::Settings settings(kFile);

    const auto timeout = settings.Number("timeout");      // "soon"
    const auto backup  = settings.Number("port-backup");  // "80 81"
    const auto debug   = settings.Flag("debug");          // "maybe"

    EXPECT_FALSE(timeout);
    EXPECT_FALSE(backup);
    EXPECT_FALSE(debug);
    EXPECT_FALSE(review_a::is_missing(timeout)) << "the key is there; its value is the problem";
    EXPECT_FALSE(review_a::is_missing(backup));
    EXPECT_FALSE(review_a::is_missing(debug));
}

TEST(Settings, AnEmptyValueIsATextAndNotANumber)
{
    const review_a::Settings settings(kFile);

    const auto asText   = settings.Text("motto");
    const auto asNumber = settings.Number("motto");

    if (!asText)
    {
        FAIL() << "motto is in the file, with an empty value";
    }
    EXPECT_EQ(*asText, "");
    EXPECT_FALSE(asNumber);
    EXPECT_FALSE(review_a::is_missing(asNumber));
}

TEST(Settings, CommentsAndBlankLinesAreNotSettings)
{
    const review_a::Settings settings(kFile);
    EXPECT_TRUE(review_a::is_missing(settings.Text("# demo settings")));
    EXPECT_TRUE(review_a::is_missing(settings.Text("")));
}

TEST(Settings, DoNotDependOnTheTextTheyWereReadFrom)
{
    // The file is read into a buffer, the settings are built, and the buffer is reused for the next
    // file. The settings must not care.
    std::string              buffer = "port = 8080\nname = demo\n";
    const review_a::Settings settings(buffer);

    buffer.assign(buffer.size(), '?');

    const auto port = settings.Number("port");
    const auto name = settings.Text("name");
    if (!port || !name)
    {
        FAIL() << "the settings forgot what they had read when the buffer changed";
    }
    EXPECT_EQ(*port, 8080);
    EXPECT_EQ(*name, "demo");
}

}  // namespace
