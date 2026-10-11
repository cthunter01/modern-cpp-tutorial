# modern-cpp-tutorial

A tutorial on the modern C++ that the `logger` library uses, with exercises. Companion to
`docs/rebuild-guide/` in the logger repository (expected next to this one, at `../logger`; quoted logger
code is pinned to its commit `49f42d1`). C++23, CMake presets + Ninja, GoogleTest. Checked on Linux with
GCC 16 and Clang 23 only.

The reader works on a branch; `main` holds the untouched exercises. Never commit a solved exercise to
`chapters/*/exercises/`, and never create a design-problem file (`pNN_*.h`) there.

## Commands
- First build, and the examples: `cmake --workflow --preset dev`
- One exercise (builds it, then runs it): `ctest --preset clang-debug -R ch05_d03`
- A chapter's exercises: `ctest --preset clang-debug -R ch05`
- A reference solution: `ctest --preset solutions -R sol05_d03`
- Other trees: presets `gcc-debug`, `asan`, `tsan`, `tidy` take the same commands
- **The author-side check of everything: `python3 tools/verify.py`** (working tree) or
  `python3 tools/verify.py --pristine` (clean export of `HEAD`; the one that counts). Run it before
  finishing any change; logs are in `build/verify/logs/`
- Listings: `python3 tools/docs.py update` rewrites every tagged listing from its source,
  `python3 tools/docs.py check` verifies listings, links and file references
- Formatting is automatic: `.claude/hooks/format-cpp.sh` runs clang-format after each edit of a C/C++ file

## Layout
- `chapters/NN-slug/`: `README.md` (the chapter), `CMakeLists.txt` (declarations only), `examples/`,
  `experiments/` (with `.expect` files), `exercises/` (stubs), `tests/`
- `solutions/NN-slug/`: reference solutions under the exercises' file names, and `README.md` with the
  commentaries and the answers to "Back in logger"
- `support/tut/`: `todo.h` (`TUT_TODO`), `tracked.h` (counts copies and moves), `allocation_count.h`,
  `storage.h`
- `cmake/TutExercises.cmake`: `tut_chapter`, `tut_example`, `tut_experiment`, `tut_drill`, `tut_problem`;
  `cmake/Expect.cmake`: the script behind every expected failure
- `tools/verify.py`, `tools/docs.py`
- `ROADMAP.md`: the agreed design of the chapters that are not written yet

## How the build works
- Only examples are in the default build. Every exercise, solution and experiment is its own target, built
  by the ctest test `<name>.build` that the run test `<name>` has as its fixture.
- Names: `ch05_d03` the reader's drill, `ch05_p01` the reader's design problem, `sol05_...` the reference,
  `stub05_...` the inverse check (the untouched exercise must fail), `eg05_01` an example, `x05_02` an
  experiment. Chapter ids are two characters: `00` to `15`, and `ra`, `rb`, `rc` for the review sets.
- The same test source is compiled twice, with `exercises/` or `solutions/NN-slug/` as the only include
  directory. Tests live in `tests/`, never next to a stub.
- A header given to the reader (no `dNN_`/`pNN_` prefix) exists in both trees and must be identical.

## Writing a chapter
Order of work: examples and experiments (run them, record what happens) → tests and reference solutions →
stubs → prose → `docs.py update` → `verify.py`.

- **Prose:** the voice of the rebuild guide. Second person, British spelling, about 112 columns, no emojis.
  Sections: Goal, Where you met it, The idea, The details that bite, When to reach for it, Drills, Design
  problems, Break it on purpose, Back in logger, (Take it to your rebuild,) Checkpoint.
- **Every `cpp` block is tagged** with its source, `<!-- file: path#region -->` or
  `<!-- logger: path:first-last -->`, and filled by `docs.py update`; regions are delimited by `//! [name]`
  lines. A block that is deliberately not compiled says so: `<!-- not-compiled: why -->`.
- **Nothing observed is written from memory.** An experiment's outcome is its `.expect` file (literal
  fragments, in order); the chapter quotes that file. A claim about what the logger does when changed is
  tried on a throw-away copy first.
- **Stubs** keep the final signature, `noexcept` included, with `TUT_TODO(all, parameters);` as the body.
  Every test case must fail on the stub. Variants: `FIXIT` (starts from wrong code), `STUB_SAYS` (a table
  of predictions), `COMPILE_TIME` (the stub is meant not to compile), `COMPILES`/`REJECTS` (extra files that
  must or must not compile; the rejected lines go under `#ifdef TUT_BREAK`).
- **Design problems** state behaviour and never name the feature. Their tests fix call expressions and
  observable properties, and begin with a comment saying exactly what they fix.
- **Code:** logger's naming (functions `lower_case`, methods `CamelCase`, members `camelBack`, constants
  `kCamelCase`), `namespace chNN`, headers `.h` with `#pragma once`, everything header-only.

## Things that cost time once
- A `NOLINT` comment must fit on its line: `.clang-format` has `CommentPragmas: 'NOLINT'` so that it is
  never wrapped, but a trailing one can still push the code onto two lines. Prefer `NOLINTNEXTLINE` on a
  line of its own.
- clang-tidy's `bugprone-unchecked-optional-access` does not understand `ASSERT_TRUE(opt)`. Tests compare
  the optional (`EXPECT_EQ(opt, value)`) or leave early with `if (!opt) { FAIL(); }`.
- clang-tidy's `InnerPointer` analysis flags comparing `.data()` pointers across a move. Tests use
  `tut::storage_of(text)`.
- A designated initialiser must name every member that has no default member initialiser (`-Wextra`).
- libstdc++ 16 checks its preconditions in every unoptimised build; `#define _GLIBCXX_NO_ASSERTIONS` before
  the first include turns that off for one experiment file.
- A program that crashes loses what it had written to a buffered `stdout`. Expectations for crashing
  experiments list only what the crash report says.
- `gtest_discover_tests` is not used on purpose: it drops tests whose program does not build yet.
