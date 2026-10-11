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
// what is T in each call? Write the type exactly as it would appear in code.
inline constexpr std::array<std::string_view, 5> kDeduced{
    "int",         // 1  f(42)                  an rvalue: T is the plain type, and x is int&&
    "int&",        // 2  f(number)              an lvalue: T is a reference, and x is int& && = int&
    "const int&",  // 3  f(fixed)               const comes along
    "int",         // 4  f(std::move(number))   an rvalue again
    "std::string&",  // 5  f(text)
};

// Part 2. Is x a forwarding reference? "yes" or "no".
inline constexpr std::array<std::string_view, 6> kForwarding{
    "yes",  // 1  template <typename T> void f(T&& x);
    "no",   // 2  template <typename T> void f(const T&& x);           const: an rvalue reference
    "no",   // 3  template <typename T> void f(std::vector<T>&& x);    not T&& itself: an rvalue
            // reference
    "no",  // 4  template <typename T> struct Box { void Push(T&& x); };   T is fixed when Box<T> is
           // named
    "yes",  // 5  template <typename T> struct Box { template <typename U> void Push(U&& x); };
    "yes",  // 6  void f(auto&& x);                                    a template in disguise
};

}  // namespace ch05
