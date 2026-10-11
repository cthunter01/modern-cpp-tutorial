#include "d04_is_one_of.h"

static_assert(ch05::is_one_of(3, 1, 2, 3));
static_assert(!ch05::is_one_of(4, 1, 2, 3));
static_assert(!ch05::is_one_of(4));
static_assert(ch05::is_one_of('b', 'a', 'b'));
