# Chapter 0: Warm-up

Two things happen in this chapter, and neither is hard. You learn the loop that every later chapter uses: run
an exercise, read why it fails, write code, run it again. And you pick up six small conveniences from C++14 to
C++23 that the logger uses on almost every page without comment, and that every reference solution in this
tutorial uses too. An hour, perhaps less.

## Goal

- You can build and run one exercise, one chapter, and a reference solution, and you can reset an exercise.
- You know the three kinds of work a chapter asks for: drills, design problems, and experiments.
- You can read and write structured bindings, an `if` with an initialiser, class template argument deduction,
  designated initialisers, `std::exchange` and `[[nodiscard]]` without stopping to think.

## The exercise loop

Do this once, in the repository's root:

```sh
git switch -c work                  # your answers live on this branch; main stays as it was
cmake --workflow --preset dev       # configures, builds the examples, runs them
```

The second command must end with `100% tests passed`. It proves that your compiler, GoogleTest and the
tutorial's own support code work, before you have written anything. Nothing else is built yet: every exercise
is its own small program and is compiled when you ask for it.

### A drill

A drill is a function or a class with its interface given and its body missing. The file is in `exercises/`,
its tests are in `tests/`, and both belong to one target, here `ch00_d02`: chapter 00, drill 02.

```sh
ctest --preset clang-debug -R ch00_d02
```

That builds the drill and runs its tests. Before you have touched it, you get:

```
    Start 13: ch00_d02.build
1/2 Test #13: ch00_d02.build ...................   Passed    1.36 sec
    Start 14: ch00_d02
2/2 Test #14: ch00_d02 .........................***Failed    0.00 sec
unknown file: Failure
C++ exception with description "NOT IMPLEMENTED: int ch00::first_repeat(const std::vector<int> &)
(d02_first_repeat.h:13)" thrown in the test body.

[  FAILED  ] FirstRepeat.ReportsTheValueThatRepeatsFirst (0 ms)
```

Two tests ran. `ch00_d02.build` compiled the drill; had there been a compiler error, it would have been printed
there and the second test would not have run. `ch00_d02` is the test program itself, and its output is
GoogleTest's. Open `exercises/d02_first_repeat.h` and you find the reason:

<!-- file: chapters/00-warm-up/exercises/d02_first_repeat.h -->
```cpp
#pragma once

#include <vector>

#include "tut/todo.h"

namespace ch00
{

/// The first value that turns up for the second time, or -1 when no value repeats.
[[nodiscard]] inline int first_repeat(const std::vector<int>& values)
{
    TUT_TODO(values);
}

}  // namespace ch00
```

`TUT_TODO(values);` is the whole body of every unwritten function in this tutorial. It throws, which is why a
function that promises an `int` needs no made-up `return`. Its arguments are the function's parameters, so that
the compiler does not warn about parameters nobody uses. Delete the line, write the function, run the same
`ctest` command again.

Three things worth knowing about the loop:

- **Warnings are errors.** The tutorial compiles with the logger's warning set and `-Werror`, because `ctest`
  shows compiler output only when a build fails: a warning that did not stop the build would never reach you.
- **One exercise never blocks another.** Each has its own target. A drill that does not compile yet is that
  drill's problem only.
- **To run a single test case, or to use a debugger,** run the program directly once it has been built:
  `build/clang-debug/bin/ch00_d02 --gtest_filter='FirstRepeat.Neighbours*'`.

### A whole chapter, and the scoreboard

```sh
cmake --build --preset clang-debug -t ch00     # optional: compiles the chapter's exercises in parallel
ctest --preset clang-debug -R ch00             # every exercise of chapter 0
ctest --preset clang-debug                     # every exercise of the tutorial
```

Without the first line the result is the same; `ctest` then compiles one exercise at a time.

### A design problem

A design problem has no file to start from. The chapter describes what is wanted, in terms of behaviour, and
the tests fix only how your code is called. Which types and which library features you use is the exercise.

```sh
ctest --preset clang-debug -R ch00_p01
```

```
ch00_p01 is a design problem: it starts from no file at all.
Create chapters/00-warm-up/exercises/p01_most_frequent.h and run this again.
```

Create that file and run the same command; the build notices the new file by itself. If you want to split
your answer, any `exercises/p01_most_frequent*.cpp` is compiled in as well.

### An experiment

Every chapter has a section called "Break it on purpose", as the rebuild guide does. Here the broken code is
already written, in `experiments/`, and each experiment is a target you build and run yourself. The outcome
quoted in the chapter is the one that was observed; the tutorial's own checks fail if it stops being true.

Some experiments only show their point under a sanitizer. The `asan` preset builds with AddressSanitizer and
UndefinedBehaviorSanitizer, and works for exercises exactly as `clang-debug` does:

```sh
cmake --preset asan                             # once; downloads GoogleTest to build it instrumented
ctest --preset asan -R ch00_d02                 # your drill, under the sanitizers
```

### Stuck, finished, or starting over

- **The reference solution** of every exercise is in `solutions/00-warm-up/`, under the same file name. For
  design problems `solutions/00-warm-up/README.md` also says why it is built the way it is, and what else
  would have been a good answer. `ctest --preset solutions -R sol00_d02` runs the same tests against it.
- **To reset a drill** to its untouched state: `git restore --source=main -- chapters/00-warm-up/exercises/d02_first_repeat.h`.
  To reset a design problem, delete your file.
- **Other compilers and checks:** the presets `gcc-debug`, `asan`, `tsan` and `tidy` take the same commands.
  `tidy` runs clang-tidy with the logger's rules on your code, which is worth doing on a finished chapter.

## Six small conveniences

None of these changes what a program can do. Each removes a line or a name that C++11 made you write, and each
is in the logger.

### Structured bindings (C++17)

A structured binding gives names to the parts of something that has parts: a `std::pair`, a `std::tuple`, an
array, or a struct with public members.

<!-- file: chapters/00-warm-up/examples/e01_conveniences.cpp#bindings -->
```cpp
const std::map<std::string, int> stock{{"apple", 3}, {"pear", 0}, {"plum", 12}};

for (const auto& [name, count] : stock)  // each element is a std::pair<const std::string, int>
{
    std::cout << name << ": " << count << '\n';
}
```

Before C++17 the loop variable was a pair, and the body said `entry.first` and `entry.second`. The names
`name` and `count` are not copies and not references of their own: they are other names for the two members
of the thing on the right, and the `const auto&` in front applies to that thing as a whole.

The logger's tests use one to walk a table of levels and expected texts (step 2 of the guide):

<!-- logger: tests/RecordTests.cpp:68-74 -->
```cpp
for (const auto& [level, tag] : {
         std::pair{Level::Error, std::string_view{"[ERROR]"}},
         std::pair{Level::Warn, std::string_view{"[WARN ]"}},
         std::pair{Level::Info, std::string_view{"[INFO ]"}},
         std::pair{Level::Debug, std::string_view{"[DEBUG]"}},
         std::pair{Level::Perf, std::string_view{"[PERF ]"}},
     })
```

### `if` with an initialiser (C++17)

`if (init; condition)` declares a variable that exists for the `if` and its `else`, and nowhere else.

<!-- file: chapters/00-warm-up/examples/e01_conveniences.cpp#if-init -->
```cpp
if (const auto found = stock.find("pear"); found != stock.end())
{
    std::cout << "pear is listed, " << found->second << " in stock\n";
}
// `found` no longer exists here.
```

The gain is scope. Without it `found` is declared on the line above and stays alive, and usable by mistake,
until the end of the enclosing block. The pattern fits everything that returns "an answer, and whether there
is one": `find`, `insert`, and later `std::from_chars`.

The logger does not happen to use this form. It uses its older sibling, a declaration that is itself the
condition, which works when the variable can be tested directly:

<!-- logger: src/Core.cpp:126-129 -->
```cpp
if (const auto parsed = parse_level(raw))
{
    threshold.store(*parsed, std::memory_order_relaxed);
}
```

### Class template argument deduction (C++17)

When you construct an object of a class template, the compiler can work out the template arguments from the
constructor arguments, as it always could for function templates. This is usually called CTAD.

<!-- file: chapters/00-warm-up/examples/e01_conveniences.cpp#ctad -->
```cpp
std::mutex mutex;

const std::pair        entry{"plum", 12};       // std::pair<const char*, int>
const std::vector      primes{2, 3, 5, 7};      // std::vector<int>
const std::array       corners{0.0, 0.5, 1.0};  // std::array<double, 3>
const std::scoped_lock lock(mutex);             // std::scoped_lock<std::mutex>
```

It is why the logger can say `std::unique_lock lock(mutex);` and `const std::scoped_lock lock(mutex);` rather
than `std::unique_lock<std::mutex> lock(mutex);`, and why its table of levels does not have to count its own
elements (step 1):

<!-- logger: src/Level.cpp:23-25 -->
```cpp
constexpr std::array kLevels{
    Level::Off, Level::Error, Level::Warn, Level::Info, Level::Debug, Level::Perf,
};
```

One caution. CTAD deduces from what you wrote, not from what you meant: `std::pair entry{"plum", 12}` holds a
`const char*`, not a `std::string`. When the type matters, say it.

### Designated initialisers (C++20)

For a struct without constructors you may name the members you are initialising.

<!-- file: chapters/00-warm-up/examples/e01_conveniences.cpp#designated-type -->
```cpp
struct Window
{
    std::string title;
    int         width      = 800;
    int         height     = 600;
    bool        fullscreen = false;
};
```

<!-- file: chapters/00-warm-up/examples/e01_conveniences.cpp#designated -->
```cpp
const Window dialog{.title = "Save as", .width = 400, .height = 300};
```

Members you leave out get their default member initialiser, or are value-initialised if they have none:
`dialog.fullscreen` is `false`. The names must come in the order the members are declared in, and you cannot
skip into a nested member. Compared with `Window{"Save as", 400, 300}`, nobody has to remember which number
is the width, and adding a member to the struct breaks no existing initialiser.

The logger builds every `Record` this way (step 5):

<!-- logger: src/Core.cpp:257-262 -->
```cpp
Record record{
    .timestamp = {},
    .level     = level,
    .location  = location,
    .message   = std::move(message),
};
```

### `std::exchange` (C++14)

`std::exchange(object, newValue)` stores `newValue` in `object` and returns what `object` held before. It is
one line for "take the value and leave something defined behind".

<!-- file: chapters/00-warm-up/examples/e01_conveniences.cpp#exchange -->
```cpp
std::string pending = "disk almost full";

const std::string taken =
    std::exchange(pending, {});  // take the value, leave an empty one behind
```

The logger uses it in exactly this role. Here a pending complaint is handed to the caller and cleared in the
same expression (step 11):

<!-- logger: src/Core.cpp:229-232 -->
```cpp
std::optional<std::string> Core::TakeStartupErrorLocked() noexcept
{
    return std::exchange(startupError, std::nullopt);
}
```

And in the demo, a sink that fails the first time only (step 14):

<!-- logger: src/main.cpp:58-61 -->
```cpp
if (!std::exchange(failed, true))
{
    throw std::runtime_error("simulated disk full");
}
```

Read that condition slowly once: it sets `failed` to `true` and asks what it was before. Chapter 4 comes
back to `std::exchange`, because "take and leave something defined behind" is also what a move constructor
has to do.

### `[[nodiscard]]` (C++17)

An attribute on a function, or on a type, that makes the compiler warn when a caller ignores the result.

<!-- file: chapters/00-warm-up/examples/e01_conveniences.cpp#nodiscard -->
```cpp
[[nodiscard]] bool save(const std::string& text)
{
    return !text.empty();
}
```

Calling `save("report");` as a statement now produces a warning, and in this tutorial and in the logger a
warning is an error. Put it on every function whose result is the reason for calling it: a parser, a getter,
anything that reports success by its return value. The logger does:

<!-- logger: include/logger/Level.h:89 -->
```cpp
[[nodiscard]] std::optional<Level> parse_level(std::string_view text) noexcept;
```

When ignoring a result is what you mean, say so with `static_cast<void>(...)`, as the logger does where it
writes to `stderr` and cannot do anything about a failure.

### And one from C++23: `std::to_underlying`

`std::to_underlying(Level::Info)` converts a scoped enumerator to its underlying integer type, whatever that
is. It replaces `static_cast<int>(level)`, which names a type that may be the wrong one.

<!-- file: chapters/00-warm-up/examples/e01_conveniences.cpp#enum -->
```cpp
enum class Level : std::uint8_t
{
    Error,
    Warn,
    Info,
};
```

<!-- file: chapters/00-warm-up/examples/e01_conveniences.cpp#to-underlying -->
```cpp
const Level level = Level::Info;
std::cout << "level " << static_cast<int>(std::to_underlying(level)) << '\n';
```

The remaining cast is there because `Level`'s underlying type is `std::uint8_t`, which streams print as a
character. The whole example is `examples/e01_conveniences.cpp`; it is built by the `dev` workflow and prints:

<!-- file: chapters/00-warm-up/examples/e01_conveniences.out -->
```
apple: 3
pear: 0
plum: 12
pear is listed, 0 in stock
plum 4 3
Save as 400x300 windowed
level 2
taken: 'disk almost full', left behind: ''
saved
```

## Drills

Run each with `ctest --preset clang-debug -R ch00_dNN`.

| # | Target | File | Write |
| --- | --- | --- | --- |
| 1 | `ch00_d01` | `exercises/d01_min_max.h` | `min_max`: the smallest and largest value of a vector, in one `Range` |
| 2 | `ch00_d02` | `exercises/d02_first_repeat.h` | `first_repeat`: the first value that occurs for the second time |
| 3 | `ch00_d03` | `exercises/d03_make_request.h` | `make_request`: a `Request` with defaults, or the upload variant |
| 4 | `ch00_d04` | `exercises/d04_toggle.h` | `Toggle::TakeAndReset`: report the flag and clear it |

**Drill 1: `min_max`.** Return both ends of the range of values in one `Range`. `std::minmax_element` finds
both in one pass and returns a pair of iterators: take it apart with a structured binding. The tests do the
same to your result.

This drill has a second test, `ch00_d01.r1`, of a kind you will meet again: it checks that something does
**not** compile. `tests/d01_r1_discarded.cpp` contains a function that calls `min_max` and throws the answer
away, and that function must be rejected by the compiler. With the stub as it is, it is accepted, so the test
fails. Find the one word that fixes it.

**Drill 2: `first_repeat`.** Walk the values, remember the ones you have seen in a `std::set`, and return the
first one that is already there. `std::set::insert` returns a pair of an iterator and a `bool` that says
whether the value was new: an `if` with an initialiser and a structured binding make that one line.

The function returns -1 when nothing repeats. Ask yourself what it returns for `{-1, 5, -1}`, and what the
caller can conclude from that. Chapter 2 is about the type that fixes this.

**Drill 3: `make_request`.** Build the `Request` with designated initialisers. For an ordinary path, name
only what differs from the defaults.

**Drill 4: `Toggle::TakeAndReset`.** One expression. The function is `noexcept`, and a stub that throws out
of a `noexcept` function ends the program on the spot: untouched, this drill does not report three failed
cases but stops at the first one, with `terminate called after throwing an instance of 'tut::NotImplemented'`.
You will see that wherever a stub's signature says `noexcept`.

## Design problem

**Problem 1: the most frequent word** (`ch00_p01`, you create `exercises/p01_most_frequent.h`).
Given a list of words, report the word that occurs most often and how many times it occurs. When several
words are equally frequent, report the one that comes first alphabetically. The list is never empty.

The tests call `ch00::most_frequent(words)` with a `std::vector<std::string>` and take the result apart into
a word and a count with a structured binding. What the function returns, and what it uses to count, is yours
to decide. When your tests pass, compare with `solutions/00-warm-up/p01_most_frequent.h` and read the notes
on it in `solutions/00-warm-up/README.md`.

## Break it on purpose

**Ignore a result that is marked `[[nodiscard]]`.** `experiments/x01_discarded_result.cpp` calls such a
function twice, and the second time does nothing with what comes back.

```sh
cmake --build --preset clang-debug -t x00_01
```

The build fails. With either compiler the message contains these two pieces:

<!-- file: chapters/00-warm-up/experiments/x01_discarded_result.expect -->
```
ignoring return value of
nodiscard
```

Clang's ends in `[-Werror,-Wunused-result]` and GCC's in `[-Werror=unused-result]`: a warning, promoted to an
error. This is also all that drill 1's extra test is asking for.

**Keep a reference into a vector across a `push_back`.** `experiments/x02_dangling_reference.cpp`:

<!-- file: chapters/00-warm-up/experiments/x02_dangling_reference.cpp#main -->
```cpp
std::vector<int> values{1, 2, 3};
const int&       first = values.front();  // refers to the vector's current storage

values.push_back(4);  // no room left: the vector moves to new storage and frees the old one

std::cout << "first is " << first << '\n';  // reads the storage that was freed
std::cout << "finished without a complaint\n";
```

```sh
cmake --build --preset clang-debug -t x00_02 && build/clang-debug/bin/x00_02
```

It compiles without a warning, runs, and exits with 0:

<!-- file: chapters/00-warm-up/experiments/x02_dangling_reference.expect -->
```
first is
finished without a complaint
```

Between those two lines it printed a number; which number depends on what the memory allocator did with the
freed block. Nothing told you that the program is wrong. Now the same file under AddressSanitizer:

```sh
cmake --build --preset asan -t x00_02 && build/asan/bin/x00_02
```

It stops at the read, with a report that contains these lines, each followed by a stack trace:

<!-- file: chapters/00-warm-up/experiments/x02_dangling_reference.asan.expect -->
```
ERROR: AddressSanitizer: heap-use-after-free
READ of size 4
freed by thread T0 here:
previously allocated by thread T0 here:
```

The three traces say where the freed memory was read, where it was freed (inside `push_back`) and where it
had been allocated. This is the reason every chapter asks you to run your finished
exercises under the `asan` preset as well. A test that passes tells you the answer came out right on this
run; it does not tell you that the program had the right to compute it. Chapter 1 is about exactly this kind
of mistake, with the two types that make it easiest to commit.

## Checkpoint

You should be able to explain:

- what `ctest --preset clang-debug -R ch00_d02` does, in order, and where a compiler error would appear;
- what the names in a structured binding refer to, and what the `const auto&` in front of them applies to;
- what `if (!std::exchange(failed, true))` tests, and what it has changed by the time it is tested;
- why a program can pass all of its tests and still be reported by AddressSanitizer.

Next: views. `std::string_view` and `std::span` are the types the logger passes text and records around in,
and the easiest way in C++ to end up holding a reference to something that no longer exists.

---

[Index](../../README.md) | Next: [Chapter 1: Views that do not own](../01-views/README.md)
