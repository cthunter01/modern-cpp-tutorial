# Chapter 0: reference solutions

Each file here has the name of the exercise file it answers, and is compiled against the same tests:
`ctest --preset solutions -R sol00`. A reference is one good answer, not the only one. Where yours differs,
the question to ask is whether the difference is one you chose.

## Drills

**Drill 1, `d01_min_max.h`.** The body is short; the point is the word in front of the function.
`[[nodiscard]]` is what makes `tests/d01_r1_discarded.cpp` fail to compile once `TUT_BREAK` is defined.
The reference calls `std::ranges::minmax_element(values)`, the C++20 form that takes the container itself.
`std::minmax_element(values.begin(), values.end())` is equally correct; chapter 3 is about the difference.

**Drill 2, `d02_first_repeat.h`.** `seen.insert(value)` both looks the value up and stores it, so the loop
does one search per value, not two. A version that calls `seen.contains(value)` and then `seen.insert(value)`
is correct and searches twice.

**Drill 3, `d03_make_request.h`.** The ordinary case names one member. The upload case names all four, one
per line with a trailing comma: clang-tidy's `readability-trailing-comma` asks for that as soon as an
initialiser list spans several lines, and clang-format then keeps one member per line.

**Drill 4, `d04_toggle.h`.** `return std::exchange(on, false);` and a `noexcept` on the signature.

## Problem 1: the most frequent word

`p01_most_frequent.h` returns a small struct:

<!-- file: solutions/00-warm-up/p01_most_frequent.h#result -->
```cpp
struct WordCount
{
    std::string word;
    std::size_t count = 0;
};
```

**Why a struct and not `std::pair<std::string, std::size_t>`.** Both can be taken apart by the structured
binding in the tests, so the tests cannot tell them apart. A caller who does not take the result apart can:
`result.count` says what it is, `result.second` does not. A pair is fine for something used on the next line
and forgotten; a function's return type is read far from where it was written. `std::tuple` has the same
weakness as the pair, with `std::get<1>(result)` in place of `.second`.

**Why `std::map`.** It keeps the words sorted, and that gives the tie rule for nothing: of two words with
the same count the alphabetically first is met first, and the comparison `count > best.count` keeps it. With
`std::unordered_map` the counting is faster on large inputs and the tie rule must be written out
(`count > best.count || (count == best.count && word < best.word)`). Either is a good answer if the tie rule
is right; a solution that passes only because of the order its container happens to iterate in is not.

**What the reference copies.** `best = {.word = word, .count = count}` copies the winning word each time the
lead changes, and once more is unavoidable because the function returns an owning `std::string`. Returning a
`std::string_view` into the caller's vector would avoid every copy and would be a fine design too, with a
condition attached: the result is then only valid while that vector is alive and unchanged. That trade is the
subject of chapter 1.

**The empty list.** The statement rules it out. The reference returns an empty word with count 0 for it,
which is a sentinel like drill 2's -1. Chapter 2 has the honest return type.
