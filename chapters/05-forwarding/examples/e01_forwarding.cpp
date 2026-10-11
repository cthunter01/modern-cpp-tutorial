// The listings of chapter 5: parameter packs, folds, forwarding references and std::forward.

#include <concepts>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace
{

//! [pack]
// `typename... Ts` is a template parameter pack: zero or more types. `const Ts&... values` is a
// function parameter pack: one parameter per type.
template <typename... Ts>
std::size_t how_many(const Ts&... /*values*/)
{
    return sizeof...(Ts);  // the number of elements: the only thing you can ask a pack directly
}
//! [pack]

std::size_t length_of(std::string_view text)
{
    return text.size();
}

std::size_t total(std::size_t a, std::size_t b, std::size_t c)
{
    return a + b + c;
}

//! [expansion]
// A pack is used by expanding it: a pattern followed by `...` is repeated once for each element,
// separated by commas.
template <typename... Ts>
std::size_t total_length(const Ts&... texts)
{
    return total(length_of(texts)...);  // total(length_of(t1), length_of(t2), length_of(t3))
}
//! [expansion]

//! [fold]
// A fold expression puts an operator between the elements instead of a comma.
template <typename... Ts>
bool all_positive(const Ts&... values)
{
    return ((values > 0) && ...);  // (v1 > 0) && (v2 > 0) && ...     true for an empty pack
}

template <typename... Ts>
void print_all(const Ts&... values)
{
    ((std::cout << values << ' '), ...);  // the comma operator: do this for each, in order
    std::cout << '\n';
}
//! [fold]

//! [receive]
std::string_view receive(const std::string& /*text*/)
{
    return "as an lvalue";
}

std::string_view receive(
    std::string&& /*text*/)  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
{
    return "as an rvalue";
}
//! [receive]

//! [relay]
// T&& with T deduced right here is a forwarding reference: it accepts lvalues and rvalues alike.
template <typename T>
std::string_view relay_plainly(T&& text)  // NOLINT(cppcoreguidelines-missing-std-forward)
{
    return receive(text);  // `text` is a name, so receive() always sees an lvalue
}

template <typename T>
std::string_view relay(T&& text)
{
    return receive(
        std::forward<T>(text));  // an lvalue stays an lvalue, an rvalue becomes one again
}
//! [relay]

void relaying()
{
    //! [relay-calls]
    const std::string name = "Ada";

    std::cout << relay_plainly(name) << ", " << relay_plainly(std::string("Grace")) << '\n';
    std::cout << relay(name) << ", " << relay(std::string("Grace")) << '\n';
    //! [relay-calls]
}

//! [make]
// The pattern behind make_unique, make_shared, emplace_back and the logger's add_sink: accept any
// arguments, and hand each one on to a constructor exactly as it came.
template <typename T, typename... Args>
std::unique_ptr<T> make_boxed(Args&&... args)
{
    return std::unique_ptr<T>(
        new T(std::forward<Args>(args)...));  // NOLINT(cppcoreguidelines-owning-memory)
}
//! [make]

//! [class-pack]
// A class template can have a pack too. This is all a typed table row needs.
template <typename... Columns>
struct Row
{
    std::tuple<Columns...> cells;
};
//! [class-pack]

//! [requires]
// A requires clause states what a template is for. std::integral is a concept from <concepts>: a
// compile-time yes-or-no question about a type.
template <typename T>
    requires std::integral<T>
T half(T value)
{
    return value / 2;
}

// The same, shorter: the concept in place of `typename`.
template <std::floating_point T>
T half(T value)
{
    return value / T{2};
}
//! [requires]

//! [type-identity]
// Deduction looks at every parameter that mentions T, and they have to agree: scale(2.5, 2) would
// deduce double from the first argument and int from the second, and fail. type_identity_t<T> is T,
// spelt so that the compiler does not try to deduce from it.
template <typename T>
T scale(T value, std::type_identity_t<T> factor)
{
    return value * factor;
}
//! [type-identity]

}  // namespace

int main()
{
    std::cout << std::boolalpha;
    std::cout << how_many() << ' ' << how_many(1, "two", 3.0) << '\n';
    std::cout << total_length("a", "bc", "def") << '\n';
    std::cout << all_positive(1, 2.5, 3U) << ' ' << all_positive(1, -2) << ' ' << all_positive()
              << '\n';
    print_all(1, "two", 3.5);
    relaying();
    const auto boxed = make_boxed<std::pair<int, std::string>>(1, "one");
    std::cout << boxed->second << '\n';
    const Row<int, std::string> row{.cells = {1, "one"}};
    std::cout << std::get<1>(row.cells) << '\n';
    std::cout << half(7) << ' ' << half(7.0) << '\n';
    std::cout << scale(2.5, 2) << '\n';
    return 0;
}
