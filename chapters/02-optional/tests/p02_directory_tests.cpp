#include "p02_directory.h"

#include <gtest/gtest.h>

// The tests fix: a struct ch02::Profile with members name, nickname and managerId, in that order.
// They are filled in with designated initialisers, and `{}` for a nickname or a manager must mean
// "there is none". And a class ch02::Directory with Add(id, profile), DisplayName(id) and
// ManagerName(id), the last two returning something that compares with a string. The types of the
// members are yours.

namespace
{

ch02::Directory staff()
{
    ch02::Directory directory;
    directory.Add(0, {.name = "Grace Hopper", .nickname = {}, .managerId = {}});
    directory.Add(1, {.name = "Margaret Hamilton", .nickname = "Maggie", .managerId = 0});
    directory.Add(2, {.name = "Katherine Johnson", .nickname = {}, .managerId = 1});
    directory.Add(
        3,
        {.name = "Ada Lovelace", .nickname = {}, .managerId = 77});  // 77 is not in the directory
    return directory;
}

TEST(Directory, DisplayNameIsTheNicknameWhenThereIsOne)
{
    EXPECT_EQ(staff().DisplayName(1), "Maggie");
}

TEST(Directory, DisplayNameIsTheNameOtherwise)
{
    EXPECT_EQ(staff().DisplayName(0), "Grace Hopper");
    EXPECT_EQ(staff().DisplayName(2), "Katherine Johnson");
}

TEST(Directory, DisplayNameOfAnUnknownIdSaysSo)
{
    EXPECT_EQ(staff().DisplayName(42), "<unknown #42>");
}

TEST(Directory, ManagerNameIsTheManagersDisplayName)
{
    EXPECT_EQ(staff().ManagerName(2), "Maggie");
}

TEST(Directory, IdZeroIsAPersonLikeAnyOther)
{
    // Grace Hopper has id 0 and manages Margaret Hamilton. A design that uses 0 for "no manager"
    // cannot say this.
    EXPECT_EQ(staff().ManagerName(1), "Grace Hopper");
}

TEST(Directory, ManagerNameIsNobodyWhenALinkIsMissing)
{
    EXPECT_EQ(staff().ManagerName(0), "nobody");   // has no manager
    EXPECT_EQ(staff().ManagerName(3), "nobody");   // names a manager who is not listed
    EXPECT_EQ(staff().ManagerName(42), "nobody");  // is not listed herself
}

TEST(Directory, AnEmptyNicknameIsNotANickname)
{
    ch02::Directory directory;
    directory.Add(5, {.name = "Hedy Lamarr", .nickname = {}, .managerId = {}});
    EXPECT_EQ(directory.DisplayName(5), "Hedy Lamarr");
}

}  // namespace
