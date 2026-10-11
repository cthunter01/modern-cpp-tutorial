#include "d03_name_of.h"

#include <optional>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "person.h"

namespace
{

const std::vector<ch03::Person>& people()
{
    static const std::vector<ch03::Person> kPeople{
        {.name = "Ada", .age = 36, .id = 7},
        {.name = "Grace", .age = 85, .id = 3},
        {.name = "Linus", .age = 54, .id = 0},
    };
    return kPeople;
}

TEST(NameOf, FindsByNumber)
{
    EXPECT_EQ(ch03::name_of(people(), 3), "Grace");
    EXPECT_EQ(ch03::name_of(people(), 7), "Ada");
}

TEST(NameOf, IdZeroIsAnIdLikeAnyOther)
{
    EXPECT_EQ(ch03::name_of(people(), 0), "Linus");
}

TEST(NameOf, AnUnknownIdGivesNothing)
{
    EXPECT_EQ(ch03::name_of(people(), 99), std::nullopt);
    EXPECT_EQ(ch03::name_of({}, 3), std::nullopt);
}

TEST(NameOf, TheNameIsAPartOfThePeople)
{
    const std::optional<std::string_view> name = ch03::name_of(people(), 3);

    EXPECT_EQ(name.value_or("").data(), people()[1].name.data());
}

}  // namespace
