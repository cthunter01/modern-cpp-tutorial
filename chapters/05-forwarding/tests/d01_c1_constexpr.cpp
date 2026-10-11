#include "d01_count_and_sum.h"

// Both functions are constexpr, and the size of a pack is known to the compiler anyway.
static_assert(ch05::count_args() == 0);
static_assert(ch05::count_args('a', 2, 3.0) == 3);
static_assert(ch05::sum_all() == 0);
static_assert(ch05::sum_all(1, 2, 3) == 6);
