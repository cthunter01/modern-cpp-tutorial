#include "d06_fixed_stack.h"

#include <string>
#include <utility>

#include <gtest/gtest.h>

// Whether a variable is const decides what is deduced from it and whether it can be moved from, so
// the variables in this file are declared the way each test needs them.
//
// NOLINTBEGIN(misc-const-correctness)

#include "tut/tracked.h"

namespace
{

TEST(FixedStack, EmplaceBuildsTheElementFromItsConstructorArguments)
{
    ch05::FixedStack<std::pair<int, std::string>, 4> stack;

    const std::pair<int, std::string>& top = stack.Emplace(7, "seven");

    EXPECT_EQ(top.first, 7);
    EXPECT_EQ(top.second, "seven");
    EXPECT_EQ(stack.Size(), 1U);
    EXPECT_EQ(&top, &stack.Top());
}

TEST(FixedStack, EmplaceConstructsInPlace)
{
    ch05::FixedStack<tut::Tracked, 4> stack;
    tut::Tracked::ResetCounts();

    stack.Emplace(5);  // the int goes all the way to Tracked's constructor

    EXPECT_EQ(stack.Top().Value(), 5);
    EXPECT_EQ(tut::Tracked::Counts().copies, 0);
    EXPECT_EQ(tut::Tracked::Counts().moves, 0)
        << "a Tracked was built somewhere else and then moved in";
}

TEST(FixedStack, EmplaceWithAnExistingObjectCopiesOrMovesAsTheCallerSays)
{
    ch05::FixedStack<tut::Tracked, 4> stack;
    tut::Tracked                      kept{1};
    tut::Tracked                      given{2};
    tut::Tracked::ResetCounts();

    stack.Emplace(kept);
    stack.Emplace(std::move(given));

    EXPECT_EQ(tut::Tracked::Counts().copies, 1);
    EXPECT_EQ(tut::Tracked::Counts().moves, 1);
    EXPECT_FALSE(kept.IsMovedFrom());
}

TEST(FixedStack, PopRemovesTheTop)
{
    ch05::FixedStack<std::string, 3> stack;
    stack.Emplace("bottom");
    stack.Emplace(3, 'x');  // std::string(3, 'x')

    EXPECT_EQ(stack.Top(), "xxx");
    stack.Pop();
    EXPECT_EQ(stack.Top(), "bottom");
    EXPECT_EQ(stack.Size(), 1U);
}

}  // namespace

// NOLINTEND(misc-const-correctness)
