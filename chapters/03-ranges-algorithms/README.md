# Chapter 3: Ranges I: algorithms and projections

`parse_level` contains one line that the guide passes over in a sentence:
`std::ranges::equal(text, level_name(level), {}, to_lower_ascii, to_lower_ascii)`. Five arguments, one of
them an empty pair of braces. This chapter takes that call apart. Its subject is the C++20 versions of the
standard algorithms, which take a whole range where the old ones took two iterators, and which accept one
more kind of argument, the projection, that removes most of the lambdas the old ones needed.

Views, the lazy half of `<ranges>`, are chapter 12. Algorithms come first because they are the half you can
use everywhere, today, without learning a new way of thinking.

## Goal

- You call `std::ranges::sort`, `find`, `count_if`, `equal`, `max_element` and their relatives with a
  comparator or a projection, and know which slot is which.
- You know what each of them returns and what it refers to.
- You can write `parse_level` from an empty file.

## Where you met it

Step 1 of the guide. This is the whole function, and by the end of the chapter nothing in it is new:

<!-- logger: src/Level.cpp:21-34 -->
```cpp
std::optional<Level> parse_level(std::string_view text) noexcept
{
    constexpr std::array kLevels{
        Level::Off, Level::Error, Level::Warn, Level::Info, Level::Debug, Level::Perf,
    };
    for (const Level level : kLevels)
    {
        if (std::ranges::equal(text, level_name(level), {}, to_lower_ascii, to_lower_ascii))
        {
            return level;
        }
    }
    return std::nullopt;
}
```

The logger has exactly two more range calls. The demo sorts a vector (step 14):

<!-- logger: src/main.cpp:114 -->
```cpp
std::ranges::sort(values);
```

and one test checks that records come out in time order (step 8):

<!-- logger: tests/logger_tests.cpp:683 -->
```cpp
EXPECT_TRUE(std::ranges::is_sorted(records, {}, &logger::Record::timestamp));
```

Three calls, and between them every idea of this chapter: a whole range, a `{}`, a pointer to a member used
as a projection, and two ranges with a projection each.

## The idea

### Pass the range

<!-- file: chapters/03-ranges-algorithms/examples/e01_ranges.cpp#whole-range -->
```cpp
std::vector<int> values{5, 3, 8, 1};

std::sort(values.begin(), values.end());  // C++98: two iterators   NOLINT(modernize-use-ranges)
std::ranges::sort(values);                // C++20: the range itself
```

Every algorithm in `<algorithm>` has a second version in the namespace `std::ranges`. The first thing it
gives you is that a range is one argument. "Range" means anything with a beginning and an end: a container,
a built-in array, a `std::string_view`, a `std::span`. That alone removes the mistake of passing the begin
of one container with the end of another.

### Say what to look at: the projection

Sorting people by age with the old form needs a comparator that takes two people, digs the age out of each,
and compares:

<!-- file: chapters/03-ranges-algorithms/examples/e01_ranges.cpp#sort-lambda -->
```cpp
// Without projections: a comparator that digs the member out, twice.
std::ranges::sort(list, [](const Person& a, const Person& b) { return a.age < b.age; });
```

Two separate things are tangled in that lambda: *what* to compare (the age) and *how* (with `<`). The range
algorithms take them as two arguments:

<!-- file: chapters/03-ranges-algorithms/examples/e01_ranges.cpp#sort-projection -->
```cpp
std::ranges::sort(list, {}, &Person::age);                      // by age, ascending
std::ranges::sort(list, std::ranges::greater{}, &Person::age);  // by age, descending
std::ranges::sort(list, {}, &Person::name);                     // by name
```

The second argument is the **comparator**: how two keys are compared. The third is the **projection**: how
a key is obtained from an element. `&Person::age` is a pointer to a data member, and as a projection it
means "for each element, look at its `age`".

`{}` in the comparator's place is not special syntax. It is an empty initialiser, which default-constructs
the parameter, and the comparator's type defaults to `std::ranges::less`. So `{}` reads as "the usual
comparison". You write it because arguments are positional: to reach the third slot you have to say
something in the second.

A projection may be any callable, including one that computes:

<!-- file: chapters/03-ranges-algorithms/examples/e01_ranges.cpp#sort-key -->
```cpp
// A projection can be any callable. This one computes a sort key: oldest first, then by name.
std::ranges::sort(list, {},
                  [](const Person& p) { return std::pair(-p.age, std::string_view(p.name)); });
```

Pairs and tuples compare member by member, so returning one from a projection sorts by several keys at once.

### How a projection is called: `std::invoke`

An algorithm does not call a projection as `projection(element)`. It calls
`std::invoke(projection, element)`, and `std::invoke` (C++17) knows what to do with things that are not
functions:

<!-- file: chapters/03-ranges-algorithms/examples/e01_ranges.cpp#invoke -->
```cpp
// What an algorithm does with a projection: std::invoke. For a pointer to a data member that reads
// the member, for a pointer to a member function it calls it, for anything else it is an ordinary
// call.
void invoking()
{
    const Person ada{.name = "Ada", .age = 36, .id = 7};

    std::cout << std::invoke(&Person::age, ada) << ' '             // ada.age
              << std::invoke(&std::string::size, ada.name) << ' '  // ada.name.size()
              << std::invoke([](const Person& p) { return p.id; }, ada)
              << '\n';  // the lambda, called
}
```

That is the whole reason `&Person::age` and `&logger::Record::timestamp` work as projections.

### Predicates take projections too

<!-- file: chapters/03-ranges-algorithms/examples/e01_ranges.cpp#find -->
```cpp
// "The element whose id is 7": a value and a projection.
const auto ada = std::ranges::find(list, 7, &Person::id);
if (ada != list.end())
{
    std::cout << ada->name << " has id 7\n";
}

// "The first element for which this is true": a predicate. It may have a projection too.
const auto senior = std::ranges::find_if(list, [](int age) { return age > 60; }, &Person::age);
std::cout << senior->name << " is the first over 60\n";
```

<!-- file: chapters/03-ranges-algorithms/examples/e01_ranges.cpp#questions -->
```cpp
const auto isOver60 = [](int age) { return age > 60; };

std::cout << std::ranges::count_if(list, isOver60, &Person::age) << ' '    // 2
          << std::ranges::any_of(list, isOver60, &Person::age) << ' '      // 1 (true)
          << std::ranges::all_of(list, isOver60, &Person::age) << ' '      // 0 (false)
          << std::ranges::contains(list, "Linus", &Person::name) << '\n';  // 1          (C++23)
```

`find` looks for a value, `find_if` for the first element satisfying a predicate; with a projection both
are applied to the projected key. `contains` is C++23.

### Two ranges, two projections

Algorithms that work on two ranges take a projection for each. Now the logger's line can be read from left
to right:

<!-- file: chapters/03-ranges-algorithms/examples/e01_ranges.cpp#iequals -->
```cpp
constexpr char to_lower_ascii(char c) noexcept
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

constexpr bool same_ignoring_case(std::string_view a, std::string_view b) noexcept
{
    //                        range 1, range 2, comparison, projection for 1, projection for 2
    return std::ranges::equal(a, b, {}, to_lower_ascii, to_lower_ascii);
}
```

Both texts are lower-cased character by character, while they are being compared, without a copy of
either. In `parse_level` the first range is what the user typed and the second is a level's name.

## The details that bite

### Which slot is which

Every range algorithm has the same order: the range or ranges, then whatever the algorithm needs (a value,
an output, a count), then the comparator or predicate, then the projection or projections. The slot most
often confused is the last pair: `std::ranges::sort(people, &Person::age)` puts a projection where the
comparator belongs. It does not compile, and the error message is two screens long. The fourth experiment
shows how to read it.

### A projection changes what is compared, never what is returned

<!-- file: chapters/03-ranges-algorithms/examples/e01_ranges.cpp#max -->
```cpp
// The projection decides what is compared. What comes back is the element, not the age.
const auto oldest =
    std::ranges::max_element(list, {}, &Person::age);              // an iterator into `list`
const Person youngest = std::ranges::min(list, {}, &Person::age);  // a copy of the element
std::cout << oldest->name << ' ' << youngest.name << '\n';
```

`max_element` compares ages and returns an iterator to a `Person`. `find(list, 7, &Person::id)` compares
ids and returns an iterator to a `Person`. The projection is used inside the algorithm and thrown away. If
you want the ages themselves, that is a transformation, and belongs to chapter 12.

Note also the two families: `max_element` and `min_element` return an iterator into the range, `max` and
`min` return a copy of the element. For a range of large objects the difference matters.

### `remove` does not remove

<!-- file: chapters/03-ranges-algorithms/examples/e01_ranges.cpp#remove -->
```cpp
std::vector<int> values{4, 0, 8, 0, 0, 15};

// remove does not remove. It moves the elements to keep to the front and returns the rest of
// the range, which the container still has to be told to drop.
const auto leftover = std::ranges::remove(values, 0);
values.erase(leftover.begin(), leftover.end());

// Since C++20 a container can do both steps itself:
std::erase_if(values, [](int value) { return value > 10; });
```

An algorithm sees a range, not a container; it can move elements, but it cannot change how many there are.
`remove`, `remove_if` and `unique` therefore shuffle the elements to keep to the front and return the part
of the range that is left over. The container has the same size as before until you `erase` that part. The
`std::ranges` versions return the leftover part as a range, which makes the second step harder to get wrong
than it was with iterators. `std::erase` and `std::erase_if` (C++20) are free functions for the standard
containers that do both steps; when you have the container in hand, use those.

### A temporary range gives you nothing to hold

An algorithm that returns an iterator returns a pointer into the range. If the range was a temporary
container, it is gone at the end of the statement, and the iterator with it. The range algorithms detect
this at compile time: for an rvalue container they return an object of type `std::ranges::dangling`, which
has no members, so the first use of it fails to compile. The third experiment shows the message.

A `std::span` or a `std::string_view` passed as a temporary is fine, and the algorithms know it: destroying
a view does not destroy what it looks at. Such types are called *borrowed ranges*.

### The range form of `equal` compares lengths, the old one does not

`std::equal(a.begin(), a.end(), b.begin())`, the form C++98 offered, is given one whole range and only the
*start* of the second. It compares as many elements as the first range has. If the second is longer, the
extra elements are ignored, so "war" equals "warn". If the second is shorter, it reads past its end.
`std::ranges::equal(a, b)` knows both ends, and two ranges of different length are not equal. This is why
`parse_level("war")` and `parse_level("warning")` both give nothing, and the first two experiments show
what the old form does with each.

### Sorted-range algorithms need the same order they were sorted by

<!-- file: chapters/03-ranges-algorithms/examples/e01_ranges.cpp#sorted -->
```cpp
std::vector<Person> list = people();
std::ranges::sort(list, {}, &Person::age);  // 36 54 85 85

// On a sorted range, searching is a binary search. The projection has to be the one it was
// sorted by.
const auto firstOver50 = std::ranges::lower_bound(list, 50, {}, &Person::age);
std::cout << firstOver50->name << ' ' << std::ranges::is_sorted(list, {}, &Person::age) << '\n';
```

`lower_bound`, `upper_bound`, `equal_range`, `binary_search`, `partition_point` and the set algorithms
assume that the range is sorted, and by exactly the comparator and projection you pass them. Nothing checks
this. Sort by age and search by name, and you get an answer that means nothing.

## The algorithms worth knowing by name

All in `<algorithm>`, all in `std::ranges`, C++20 unless marked.

| Question | Algorithms |
| --- | --- |
| Is it there? Where? | `find`, `find_if`, `find_if_not`, `contains` (23), `count`, `count_if` |
| Does it hold for all, some, none? | `all_of`, `any_of`, `none_of` |
| Smallest, largest | `min_element`, `max_element`, `minmax_element` (iterators); `min`, `max`, `minmax` (values) |
| Are two ranges alike? | `equal`, `mismatch`, `starts_with` (23), `ends_with` (23), `lexicographical_compare` |
| Put in order | `sort`, `stable_sort`, `partial_sort`, `nth_element`, `is_sorted`, `is_sorted_until` |
| Search a sorted range | `lower_bound`, `upper_bound`, `equal_range`, `binary_search`, `partition_point` |
| Drop elements | `remove`, `remove_if`, `unique`, then `erase`; or `std::erase`, `std::erase_if` |
| Combine sorted ranges | `merge`, `set_difference`, `set_intersection`, `set_union`, `includes` |
| Copy or change | `copy`, `copy_if`, `transform`, `fill`, `replace`, `reverse`, `rotate` |
| Reduce to one value | `fold_left` (23); `std::accumulate` in `<numeric>` has no range form |

## When to reach for it, and when not

- **Prefer the `std::ranges` form** of any algorithm. It takes the range, it takes projections, and it
  checks its arguments with concepts, so a mistake is reported at your call and not inside the library.
- **Use a projection** whenever a lambda would only pick a member out of its argument.
- **Keep the plain loop** when the body does more than one thing, needs to stop on a condition that is not
  the algorithm's own, or has side effects in a particular order. `parse_level` itself is a loop around one
  algorithm call, and reads well that way.
- **Use `stable_sort`** when the order of equal elements is part of the result. Sorting a table by one
  column and expecting the previous order to survive within equal keys is the usual case.
- The `std::ranges` algorithms have no parallel versions; the execution-policy overloads exist only for the
  old forms.

## Drills

Run each with `ctest --preset clang-debug -R ch03_dNN`. `exercises/person.h` is given and shared by the
first three.

| # | Target | File | Write |
| --- | --- | --- | --- |
| 1 | `ch03_d01` | `exercises/d01_sort_by_age.h` | `sort_by_age`: youngest first, equal ages keep their order |
| 2 | `ch03_d02` | `exercises/d02_oldest.h` | `oldest`: a pointer to the oldest person |
| 3 | `ch03_d03` | `exercises/d03_name_of.h` | `name_of`: the name that belongs to an id, if any |
| 4 | `ch03_d04` | `exercises/d04_iequals.h` | `iequals`: equality that ignores letter case |
| 5 | `ch03_d05` | `exercises/d05_drop_blank.h` | `drop_blank`: remove blank lines from a vector |
| 6 | `ch03_d06` | `exercises/d06_first_out_of_order.h` | `first_out_of_order`: where timestamps first go backwards |
| 7 | `ch03_d07` | `exercises/d07_top_n.h` | `top_n`: the best scores, ties by name |

**Drill 1: `sort_by_age`.** One line, no lambda. The second test builds three hundred people of whom a
hundred are thirty years old, and requires those hundred to stay in the order they were in.

**Drill 2: `oldest`.** One algorithm call and a conversion from iterator to pointer. The first test checks
that what you return is the element in the caller's storage: not a copy, and not the number 85. The
parameter is chapter 1's span.

**Drill 3: `name_of`.** Find by id, with a value and a projection. The return type combines chapters 1 and
2: an optional view. One of the people has id 0.

**Drill 4: `iequals`.** `to_lower_ascii` is given. This is the logger's line; write it without looking. The
last test passes texts of different lengths, one of them in a buffer with nothing behind it, and there is a
second test, `ch03_d04.c1`, that asks the same questions at compile time.

**Drill 5: `drop_blank`.** A blank line is empty or consists of spaces and tabs. Either form from "`remove`
does not remove" will do; try both.

**Drill 6: `first_out_of_order`.** There is an algorithm that does precisely this, and it takes a
projection. Turn the iterator it returns into an index.

**Drill 7: `top_n`.** Two keys: more points first, then the name in alphabetical order. A projection that
returns a pair does it; so does a comparator that compares two scores. Write whichever you find clearer,
then read the other in the solution. The parameter is taken by value, so it is yours to sort.

## Design problems

**Problem 1: a ranking** (`ch03_p01`, you create `exercises/p01_ranking.h`).
A match produces results: a name, a number of points, and a finishing time in seconds. Write a struct
`ch03::Result` with members `name`, `points` and `finishSeconds`, and a class `ch03::Ranking` that is built
from a `std::vector<Result>` and answers three questions:

- `Table()`: all results in rank order. More points rank higher; among equal points the earlier finish;
  among results equal in both, the alphabetically first name.
- `RankOf(name)`: that person's rank, 1 for the winner, or the fact that they did not take part.
- `WithinOfLeader(n)`: the results that are at most `n` points behind the leader, in rank order.

The questions are asked many times, the table is built once.

**Problem 2: what changed in the warehouse** (`ch03_p02`, you create `exercises/p02_inventory_diff.h`).
An inventory is a list of items, each a SKU (a text) and a quantity. Given yesterday's inventory and
today's, write `ch03::diff(before, after)`, which reports three lists of SKUs: those that were *added*, those
that were *removed*, and those that are in both with a *changed* quantity. Each list is sorted. Neither
inventory arrives in any particular order, and neither lists a SKU twice. A warehouse has a hundred thousand
SKUs, and the comparison runs every night; one of the tests has that size and a time limit.

Write a struct `ch03::Item` with members `sku` and `quantity`; the result has members `added`, `removed`
and `changed`.

**Problem 3: abbreviated commands** (`ch03_p03`, you create `exercises/p03_resolve_command.h`).
A command-line tool has sub-commands such as `status`, `start`, `stop` and `commit`. Users may abbreviate:
any beginning of a command's name stands for that command, in any letter case, as long as no other command
begins the same way. A name typed in full always means that command, even if a longer command starts with
it. Write `ch03::resolve(commands, typed)`, which returns the command that was meant, spelt as it is in the
list, and `ch03::explain(result)`, which for a result that is not a command returns `unknown command` or
`ambiguous command`. An empty input is the beginning of every command.

**Problem 4: `parse_level`, from nothing** (`ch03_p04`, you create `exercises/p04_parse_level.h`).
Close the guide and the logger's source. `exercises/level.h` gives you the `Level` enumeration and
`level_name`, as in step 1. Write `ch03::parse_level`: it takes a text and returns the level with that name,
ignoring the case of ASCII letters, or the fact that there is no such level. The tests are the logger's own.

When they pass, compare with `src/Level.cpp` in the logger. Where yours differs, decide which you prefer
and why.

## Break it on purpose

**Compare two texts with the old `std::equal`.** `experiments/x01_three_iterator_equal.cpp` has both forms:

<!-- file: chapters/03-ranges-algorithms/experiments/x01_three_iterator_equal.cpp#functions -->
```cpp
// The form C++98 had: one whole range, and only the *start* of a second one.
bool same_old(std::string_view a, std::string_view b)
{
    return std::equal(a.begin(), a.end(), b.begin());  // NOLINT(modernize-use-ranges)
}

// The range form knows where both texts end.
bool same_new(std::string_view a, std::string_view b)
{
    return std::ranges::equal(a, b);
}
```

```sh
cmake --build --preset clang-debug -t x03_01 && build/clang-debug/bin/x03_01
```

<!-- file: chapters/03-ranges-algorithms/experiments/x01_three_iterator_equal.expect -->
```
std::equal:         "war" and "warn" are equal: true
std::ranges::equal: "war" and "warn" are equal: false
```

With the old form, a `parse_level` would accept `war`, `e` and the empty text. Put the longer text first
and it is worse. `experiments/x02_equal_overrun.cpp`:

<!-- file: chapters/03-ranges-algorithms/experiments/x02_equal_overrun.cpp#main -->
```cpp
const std::vector<char> raw{'w', 'a', 'r'};  // three characters, and the buffer ends there
const std::string_view  typed(raw.data(), raw.size());
const std::string_view  name = "warning";

// Seven characters of `name` are compared with seven characters starting at typed.begin().
const bool same =
    std::equal(name.begin(), name.end(), typed.begin());  // NOLINT(modernize-use-ranges)
```

Under `asan` (`cmake --build --preset asan -t x03_02 && build/asan/bin/x03_02`):

<!-- file: chapters/03-ranges-algorithms/experiments/x02_equal_overrun.asan.expect -->
```
ERROR: AddressSanitizer: heap-buffer-overflow
READ of size 7
is located 0 bytes after 3-byte region
```

Seven bytes were read from a buffer of three.

**Search a container that no longer exists.** `experiments/x03_dangling_find.cpp`:

<!-- file: chapters/03-ranges-algorithms/experiments/x03_dangling_find.cpp#main -->
```cpp
    const std::vector<Person> people = load_people();
    std::cout << std::ranges::find(people, 7, &Person::id)->name
              << '\n';  // fine: `people` is still here

#ifdef TUT_BREAK
    // The vector that load_people() returns is destroyed at the end of this statement...
    const auto found = std::ranges::find(load_people(), 7, &Person::id);
    std::cout << found->name << '\n';  // ...so what would `found` point at?
#endif
```

`cmake --build --preset clang-debug -t x03_03` fails. Clang says that there is
`no member named 'name' in 'std::ranges::dangling'`; GCC says that the
`base operand of '->' has non-pointer type 'const std::ranges::dangling'`. Both name the type:

<!-- file: chapters/03-ranges-algorithms/experiments/x03_dangling_find.expect -->
```
std::ranges::dangling
```

With the iterator form, `std::find(load_people().begin(), load_people().end(), ...)`, the same mistake
compiles and is chapter 1's use-after-free. Here the library turned it into a type error. When you see
`dangling` in a message, the fix is always the same: give the range a name, so that it lives long enough.

**Put the projection where the comparator belongs.** `experiments/x04_projection_as_comparator.cpp`:

<!-- file: chapters/03-ranges-algorithms/experiments/x04_projection_as_comparator.cpp#main -->
```cpp
    std::ranges::sort(people, {}, &Person::age);  // comparator: the default; projection: the age

#ifdef TUT_BREAK
    std::ranges::sort(people, &Person::age);  // the age, in the comparator's place
#endif
```

`cmake --build --preset clang-debug -t x03_04` fails with one error and a long list of notes. These three
lines are the ones to find, in this order:

<!-- file: chapters/03-ranges-algorithms/experiments/x04_projection_as_comparator.clang.expect -->
```
no matching function for call to object of type 'const __sort_fn'
_Comp = int Person::*, _Proj = identity
is_invocable_v<int
```

How to read such a message. The first line says no overload of `sort` fits. The compiler then lists every
candidate and why it was rejected. `sort` has two: one taking an iterator and a sentinel, and one taking a
range. Skip the first; you did not pass iterators, and its complaint that a vector is not an iterator is
true and irrelevant. The second line above is the candidate you meant, and it shows what the compiler made
of your arguments: `_Comp = int Person::*`. Your member pointer was taken as the comparator. The last line
is the innermost reason: a comparator is called with two elements, and a pointer to a data member cannot be
invoked with two `Person`s. GCC's message has the same content in a different layout; look for `_Comp =`.

**Remove, and forget the container.** `experiments/x05_remove_without_erase.cpp`:

<!-- file: chapters/03-ranges-algorithms/experiments/x05_remove_without_erase.cpp#main -->
```cpp
std::vector<int> values{1, 0, 2, 0, 3};

const auto leftover =
    std::ranges::remove(values, 0);  // moves 1 2 3 to the front, returns the rest
show("after remove: ", values);
std::cout << "the leftover part has " << leftover.size() << " elements\n";

values.erase(leftover.begin(), leftover.end());  // only this changes the size
show("after erase:  ", values);
```

<!-- file: chapters/03-ranges-algorithms/experiments/x05_remove_without_erase.expect -->
```
after remove: 5 elements: 1 2 3
the leftover part has 2 elements
after erase:  3 elements: 1 2 3
```

After `remove` the vector still has five elements. The first three are the ones that were kept; the program
prints two more after them, and what those are is unspecified. Only `erase` makes the vector shorter.

## Back in logger

The answers are in `solutions/03-ranges-algorithms/README.md`.

1. Name the role of each of the five arguments in the call at `src/Level.cpp:28`. Then predict: what would
   `parse_level("warn")` return if the last argument were left out? What makes `parse_level("war")` return
   nothing, and what would `std::equal(text.begin(), text.end(), name.begin())` have done with `"war"`, and
   with `"warning"`?
2. In `std::ranges::is_sorted(records, {}, &logger::Record::timestamp)`, how does a pointer to a member
   become something the algorithm can call? Rewrite the call without a projection.
3. `to_lower_ascii` (`src/Level.cpp:14-17`) is six lines the logger wrote itself. `std::tolower` exists.
   Give three reasons why it could not be used here; the guide's step 1 names two of them.

## Checkpoint

You should be able to explain:

- what `{}` means in `std::ranges::sort(people, {}, &Person::age)`, and why it has to be written;
- what `std::ranges::max_element(people, {}, &Person::age)` returns, and what the projection had to do
  with that;
- why `std::ranges::equal("war", "warn")` is false and the three-iterator form says true;
- what `std::ranges::dangling` in an error message is telling you to change.

Next: move semantics. So far every function has borrowed its arguments or copied them. The logger moves:
a message, once formatted, travels from the caller's thread to a file without its characters ever being
copied again.

---

Previous: [Chapter 2: `std::optional`](../02-optional/README.md) | [Index](../../README.md) | Next: [Chapter 4: Move semantics](../04-move-semantics/README.md)
