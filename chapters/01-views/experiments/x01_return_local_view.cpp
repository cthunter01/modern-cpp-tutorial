// Break it on purpose, chapter 1: return a view of a local string.
//
//   cmake --build --preset clang-debug -t x01_01
//   cmake --build --preset gcc-debug   -t x01_01 && build/gcc-debug/bin/x01_01

#include <iostream>
#include <string>
#include <string_view>

namespace
{

//! [function]
std::string_view full_name(const std::string& first, const std::string& last)
{
    const std::string joined =
        first + " " + last;  // a local: it is destroyed when the function returns
    return joined;           // ... and this is a view of its characters
}
//! [function]

}  // namespace

int main()
{
    const std::string_view name = full_name("Ada", "Lovelace, Countess of Lovelace");
    std::cout << "the name has " << name.size() << " characters\n";
    std::cout << "and it is: " << name << '\n';
    return 0;
}
