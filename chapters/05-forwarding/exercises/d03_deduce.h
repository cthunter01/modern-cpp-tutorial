#pragma once

#include <array>
#include <string_view>

namespace ch05
{

// Part 1. Given
//
//   template <typename T> void f(T&& x);
//   int               number = 1;
//   const int         fixed  = 2;
//   std::string       text   = "t";
//
// what is T in each call? Replace each "?" by the type, written exactly as it would appear in code:
// "int", "int&", "const int&", "std::string" or "std::string&". Decide before you run the tests.
inline constexpr std::array<std::string_view, 5> kDeduced{
    "?",  // 1  f(42)
    "?",  // 2  f(number)
    "?",  // 3  f(fixed)
    "?",  // 4  f(std::move(number))
    "?",  // 5  f(text)
};

// Part 2. Is x a forwarding reference? Replace each "?" by "yes" or "no".
inline constexpr std::array<std::string_view, 6> kForwarding{
    "?",  // 1  template <typename T> void f(T&& x);
    "?",  // 2  template <typename T> void f(const T&& x);
    "?",  // 3  template <typename T> void f(std::vector<T>&& x);
    "?",  // 4  template <typename T> struct Box { void Push(T&& x); };
    "?",  // 5  template <typename T> struct Box { template <typename U> void Push(U&& x); };
    "?",  // 6  void f(auto&& x);
};

}  // namespace ch05
