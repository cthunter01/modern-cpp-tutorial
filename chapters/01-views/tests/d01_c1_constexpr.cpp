#include "d01_trim.h"

// trim() is constexpr: it can be asked at compile time, and then a wrong answer is a build error.
static_assert(ch01::trim("  hello  ") == "hello");
static_assert(ch01::trim("hello") == "hello");
static_assert(ch01::trim(" \t\r\n").empty());
static_assert(ch01::trim("").empty());
static_assert(noexcept(ch01::trim("x")));
