# Chapter 3: reference solutions

Compiled against the chapter's tests: `ctest --preset solutions -R sol03`.

## Drills

**Drill 1, `d01_sort_by_age.h`.** `std::ranges::stable_sort(people, {}, &Person::age);`. With
`std::ranges::sort` the first test passes and the second fails, under both compilers: `sort` makes no
promise about equal elements, and on three hundred elements it does reorder them.

**Drill 2, `d02_oldest.h`.** `max_element` returns an iterator; `&*found` turns it into a pointer to the
element, after the check for an empty range (where the iterator is `end()` and must not be dereferenced).
Of several equally old people `max_element` returns the first.

**Drill 3, `d03_name_of.h`.** `std::ranges::find(people, id, &Person::id)`: no lambda, because "equal to
this value" is what `find` does and "the id" is a projection. `return found->name;` converts the
`std::string` in the caller's storage to a view of it, wrapped in an optional.

**Drill 4, `d04_iequals.h`.** One call. Everything a hand-written loop would have to get right (stop at the
shorter text, then compare the lengths) is inside `std::ranges::equal`.

**Drill 5, `d05_drop_blank.h`.** `std::erase_if(lines, isBlank)`. The two-step form is in a comment. The
predicate takes a `std::string_view`, so it accepts the `std::string` elements without copying them.

**Drill 6, `d06_first_out_of_order.h`.** `std::ranges::is_sorted_until` returns the first element that
breaks the order, or `end()`. An iterator minus `begin()` is the index; it is a signed difference, hence
the cast.

**Drill 7, `d07_top_n.h`.** The projection returns `std::pair<int, std::string_view>(-score.points,
score.name)`. Pairs compare their first members and, if those are equal, their second, which is exactly the
rule; the minus sign turns ascending into descending for the points only. The name is projected as a view:
a pair of `int` and `std::string` would copy every name on every comparison.

The same with a comparator and no projection:

<!-- not-compiled: an alternative to the solution in d07_top_n.h, for comparison -->
```cpp
std::ranges::sort(scores, [](const Score& a, const Score& b) {
    return a.points != b.points ? a.points > b.points : a.name < b.name;
});
```

Longer, and it says more plainly which key is descending. Negating a key only works for numbers, and not
for the smallest `int`. When that matters, or when one key is a text to be sorted descending, write the
comparator.

## Problem 1: a ranking

**Sort once, then every question is a search.**

<!-- file: solutions/03-ranges-algorithms/p01_ranking.h#sort -->
```cpp
explicit Ranking(std::vector<Result> results) : table(std::move(results))
{
    // The whole ranking rule is this one key. Tuples compare member by member, so the first
    // difference decides; negated points put the larger number first.
    std::ranges::sort(table, {}, [](const Result& result) {
        return std::tuple<int, int, std::string_view>(-result.points, result.finishSeconds,
                                                      result.name);
    });
}
```

The ranking rule is the tuple, nothing else. A fourth tie-break would be a fourth member.

<!-- file: solutions/03-ranges-algorithms/p01_ranking.h#rank -->
```cpp
/// The rank of @p name, 1 for the winner, or nothing when @p name did not take part.
[[nodiscard]] std::optional<std::size_t> RankOf(std::string_view name) const
{
    const auto found = std::ranges::find(table, name, &Result::name);
    if (found == table.end())
    {
        return std::nullopt;
    }
    return static_cast<std::size_t>(found - table.begin()) + 1;
}
```

A linear `find` with a projection. The table is sorted by rank, not by name, so there is nothing faster to
use without a second data structure. For the sizes a match has, that is right. If rank lookups dominated, a
`std::map` from name to rank, built in the constructor, would be the next step. The return type is chapter
2's: a rank, or nothing.

<!-- file: solutions/03-ranges-algorithms/p01_ranking.h#within -->
```cpp
/// The results that are at most @p points behind the leader, best first. A part of Table().
[[nodiscard]] std::span<const Result> WithinOfLeader(int points) const
{
    if (table.empty())
    {
        return {};
    }
    // The table is sorted by points, descending, so "close enough to the leader" is true for a
    // run of elements at the front and false for the rest. partition_point finds where the run
    // ends, by binary search.
    const int  threshold = table.front().points - points;
    const auto end       = std::ranges::partition_point(
        table, [threshold](int candidate) { return candidate >= threshold; }, &Result::points);
    return {table.begin(), end};
}
```

Because the table is sorted by points, "at most `n` behind the leader" is true for some elements at the
front and false for all the rest. `partition_point` finds the boundary by binary search; the predicate
receives the projected points. The result is a `std::span` into the table: chapter 1's "a part of what I
already have", with no copy. A `std::vector<Result>` built with `copy_if` is a correct answer that copies.

## Problem 2: what changed in the warehouse

**The time limit is the design constraint.** For every item of one inventory, searching the other with
`find` is 100 000 times 100 000 comparisons. Sorting both first costs two sorts and then one pass.

<!-- file: solutions/03-ranges-algorithms/p02_inventory_diff.h#diff -->
```cpp
// Sorted by SKU, the two inventories can be compared in one pass each.
std::ranges::sort(before, {}, &Item::sku);
std::ranges::sort(after, {}, &Item::sku);

InventoryDiff result;
// "The elements of the first range that are not in the second", compared by SKU on both sides.
// The output is a sequence of Items; the transform below keeps only the SKU of each.
std::vector<Item> onlyAfter;
std::vector<Item> onlyBefore;
std::ranges::set_difference(after, before, std::back_inserter(onlyAfter), {}, &Item::sku,
                            &Item::sku);
std::ranges::set_difference(before, after, std::back_inserter(onlyBefore), {}, &Item::sku,
                            &Item::sku);
std::ranges::transform(onlyAfter, std::back_inserter(result.added), &Item::sku);
std::ranges::transform(onlyBefore, std::back_inserter(result.removed), &Item::sku);
```

`set_difference` is "what is in the first sorted range and not in the second", and it takes a projection
for each range, so it compares by SKU although the elements are `Item`s. It writes `Item`s; `transform`
with the member pointer as its function keeps only the SKUs. Because the inputs are sorted, so are the
outputs: the requirement that each list be sorted costs nothing.

<!-- file: solutions/03-ranges-algorithms/p02_inventory_diff.h#changed -->
```cpp
// For "in both, but different" no algorithm fits exactly: walk the two sorted sequences
// together.
auto left  = before.begin();
auto right = after.begin();
while (left != before.end() && right != after.end())
{
    if (left->sku < right->sku)
    {
        ++left;
    }
    else if (right->sku < left->sku)
    {
        ++right;
    }
    else
    {
        if (left->quantity != right->quantity)
        {
            result.changed.push_back(left->sku);
        }
        ++left;
        ++right;
    }
}
```

No standard algorithm says "in both, and different in another member". `set_intersection` would give the
items that are in both, taken from the first range, and then their quantities would have to be looked up
again. The merge-style walk is the loop the set algorithms are built from, written out once. Knowing when
no algorithm fits is part of knowing the algorithms.

**The other good answer** is a `std::map<std::string, int>` or `std::unordered_map` of `before`, and one
pass over `after`. It is about as fast, arguably simpler, and the added and changed lists come out in
whatever order `after` had, so they need a sort at the end. If the inventories were already kept sorted,
the reference's approach would need no sort at all.

## Problem 3: abbreviated commands

<!-- file: solutions/03-ranges-algorithms/p03_resolve_command.h#resolve -->
```cpp
const auto isExactly = [typed](std::string_view command) {
    return std::ranges::equal(command, typed, {}, detail::lower, detail::lower);
};
const auto startsWith = [typed](std::string_view command) {
    return std::ranges::starts_with(command, typed, {}, detail::lower, detail::lower);
};

// An exact match wins, even when it is also the beginning of a longer command.
if (const auto exact = std::ranges::find_if(commands, isExactly); exact != commands.end())
{
    return *exact;
}
switch (std::ranges::count_if(commands, startsWith))
{
    case 0:
        return std::unexpected(ResolveError::Unknown);
    case 1:
        return *std::ranges::find_if(commands, startsWith);
    default:
        return std::unexpected(ResolveError::Ambiguous);
}
```

- **The two predicates are the logger's line, twice.** `equal` with two projections is "the same, ignoring
  case"; `starts_with` (C++23) with the same projections is "begins with, ignoring case".
- **Exact match first.** The rule that a name typed in full always wins has to be checked before counting
  prefixes: `co` is a prefix of three commands and equal to one.
- **`count_if`, then `find_if`.** Counting answers unknown, unique or ambiguous; when the count is 1 the
  second search finds that one. Two passes over a handful of commands is not worth optimising.
- **The result** is `std::expected<std::string_view, ResolveError>`, as in chapter 2's problem 3: two
  different failures, and the caller words the message. The view is one of the elements of `commands`,
  which is how "spelt as it is in the list" comes for nothing.
- **The parameter** is a `std::span<const std::string_view>`: a `std::array` of names converts to it.

## Problem 4: `parse_level`

<!-- file: solutions/03-ranges-algorithms/p04_parse_level.h#parse-level -->
```cpp
/// The level whose name is @p text, ignoring the case of ASCII letters, or nothing.
[[nodiscard]] constexpr std::optional<Level> parse_level(std::string_view text) noexcept
{
    constexpr std::array kLevels{
        Level::Off, Level::Error, Level::Warn, Level::Info, Level::Debug, Level::Perf,
    };
    for (const Level level : kLevels)
    {
        if (std::ranges::equal(text, level_name(level), {}, detail::to_lower_ascii,
                               detail::to_lower_ascii))
        {
            return level;
        }
    }
    return std::nullopt;
}
```

This is the logger's function, and yours is probably close to it. Common differences and what to make of
them:

- **A chain of `if (iequals(text, "off")) return Level::Off;`** is correct, and repeats each name in two
  places: here and in `level_name`. The loop over the levels asks `level_name` instead, so a new level
  cannot be forgotten in one of them.
- **Lower-casing the input into a `std::string` first** is correct and allocates. The projections do the
  same without a copy, which also lets the function be `noexcept`.
- **`std::tolower`** compiles and is wrong for the reasons in question 3 below.
- **`constexpr`.** The reference marks it `constexpr`; the logger's is not, because it is defined in a
  `.cpp` file, where `constexpr` would gain nothing. Chapter 6 comes back to this.

## Back in logger

**1.** `text` and `level_name(level)` are the two ranges. `{}` is the comparison, defaulted to
`std::ranges::equal_to`, that is `==`. The first `to_lower_ascii` is the projection applied to the
characters of `text`, the second to those of the level's name. Without the last argument only the input
would be lower-cased, and it would be compared with names that are in upper case: `"warn"` against
`"WARN"` is false, and so is every other input, because every name contains a letter. The function would
return nothing for everything. `"war"` is rejected because the range form of `equal` compares lengths
first: three characters cannot equal four. The three-iterator form would compare three characters of `"war"`
with the first three of `"WARN"`, find them equal, and return `Level::Warn`. With `"warning"` it would
compare seven characters against a name that has four, reading three characters past its end.

**2.** The algorithm calls `std::invoke(&logger::Record::timestamp, record)` for each element, and for a
pointer to a data member `std::invoke` means `record.timestamp`. Without a projection the member access
moves into a comparator:

<!-- not-compiled: a rewrite of one line of the logger's tests -->
```cpp
std::ranges::is_sorted(records, [](const logger::Record& a, const logger::Record& b) {
    return a.timestamp < b.timestamp;
});
```

**3.** The guide's two: `std::tolower` depends on the C locale, so the same program could accept different
texts on different machines; and its parameter is an `int` that must hold an `unsigned char` value, so
passing a negative `char` is undefined behaviour. The third: it could not be passed as a projection as it
stands. `std::tolower` is overloaded (there is a second one in `<locale>`), and the standard does not allow
taking the address of most standard-library functions anyway, so it would have to be wrapped in a lambda
that also does the cast. And a fourth for this tutorial: it is not `constexpr`. Six lines of arithmetic
avoid all four.
