#include "d01_categories.h"

#include <cstddef>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>

#include "tut/tracked.h"
#include "which.h"

// Each test makes one of the eight calls and compares what really happened with your prediction.

namespace
{

tut::Tracked make()
{
    return tut::Tracked{4};
}

// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved): expression 6 is the question
std::string_view inside_sink_plain(tut::Tracked&& parameter)
{
    return ch04::which(parameter);
}

std::string_view inside_sink_moved(tut::Tracked&& parameter)
{
    return ch04::which(std::move(parameter));
}

void check(std::size_t number, std::string_view actual)
{
    EXPECT_EQ(ch04::kPredictions.at(number - 1), actual)
        << "your prediction for expression " << number << " is not what the compiler does";
}

TEST(Categories, Expression1AName)
{
    const tut::Tracked named{1};
    check(1, ch04::which(named));
}

TEST(Categories, Expression2ATemporary)
{
    check(2, ch04::which(tut::Tracked{3}));
}

TEST(Categories, Expression3AMovedName)
{
    tut::Tracked named{1};
    check(3, ch04::which(std::move(named)));
}

TEST(Categories, Expression4TheResultOfACall)
{
    check(4, ch04::which(make()));
}

TEST(Categories, Expression5AMovedConstObject)
{
    const tut::Tracked fixed{2};
    check(5, ch04::which(std::move(fixed)));  // NOLINT(performance-move-const-arg): the question
}

TEST(Categories, Expression6ANamedRvalueReference)
{
    check(6, inside_sink_plain(tut::Tracked{6}));
}

TEST(Categories, Expression7ANamedRvalueReferenceMovedAgain)
{
    check(7, inside_sink_moved(tut::Tracked{7}));
}

TEST(Categories, Expression8TheCastThatMoveIs)
{
    tut::Tracked named{1};
    check(8, ch04::which(static_cast<tut::Tracked&&>(named)));
}

}  // namespace
