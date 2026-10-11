// The listings of chapter 1: what a std::string_view and a std::span accept, and what slicing them
// does.

#include <array>
#include <cstddef>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace
{

//! [text-param]
std::size_t count_spaces(std::string_view text) noexcept
{
    std::size_t count = 0;
    for (const char c : text)
    {
        count += (c == ' ') ? 1U : 0U;
    }
    return count;
}
//! [text-param]

void text_sources()
{
    //! [text-sources]
    const std::string         owned = "a std::string owns its characters";
    const std::array<char, 9> raw{'n', 'o', ' ', 'n', 'u', 'l', ' ', 'h', 'i'};

    std::cout << count_spaces("a literal, in the program's read-only data") << '\n';
    std::cout << count_spaces(owned) << '\n';  // implicit conversion
    std::cout << count_spaces(std::string_view(owned).substr(0, 13)) << '\n';  // "a std::string"
    std::cout << count_spaces({raw.data(), raw.size()}) << '\n';               // pointer and length
    //! [text-sources]
}

void slicing_text()
{
    //! [text-slicing]
    const std::string_view line = "  timeout = 30s  ";

    const std::size_t      equals = line.find('=');           // 10
    const std::string_view key    = line.substr(0, equals);   // "  timeout "
    std::string_view       value  = line.substr(equals + 1);  // " 30s  "

    value.remove_prefix(1);  // "30s  "   move the start forward
    value.remove_suffix(2);  // "30s"     move the end back

    std::cout << '[' << key << "] [" << value << "]\n";
    std::cout << std::boolalpha << value.ends_with('s') << ' ' << line.starts_with("  time") << ' '
              << line.contains("out") << '\n';
    //! [text-slicing]

    //! [text-part-of]
    // Nothing was copied: `value` is three of the characters of the literal above.
    std::cout << (value.data() == &line[12]) << '\n';
    //! [text-part-of]
}

//! [span-param]
int sum(std::span<const int> values) noexcept
{
    int total = 0;
    for (const int value : values)
    {
        total += value;
    }
    return total;
}
//! [span-param]

void span_sources()
{
    //! [span-sources]
    const std::vector<int>   vector{1, 2, 3, 4};
    const std::array<int, 3> array{10, 20, 30};
    const int                builtIn[] = {100, 200};  // NOLINT(*-avoid-c-arrays): on purpose
    const int                single    = 7;

    std::cout << sum(vector) << '\n';              // 10
    std::cout << sum(array) << '\n';               // 60
    std::cout << sum(builtIn) << '\n';             // 300
    std::cout << sum({&single, 1}) << '\n';        // 7     one object is a sequence of one
    std::cout << sum({vector.data(), 2}) << '\n';  // 3     pointer and length
    std::cout << sum(std::span(vector).subspan(1, 2)) << '\n';  // 5     elements 1 and 2
    //! [span-sources]
}

//! [span-mutable]
void fill(std::span<int> values, int with) noexcept
{
    for (int& value : values)
    {
        value = with;
    }
}
//! [span-mutable]

void span_constness()
{
    //! [span-const]
    std::vector<int> numbers{1, 2, 3, 4, 5};

    const std::span<int> fixedView(numbers);  // the view cannot be re-pointed...
    fixedView[0] = 99;                        // ...but the elements are still ints, and writable

    const std::span<const int> readOnly(numbers);  // const elements: this is the read-only view
    // readOnly[0] = 1;                             // does not compile

    fill(std::span(numbers).last(2), 0);  // 99 2 3 0 0
    //! [span-const]
    for (const int number : readOnly)
    {
        std::cout << number << ' ';
    }
    std::cout << '\n';
}

void span_slicing()
{
    //! [span-slicing]
    const std::array<int, 6>   samples{0, 1, 2, 3, 4, 5};
    const std::span<const int> all(samples);

    std::cout << sum(all.first(2)) << ' '         // 0 1
              << sum(all.last(2)) << ' '          // 4 5
              << sum(all.subspan(2)) << ' '       // 2 3 4 5
              << sum(all.subspan(2, 2)) << '\n';  // 2 3
    std::cout << std::as_bytes(all).size() << " bytes\n";
    //! [span-slicing]
}

}  // namespace

int main()
{
    text_sources();
    slicing_text();
    span_sources();
    span_constness();
    span_slicing();
    return 0;
}
