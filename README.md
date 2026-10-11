# Modern C++ by exercise

A tutorial on the parts of modern C++ that the `logger` library is built from, written as a companion to
that library's rebuild guide. The guide tells you what to type and why the *design* is the way it is. This
tutorial is about the *language and library features* the design is written in: the ones you can follow
when you copy them and could not yet have produced yourself. Each chapter explains one of them from the
ground up, shows where the logger uses it, and then makes you use it, many times, until it is a tool you
reach for.

**Status:** chapters 0 to 5 and the first review set are written, about 23 hours of work: 43 drills,
17 design problems and 27 experiments in the six chapters, and a review set with five more problems and
five pieces of code to repair. The remaining ten chapters are planned in [`ROADMAP.md`](ROADMAP.md).

## What you need

- A C++23 compiler and standard library with `<expected>`, `<format>` and the C++23 additions to
  `<algorithm>`. Written and checked with GCC 16.2 and Clang 23.1, both on libstdc++ 16; nothing older has
  been tried.
- CMake 3.28 or newer, and Ninja.
- GoogleTest: an installed copy is used when CMake finds one, otherwise it is downloaded when you configure.
- For the sanitizer and clang-tidy presets: Clang with its sanitizer runtimes, and clang-tidy.
- The logger repository and its rebuild guide, to read alongside. Nothing here needs it to build.
- What the guide asks for: working knowledge of C++ templates, and C++11 habits such as `auto`, lambdas and
  smart pointers.

## How to use it

```sh
git switch -c work                  # your answers live on this branch; main keeps the untouched exercises
cmake --workflow --preset dev       # configures, builds the examples and runs them: must pass
```

Then start with [chapter 0](chapters/00-warm-up/README.md). It takes an hour, and the first half of it is
about how the exercises work: one command builds and runs an exercise, and that command is all you need.

```sh
ctest --preset clang-debug -R ch01_d03      # build and run one exercise
ctest --preset clang-debug -R ch01          # a whole chapter
ctest --preset asan -R ch01_d03             # the same under AddressSanitizer (cmake --preset asan first)
ctest --preset solutions -R sol01_d03       # the reference solution against the same tests
git restore --source=main -- chapters/01-views/exercises/d03_next_token.h      # start a drill again
```

Work in order. The chapters build on each other, and from chapter 2 on the exercises deliberately reuse
what earlier chapters taught. One exception: chapters 4 and 5 (moving and forwarding) can be done before
chapters 1 to 3 if those are what you came for.

## The chapters

| # | Chapter | You learn to use | In the logger | Hours |
| --- | --- | --- | --- | --- |
| 0 | [Warm-up](chapters/00-warm-up/README.md) | the exercise loop; structured bindings, `if` with initialiser, CTAD, designated initialisers, `std::exchange`, `[[nodiscard]]` | the idioms on every page | 1 |
| 1 | [Views that do not own](chapters/01-views/README.md) | `std::string_view`, `std::span` | `file_basename`, `WriteBatch` | 3 |
| 2 | [`std::optional`](chapters/02-optional/README.md) | `optional`, its chaining functions, `std::expected` | `parse_level`, `startupError` | 3 |
| 3 | [Ranges I: algorithms and projections](chapters/03-ranges-algorithms/README.md) | `std::ranges` algorithms, comparators, projections | the one line in `parse_level` | 3 |
| 4 | [Move semantics and value categories](chapters/04-move-semantics/README.md) | `std::move`, rvalue references, the rule of zero and five | a message's path from caller to sink | 5 |
| 5 | [Variadic templates and perfect forwarding](chapters/05-forwarding/README.md) | parameter packs, folds, `std::forward`, `requires` | `add_sink`, every log function | 5 |
| | [Review A](chapters/ra-review-a/README.md) | choosing among all of the above | | 3 |
| 6 | `constexpr`, `consteval`, `if constexpr` | *planned* | `level_name`, compile-time stripping | |
| 7 | `std::format` and the spec language | *planned* | the line layout | |
| 8 | `std::format_to`, output iterators, formatters | *planned* | `format_line_to` | |
| 9 | `std::format_string` and `std::source_location` | *planned* | `FormatString` | |
| 10 | `std::chrono` | *planned* | `format_timestamp` | |
| 11 | `std::filesystem` | *planned* | `FileSink` | |
| 12 | Ranges II: views and pipelines | *planned* | | |
| 13 | Mutex and `std::condition_variable` | *planned* | the queue and its three condition variables | |
| 14 | Shutdown, timed waits, `stop_token`, atomics | *planned* | `Core::Stop` | |
| 15 | Capstones | *planned* | | |

The order is not the order of the list this tutorial grew from, which began with `std::move`. Views,
`optional` and range algorithms are short, each is useful on its own, and together they are everything
`parse_level` is made of: at the end of chapter 3 you write that function from an empty file. Value
categories are the most abstract subject here and are easier once "owning" and "borrowing" are familiar.

## Four kinds of practice

- **Drills.** A function or class with its interface given and its body missing, and tests that fail until
  you have written it. Small on purpose: most take ten to thirty lines. A few are tables of predictions
  (what will the compiler do here?), and a few start from working code with a defect.
- **Design problems.** A description of behaviour that does not say which feature to use, and no file to
  start from. The tests fix only how your code is called. Each has a reference solution with a commentary
  on the decisions in it and on the other good answers.
- **Break it on purpose.** As in the rebuild guide: a small program that makes a specific mistake, and the
  outcome that was observed, whether a compiler error, a wrong result or a sanitizer report.
- **Review sets.** Problems that do not say which chapter they belong to. Choosing the tool is the exercise.

Every chapter ends with "Back in logger", questions about why the logger's code is written the way it is,
and with a checkpoint.

## Layout

```
chapters/NN-slug/
  README.md        the chapter
  examples/        the programs the chapter quotes
  experiments/     "Break it on purpose", each with the outcome it must produce
  exercises/       the drills; you create the files of design problems here
  tests/           the tests of both
solutions/NN-slug/ reference solutions, and README.md with commentaries and answers
support/tut/       TUT_TODO and the small instrumented types the tests use
cmake/, tools/     the build functions, and the author-side checks
```

## How this was checked

`python3 tools/verify.py --pristine` runs the following on a clean export of the committed tree.

- Every reference solution compiles with the logger's warning set as errors, under GCC 16.2 and
  Clang 23.1, and passes its tests; also under AddressSanitizer with UndefinedBehaviorSanitizer; and is
  clean under clang-tidy with the logger's rules (one check is switched off; `.clang-tidy` says which and
  why).
- Every untouched exercise fails, for the intended reason: a stub says `NOT IMPLEMENTED` and passes no test
  case, a prediction table is wrong in every row, a fix-it fails its tests, and a design problem has no file.
  A test that an unwritten exercise can pass checks nothing, and this is how that is ruled out.
- Every example prints exactly what its chapter quotes.
- Every experiment ends the way its chapter says, in the build it says: the quoted compiler messages,
  outputs and sanitizer reports are files that the check compares with what really happens.
- Every code listing in a chapter is taken, by a tool, from a file that is compiled. Listings of logger
  code are compared with the logger repository at commit `49f42d1`, version 0.1.0, the one the rebuild
  guide is written against.
- Links and file references in the chapters resolve, and all C++ is clang-format clean.

**Not checked:** nothing here has been built with MSVC, with Apple Clang or against libc++, with a compiler
older than the two named, or on any system but Linux. The code keeps to the standard library, so it should
travel; the compiler messages quoted in the chapters are those of GCC 16 and Clang 23 and will read
differently elsewhere. ThreadSanitizer has nothing to check until the concurrency chapters exist.
