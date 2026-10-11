#include "d03_deduce.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

// Whether a variable is const decides what is deduced from it and whether it can be moved from, so
// the variables in this file are declared the way each test needs them.
//
// NOLINTBEGIN(misc-const-correctness)

// Each test asks the compiler the question you answered in the table, and compares.

namespace
{

/// The type T, spelt as in code, for the handful of types this drill uses.
template <typename T>
constexpr std::string_view name_of() noexcept
{
    if constexpr (std::is_same_v<T, int>)
    {
        return "int";
    }
    else if constexpr (std::is_same_v<T, int&>)
    {
        return "int&";
    }
    else if constexpr (std::is_same_v<T, const int&>)
    {
        return "const int&";
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        return "std::string";
    }
    else if constexpr (std::is_same_v<T, std::string&>)
    {
        return "std::string&";
    }
    else
    {
        return "something else";
    }
}

/// What T was deduced as in a call f(x) of `template <typename T> void f(T&& x)`.
template <typename T>
constexpr std::string_view deduced(
    T&& /*unused*/) noexcept  // NOLINT(cppcoreguidelines-missing-std-forward)
{
    return name_of<T>();
}

void check_deduced(std::size_t number, std::string_view actual)
{
    EXPECT_EQ(ch05::kDeduced.at(number - 1), actual)
        << "your prediction for call " << number << " is not what the compiler deduces";
}

TEST(Deduce, Call1ALiteral)
{
    check_deduced(1, deduced(42));
}

TEST(Deduce, Call2AVariable)
{
    int number = 1;
    check_deduced(2, deduced(number));
}

TEST(Deduce, Call3AConstVariable)
{
    const int fixed = 2;
    check_deduced(3, deduced(fixed));
}

TEST(Deduce, Call4AMovedVariable)
{
    int number = 1;
    check_deduced(4,
                  deduced(std::move(number)));  // NOLINT(performance-move-const-arg): the question
}

TEST(Deduce, Call5AStringVariable)
{
    std::string text = "t";
    check_deduced(5, deduced(text));
}

// A forwarding reference accepts lvalues and rvalues. An rvalue reference refuses lvalues. So "is x
// a forwarding reference?" can be put to the compiler as "does a call with a named variable
// compile?".

// The six declarations of the table. NOLINTBEGIN(cppcoreguidelines-missing-std-forward)
// NOLINTBEGIN(cppcoreguidelines-rvalue-reference-param-not-moved)
template <typename T>
void form1(T&& /*x*/)
{
}

template <typename T>
void form2(const T&& /*x*/)
{
}

template <typename T>
void form3(std::vector<T>&& /*x*/)
{
}

template <typename T>
struct Box4
{
    void Push(T&& /*x*/) { }
};

template <typename T>
struct Box5
{
    template <typename U>
    void Push(U&& /*x*/)
    {
    }
};

void form6(auto&& /*x*/) { }
// NOLINTEND(cppcoreguidelines-rvalue-reference-param-not-moved)
// NOLINTEND(cppcoreguidelines-missing-std-forward)

template <typename Arg>
concept Form1TakesAName = requires(Arg& named) { form1(named); };
template <typename Arg>
concept Form2TakesAName = requires(Arg& named) { form2(named); };
template <typename Arg>
concept Form3TakesAName = requires(std::vector<Arg>& named) { form3(named); };
template <typename Arg>
concept Form4TakesAName = requires(Box4<Arg>& box, Arg& named) { box.Push(named); };
template <typename Arg>
concept Form5TakesAName = requires(Box5<Arg>& box, Arg& named) { box.Push(named); };
template <typename Arg>
concept Form6TakesAName = requires(Arg& named) { form6(named); };

void check_forwarding(std::size_t number, bool takesAName)
{
    EXPECT_EQ(ch05::kForwarding.at(number - 1), takesAName ? "yes" : "no")
        << "your prediction for declaration " << number << " is not what the compiler does";
}

TEST(Deduce, Declaration1PlainTemplateParameter)
{
    check_forwarding(1, Form1TakesAName<int>);
}

TEST(Deduce, Declaration2ConstQualified)
{
    check_forwarding(2, Form2TakesAName<int>);
}

TEST(Deduce, Declaration3VectorOfT)
{
    check_forwarding(3, Form3TakesAName<int>);
}

TEST(Deduce, Declaration4MemberOfAClassTemplate)
{
    check_forwarding(4, Form4TakesAName<int>);
}

TEST(Deduce, Declaration5MemberTemplate)
{
    check_forwarding(5, Form5TakesAName<int>);
}

TEST(Deduce, Declaration6AbbreviatedTemplate)
{
    check_forwarding(6, Form6TakesAName<int>);
}

}  // namespace

// NOLINTEND(misc-const-correctness)
