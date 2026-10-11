// Break it on purpose, chapter 3: compare two texts with the iterator form of std::equal.
//
//   cmake --build --preset clang-debug -t x03_01 && build/clang-debug/bin/x03_01

#include <algorithm>
#include <iostream>
#include <string_view>

namespace
{

//! [functions]
// The form C++98 had: one whole range, and only the *start* of a second one.
bool same_old(std::string_view a, std::string_view b)
{
    return std::equal(a.begin(), a.end(), b.begin());  // NOLINT(modernize-use-ranges)
}

// The range form knows where both texts end.
bool same_new(std::string_view a, std::string_view b)
{
    return std::ranges::equal(a, b);
}
//! [functions]

}  // namespace

int main()
{
    std::cout << std::boolalpha;
    std::cout << "std::equal:         \"war\" and \"warn\" are equal: " << same_old("war", "warn")
              << '\n';
    std::cout << "std::ranges::equal: \"war\" and \"warn\" are equal: " << same_new("war", "warn")
              << '\n';
    return 0;
}
