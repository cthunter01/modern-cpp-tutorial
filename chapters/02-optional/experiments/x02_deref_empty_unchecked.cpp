// Break it on purpose, chapter 2: the same program as x01, built the way a release build is.
//
//   cmake --build --preset clang-debug -t x02_02 && build/clang-debug/bin/x02_02

// The tutorial's debug builds make the standard library check its preconditions, and an unoptimised
// build with GCC's library does so by default. An optimised release build normally does not. These
// two lines, in front of the first #include, turn this one file into such a build.
#undef _GLIBCXX_ASSERTIONS
#define _GLIBCXX_NO_ASSERTIONS 1

#include <charconv>
#include <iostream>
#include <optional>
#include <string_view>
#include <system_error>

namespace
{

std::optional<unsigned> parse_port(std::string_view text)
{
    unsigned          value       = 0;
    const char* const end         = text.data() + text.size();
    const auto [stoppedAt, error] = std::from_chars(text.data(), end, value);
    if (error != std::errc{} || stoppedAt != end)
    {
        return std::nullopt;
    }
    return value;
}

}  // namespace

int main()
{
    const std::optional<unsigned> port = parse_port("http");

    std::cout << "listening on port " << *port << std::endl;
    std::cout << "and nothing stopped me" << std::endl;
    return 0;
}
