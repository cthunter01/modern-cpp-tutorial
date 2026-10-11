#include <vector>

#include "d01_min_max.h"

// Valid: the result is used.
int spread(const std::vector<int>& values)
{
    const auto [low, high] = ch00::min_max(values);
    return high - low;
}

#ifdef TUT_BREAK
// Must not compile: a call that throws the answer away is a mistake, and min_max should say so.
void forgets_the_answer(const std::vector<int>& values)
{
    ch00::min_max(values);
}
#endif
