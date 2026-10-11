#include "d02_journal.h"

#include <string>

#include <gtest/gtest.h>

namespace
{

TEST(Journal, ReportsHowManyCharactersWereAdded)
{
    review_a::Journal journal;

    EXPECT_EQ(journal.Add("short"), 5U);
    EXPECT_EQ(journal.Add(std::string(300, 'x')), 300U);
    EXPECT_EQ(journal.Count(), 2U);
}

}  // namespace
