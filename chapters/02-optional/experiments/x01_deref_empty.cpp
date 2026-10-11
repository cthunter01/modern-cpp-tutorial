// Break it on purpose, chapter 2: use the value of an optional without asking whether there is one.
//
//   cmake --build --preset clang-debug -t x02_01 && build/clang-debug/bin/x02_01

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
    //! [main]
    const std::optional<unsigned> port = parse_port("http");  // not a number: the optional is empty

    std::cout << "listening on port " << *port << std::endl;  // nobody asked
    //! [main]
    return 0;
}
