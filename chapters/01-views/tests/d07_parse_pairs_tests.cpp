#include "d07_parse_pairs.h"

#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

namespace
{

TEST(ParsePairs, ReadsKeysAndValues)
{
    const std::vector<ch01::KeyValue> pairs = ch01::parse_pairs("host=example.org;port=8080");

    ASSERT_EQ(pairs.size(), 2U);
    EXPECT_EQ(pairs[0].key, "host");
    EXPECT_EQ(pairs[0].value, "example.org");
    EXPECT_EQ(pairs[1].key, "port");
    EXPECT_EQ(pairs[1].value, "8080");
}

TEST(ParsePairs, TrimsKeysAndValues)
{
    const std::vector<ch01::KeyValue> pairs = ch01::parse_pairs("  a = 1 ;\tb=two words  ");

    ASSERT_EQ(pairs.size(), 2U);
    EXPECT_EQ(pairs[0].key, "a");
    EXPECT_EQ(pairs[0].value, "1");
    EXPECT_EQ(pairs[1].key, "b");
    EXPECT_EQ(pairs[1].value, "two words");
}

TEST(ParsePairs, SkipsPiecesThatAreNotPairs)
{
    // No '=' at all, an empty key, and an empty piece between two semicolons.
    const std::vector<ch01::KeyValue> pairs = ch01::parse_pairs("a=1;oops; =3;;b=2");

    ASSERT_EQ(pairs.size(), 2U);
    EXPECT_EQ(pairs[0].key, "a");
    EXPECT_EQ(pairs[1].key, "b");
}

TEST(ParsePairs, AllowsAnEmptyValueAndAnEqualsSignInTheValue)
{
    const std::vector<ch01::KeyValue> pairs = ch01::parse_pairs("empty=;formula=a=b");

    ASSERT_EQ(pairs.size(), 2U);
    EXPECT_EQ(pairs[0].key, "empty");
    EXPECT_TRUE(pairs[0].value.empty());
    EXPECT_EQ(pairs[1].key, "formula");
    EXPECT_EQ(pairs[1].value, "a=b");
}

TEST(ParsePairs, KeysAndValuesArePartsOfTheInput)
{
    const std::string owner = "name = value";

    const std::vector<ch01::KeyValue> pairs = ch01::parse_pairs(owner);

    ASSERT_EQ(pairs.size(), 1U);
    EXPECT_EQ(pairs[0].key.data(), owner.data());
    EXPECT_EQ(pairs[0].value.data(), &owner[7]);
}

TEST(ParsePairs, NothingInNothingOut)
{
    EXPECT_TRUE(ch01::parse_pairs("").empty());
    EXPECT_TRUE(ch01::parse_pairs(" ; ; ").empty());
}

}  // namespace
