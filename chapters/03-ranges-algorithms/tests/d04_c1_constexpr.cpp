#include "d04_iequals.h"

// The comparison is constexpr, like the logger's file_basename and level_name: these are tests that
// the compiler runs.
static_assert(ch03::iequals("warn", "WARN"));
static_assert(ch03::iequals("", ""));
static_assert(!ch03::iequals("war", "warn"));
static_assert(!ch03::iequals("warning", "warn"));
static_assert(!ch03::iequals("warn", "wart"));
