#include "d08_label.h"

#include <string>
#include <string_view>
#include <type_traits>

#include <gtest/gtest.h>

namespace
{

TEST(Label, IsBuiltFromAnythingAStringIsBuiltFrom)
{
    const std::string      owned = "owned";
    const std::string_view view  = "view";

    EXPECT_EQ(ch05::Label("literal").Text(), "literal");
    EXPECT_EQ(ch05::Label(owned).Text(), "owned");
    EXPECT_EQ(ch05::Label(view).Text(), "view");
    EXPECT_EQ(ch05::Label(std::string(3, 'x')).Text(), "xxx");

    // And from nothing else. As long as the constructor takes any U at all, the compiler believes
    // that a Label can be built from a double or from a vector, and reports the truth only when
    // somebody tries.
    EXPECT_FALSE((std::is_constructible_v<ch05::Label, double>));
    EXPECT_FALSE((std::is_constructible_v<ch05::Label, int*>));
}

}  // namespace
