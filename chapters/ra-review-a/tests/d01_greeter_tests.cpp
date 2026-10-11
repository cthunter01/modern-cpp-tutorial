#include "d01_greeter.h"

#include <string>

#include <gtest/gtest.h>

namespace
{

TEST(Greeter, GreetsTheNameItWasGiven)
{
    const review_a::Greeter greeter("Ada");
    EXPECT_EQ(greeter.Greet(), "Hello, Ada!");
}

TEST(Greeter, KeepsItsNameWhenTheCallersVariableChanges)
{
    std::string             visitor = "Ada";
    const review_a::Greeter greeter(visitor);

    visitor = "Bob";  // the caller reuses their variable for the next visitor

    EXPECT_EQ(greeter.Greet(), "Hello, Ada!");
}

}  // namespace
