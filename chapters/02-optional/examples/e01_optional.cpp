// The listings of chapter 2: returning, testing, unpacking and chaining std::optional.

#include <array>
#include <charconv>
#include <cstdint>
#include <expected>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace old_ways
{

//! [sentinel]
// The old ways of saying "there may be no answer".
int  stock_or_minus_one(std::string_view name);     // a value that means "none": which one is safe?
bool get_stock(std::string_view name, int& stock);  // an out-parameter, and a result easy to ignore
const int* find_stock(
    std::string_view name);  // a pointer: fine for "one of mine", not for a new value
//! [sentinel]

}  // namespace old_ways

namespace
{

struct Item
{
    std::string_view name;
    int              stock;
};

constexpr std::array kShelf{
    Item{.name = "apple", .stock = 3},
    Item{.name = "pear", .stock = 0},
    Item{.name = "plum", .stock = 12},
};

//! [return]
std::optional<int> stock_of(std::string_view name)
{
    for (const Item& item : kShelf)
    {
        if (item.name == name)
        {
            return item.stock;  // an int converts to an optional that holds it
        }
    }
    return std::nullopt;  // the optional that holds nothing
}
//! [return]

void using_one()
{
    //! [use]
    if (const std::optional<int> stock = stock_of("pear"))  // true: there is an answer
    {
        std::cout << "pear: " << *stock << " in stock\n";  // and the answer is 0
    }

    const std::optional<int> missing = stock_of("mango");
    std::cout << std::boolalpha << missing.has_value() << '\n';  // false
    std::cout << missing.value_or(-1) << '\n';  // -1: a default, chosen by the caller
    //! [use]

    //! [compare]
    std::cout << (stock_of("pear") == 0) << ' '              // true:  holds a value equal to 0
              << (stock_of("mango") == 0) << ' '             // false: holds nothing
              << (stock_of("mango") == std::nullopt) << ' '  // true
              << (stock_of("mango") < stock_of("pear"))
              << '\n';  // true:  nothing sorts before anything
    //! [compare]
}

//! [member]
struct Customer
{
    std::string                name;
    std::optional<std::string> company;  // a private customer has none
};
//! [member]

void members()
{
    //! [member-use]
    Customer customer{.name = "Ada", .company = std::nullopt};

    customer.company = "Analytical Engines Ltd";    // now it holds a string
    std::cout << customer.company->size() << '\n';  // -> reaches into the value, like a pointer

    customer.company.reset();  // nothing again
    std::cout << customer.company.value_or("(private)") << '\n';
    //! [member-use]
}

//! [parse]
std::optional<int> parse_int(std::string_view text)
{
    int               value = 0;
    const char* const first = text.data();
    // from_chars takes two pointers: where the characters start and where they end.
    const char* const last        = first + text.size();  // NOLINT(*-pointer-arithmetic)
    const auto [stoppedAt, error] = std::from_chars(first, last, value);
    if (error != std::errc{} || stoppedAt != last)
    {
        return std::nullopt;
    }
    return value;
}
//! [parse]

std::optional<std::string_view> setting(std::string_view key)
{
    if (key == "retries")
    {
        return "3";
    }
    if (key == "timeout")
    {
        return "soon";
    }
    return std::nullopt;
}

void chaining()
{
    //! [chain-by-hand]
    int retries = 1;
    if (const auto text = setting("retries"))
    {
        if (const auto number = parse_int(*text))
        {
            retries = *number * 2;
        }
    }
    //! [chain-by-hand]
    std::cout << retries << '\n';

    //! [chain]
    const auto doubled = [](int n) { return n * 2; };

    std::cout << setting("retries").and_then(parse_int).transform(doubled).value_or(1)
              << '\n';  // 6
    std::cout << setting("timeout").and_then(parse_int).transform(doubled).value_or(1)
              << '\n';  // 1: not a number
    std::cout << setting("colour").and_then(parse_int).transform(doubled).value_or(1)
              << '\n';  // 1: no such key
    //! [chain]
}

//! [expected]
enum class ReadError : std::uint8_t
{
    Missing,
    NotANumber,
};

std::expected<int, ReadError> read_setting(std::string_view key)
{
    const std::optional<std::string_view> text = setting(key);
    if (!text)
    {
        return std::unexpected(ReadError::Missing);
    }
    const std::optional<int> number = parse_int(*text);
    if (!number)
    {
        return std::unexpected(ReadError::NotANumber);
    }
    return *number;
}
//! [expected]

void expecting()
{
    //! [expected-use]
    for (const std::string_view key : {"retries", "timeout", "colour"})
    {
        if (const std::expected<int, ReadError> value = read_setting(key))
        {
            std::cout << key << " = " << *value << '\n';
        }
        else
        {
            std::cout << key << ": "
                      << (value.error() == ReadError::Missing ? "not set" : "not a number") << '\n';
        }
    }
    //! [expected-use]
}

}  // namespace

int main()
{
    using_one();
    members();
    chaining();
    expecting();
    return 0;
}
