// Break it on purpose, chapter 2: test an optional<bool> as if it were a bool.
//
//   cmake --build --preset clang-debug -t x02_04 && build/clang-debug/bin/x02_04

#include <iostream>
#include <optional>
#include <string_view>

namespace
{

std::optional<bool> parse_bool(std::string_view text)
{
    if (text == "true")
    {
        return true;
    }
    if (text == "false")
    {
        return false;
    }
    return std::nullopt;
}

}  // namespace

int main()
{
    //! [main]
    const std::optional<bool> verbose = parse_bool("false");  // the user asked for quiet

    if (verbose)  // true: there IS an answer
    {
        std::cout << "verbose output is ON\n";
    }
    else
    {
        std::cout << "verbose output is off\n";
    }
    //! [main]
    return 0;
}
