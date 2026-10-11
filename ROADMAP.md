# Roadmap

Chapters 0 to 5 and Review A are written. This file is the agreed design for the rest: ten more chapters,
two more review sets and two capstones. It was drawn up together with the first six chapters, so that the
later ones reuse what the earlier ones taught, and it is kept here so that they do not have to be designed
again. Nothing below exists yet.

It is a plan, not a promise. The first chapters are being worked through now, and what that shows about
difficulty, exercise size and how much the text assumes will change what is written here.

| # | Chapter | Covers | Unlocks in the logger |
| --- | --- | --- | --- |
| 6 | `constexpr`, `consteval`, `if constexpr`, `static_assert` | compile-time evaluation | `Level.h`, `logger.h:117-128` |
| 7 | `std::format` and the spec language | formatting, `print`, `vformat` | the format strings in `Record.cpp` |
| 8 | `format_to`, output iterators, writing a formatter | formatting into a buffer | `Record.cpp:42-55`, the sinks' `line` |
| 9 | `format_string`, compile-time checking, `source_location` | the logger's `FormatString` | `FormatString.h`, `logger.h:94-180` |
| rb | Review B | | |
| 10 | `chrono` | durations, clocks, calendar | `Record.cpp:18-31` |
| 11 | `filesystem` | paths, operations, errors | `FileSink` |
| 12 | Ranges II: views and pipelines | lazy views, `ranges::to`, range concepts | none: the logger has no views |
| rc | Review C | | |
| 13 | Mutex and `condition_variable`: the primitive | waiting, notifying, predicates | `GateSink`, every wait in `Core.cpp` |
| 14 | Lifecycles: shutdown, timed waits, `stop_token`, atomics | stopping a thread | `Core::Stop`, `State` |
| 15 | Capstones | a log analyser and a timer queue | |

## How to read the inventories

`sv` is `std::string_view`, `str` is `std::string`, and `opt<T>`, `vec<T>`, `span<T>` are the `std` types.
`[n]` marks an exercise that deliberately reuses chapter n. Test kinds: **RT** a run-time GoogleTest case,
**SA** a `static_assert`, **CF** a file that must not compile, **MC** a file that must compile. Checks of
design problems: **TXT** text in and text out; **CALL** the tests fix call expressions and use results
generically; **PROP** a property seen through an instrumented type (copy counts, addresses, allocation
counts); **REF** a commented reference solution only. Paths are relative to the logger repository. In the
finished chapters, design problems are numbered like their files (`p01`, `p02`, ...); here they still carry
the letters of the planning stage (6.A is chapter 6's first).

## Conventions for these chapters

In addition to what `CLAUDE.md` says for all chapters:

- **Support code to add:** `TempDir` (per-test directory, modelled on `tests/FileSinkTests.cpp:25-39`),
  `FakeClock` (settable static `now()`), `spin_until(pred)` (a yield loop with a watchdog that fails the test
  instead of hanging).
- **Timed-wait seam.** Classes with timed waits are templates on `Clock` and expose `void Poke()`, which
  wakes all waiters without changing state. Tests advance `FakeClock`, then `Poke()`. A fake clock alone is
  not enough: libstdc++'s `wait_until` for a custom clock sleeps the remaining delta in real time. Fake
  deltas stay at 50 ms or more.
- **Blocking seam.** Blocking classes expose `Waiting...()` counters, incremented under the mutex just before
  `wait`. No test sleeps as synchronisation; this is stricter than the guide's own tests, on purpose.
- **Concurrency tests never fail a correct solution.** A wrong one may pass a run; TSan and
  `--gtest_repeat` mitigate.
- **Names the standard library dictates.** `.clang-tidy` already exempts `parse`, `format` and `now` as
  member functions and `rep`, `period`, `duration`, `time_point` and `difference_type` as type aliases.
  `is_steady` in `FakeClock` is a constant and will need a `NOLINT`.
- **Deliberate reuse.** From chapter 6 on, at least two drills per chapter require a feature from three or
  more chapters back, enforced by tests where possible (aliasing checks force views, copy counts force
  moves).
- **To confirm by running before writing:** the effect of C++23 immediate escalation (P2564) on 6.X2; 9.X3;
  TSan's report for 13.X3; the exact `{:?}` text in 7.5; the sub-second digit counts in 10.X2 on both
  compilers; the C++26 name of the run-time format helper (libstdc++ 16 spells it `std::dynamic_format`).

### Chapter 6. `constexpr`, `consteval`, `if constexpr`, `static_assert` (4 h)

**Model.** A `constexpr` function may run at compile time when the context demands a constant. A `constexpr`
variable must be initialised at compile time. A `consteval` function must be evaluated at compile time at
every call, so a failed check inside is a build error. `if constexpr` selects code inside a template.
`static_assert` is a unit test that runs in the compiler. Constant evaluation has no undefined behaviour.

**Misconceptions.** M1 "a `constexpr` function runs at compile time". M2 "`constexpr` equals `const`; header
constants need no `inline`". M3 "`if constexpr` is a faster `if`" (it cannot test a function parameter).
M4 "`consteval` is a stronger `constexpr` I can call with anything". M5 "`constexpr` bodies are one `return`".

**Neighbours.** inline variables, `if constexpr`, `constexpr` lambdas (17); `consteval`, `constinit`,
`std::is_constant_evaluated`, `constexpr` algorithms (20); `if consteval` (23, short); enum non-type template
parameters; `std::array` CTAD (17).

**Anchors.** `include/logger/Level.h:49-50,56,59-62,67-85`; `Record.h:25-29`; `src/Level.cpp:14-17,23-25`;
`logger.h:26,117-128`; `FormatString.h:30-36`; `tests/RecordTests.cpp:42-47`; `logger_tests.cpp:182-183`;
`min_level_tests.cpp:48-52`.

**Back in logger.** (1) `log()` uses a plain `if` (`logger.h:146`), `log_at` uses `if constexpr` (`:121`):
what had to change in the signature? (2) `to_lower_ascii` is `constexpr` yet only called at run time;
`file_basename` is called at compile time, where? What does the keyword buy in each case? (3) What can go
wrong if `kDefaultQueueCapacity` is `const` or non-inline `constexpr` in a header?

Drills:
- 6.1 `constexpr bool is_power_of_two(std::uint64_t)`; `constexpr std::uint64_t ipow(std::uint64_t base, unsigned exp)`. SA plus one run-time call.
- 6.2 `constexpr sv level_tag(Severity)`; `constexpr opt<Severity> parse_severity(sv)` over a `constexpr std::array` [1, 2, 3]. SA round trip for every enumerator.
- 6.3 `inline constexpr auto kCumulativeDays = make_cumulative_days();`. SA on entries.
- 6.4 `consteval std::uint8_t percent(int value)`. SA; CF `percent(101)`; CF a run-time argument.
- 6.5 `consteval std::uint32_t rgb(sv text)` for `"#ff8800"` [1]. SA; CF `"#12345"`; CF `"#gg0000"`.
- 6.6 `template <typename T> str describe(const T& value)` with `if constexpr` [5]. Each category, `bool` first; a type with no `to_string` still compiles.
- 6.7 `template <Check C, typename F> void check(F&& condition, vec<str>& failures)` stripped by `if constexpr (C <= kCompiledChecks)`. A stripped level never invokes the callable.
- 6.8 `template <std::size_t N> constexpr bool all_unique(std::array<sv, N> names)`. SA on a command table.

Design problems:
- 6.A Prices are integer cents. A malformed price literal must fail the build; prices typed at run time report failure to the caller. (One `constexpr` core, a `consteval` door, an `optional` door.) SA + CF + RT.
- 6.B `to_bytes(value)` for integers, enums, strings and containers of those, recursively; unsupported types rejected at compile time with a readable message. RT + CF.
- 6.C Three optional subsystems chosen at build time; calls into a disabled one cost nothing, yet typos are still caught where possible. (`inline constexpr` flags with `if constexpr` versus `#if`; the ODR trap of guide step 13.) REF.

Experiments:
- 6.X1 `constexpr int f(int x) { return 100 / x; }` in a `static_assert`, then at run time: compile error; then a UBSan report or crash.
- 6.X2 Non-template `constexpr int twice(int v) { return 2 * percent(v); }`; then make it a template: compile error; then observe and record.
- 6.X3 `if constexpr (level <= kCompiled)` on a function parameter: compile error; fixed by a template parameter.

### Chapter 7. `std::format` and the spec language (3.5 h)

**Model.** `{}` placeholders with an optional spec `{:[[fill]align][sign][#][0][width][.precision][type]}`,
interpreted by the argument type's formatter. The format string is checked at compile time. No locale unless
`L`. Width and precision may come from arguments.

**Misconceptions.** M1 "`printf` with braces: I must state the type" (default alignment also differs by
type). M2 "`{:05}` and `{:0>5}` are the same" (precision truncates strings and is an error on integers).
M3 "any string can be the format string". M4 "compile-time checking means it cannot throw". M5 "`{}` behaves
like `<<`" (`std::uint8_t` prints as a number; `int*` and enums are not formattable).

**Neighbours.** `format`, `vformat`, `make_format_args` (20), `format_error`; `print`, `println` (23); range
formatting and `{:?}` (23). `std::formatter<std::filesystem::path>` is C++26, which is why `FileSink.cpp:19`
formats `path.string()`.

**Anchors.** `src/Record.cpp:26-30,45`; `main.cpp:102`; `tests/FileSinkTests.cpp:30`; `Core.cpp:132-135`;
`FileSink.cpp:18-19`; `logger.h:98-111`.

**Back in logger.** (1) What is a `string_view`'s default alignment; is the `<` in `[{:<5}]` needed?
(2) Why does `FileSink.cpp:19` format `path.string()` and not `path`? (3) The format string was checked at
compile time, yet `logger.h:98-111` has a `catch`. Name two things that can still throw.

Drills:
- 7.1 Fill `constexpr std::array<sv, 12> kSpecs` to produce twelve given outputs; each checked via `vformat`.
- 7.2 `str hex_line(std::size_t offset, span<const std::uint8_t> bytes)` [1]. `00000010  de ad be ef`; empty input.
- 7.3 `str row(sv name, double value, std::size_t nameWidth, int precision)`. Nested `{:<{}}` and `{:>10.{}f}`.
- 7.4 `str hms(std::int64_t totalSeconds)`. `01:02:03`, a negative value, over 99 hours.
- 7.5 `str json_pair(sv key, int value)`; `str describe(sv key, sv value)` with `{:?}`. `{{`; embedded quote and newline escaped.
- 7.6 `opt<str> render(sv userTemplate, sv name, int count)` [2]. Reordered indices; bad templates give nothing.
- 7.7 `void print_report(std::ostream& out, span<const Item> items)`. Exact text through an `ostringstream`.
- 7.8 `str format_entry(const Entry& e)` giving `[WARN ] net.cpp:42 retry 3/5 (60.0%)` [1, 6].

Design problems:
- 7.A Print rows of strings as a table: columns as wide as their widest cell, numeric-looking cells right-aligned, a rule under the header. TXT.
- 7.B `human_size(bytes)` and `human_duration(ns)` with one decimal and sensible units, including the 999.95 boundary. CALL.
- 7.C Messages come from a translation file loaded at run time. Translators may reorder or drop placeholders but must not be able to crash the program. CALL.

Experiments:
- 7.X1 `std::format("{:d}", "text")` and `std::format("{} {}", 1)`: compile errors; find the line that names the reason.
- 7.X2 `str fmt = "{} items"; std::format(fmt, 3);` then `vformat` with `"{} {}"`: compile error; then a run-time `format_error`.
- 7.X3 Use user input `"100% {sure}"` as the format string: exception; `"{{}}"` is silently altered.

### Chapter 8. `format_to`, output iterators, writing a formatter (4 h)

**Model.** `format_to` writes through an output iterator and returns the advanced iterator. `back_inserter`
turns assignment into `push_back`; a raw pointer writes unchecked; `format_to_n` bounds it. Appending into a
caller-owned, reused buffer allocates nothing once warm. A formatter has two halves: `constexpr parse` reads
the spec and stops at `}`; `format` writes via `format_to(ctx.out(), ...)`.

**Misconceptions.** M1 "`format_to` into a string overwrites or needs pre-sizing" (`format_to(s.begin(), ...)`
on an empty string is UB). M2 "`format_to` to a buffer is bounds-checked like `snprintf`" (neither writes a
NUL; `format_to_n`'s `size` is the untruncated length). M3 "the returned iterator can be ignored". M4 "`parse`
sees the whole string and only runs at run time". M5 "I must implement padding myself" (inherit from
`formatter<string_view>`; `format` must be `const`).

**Neighbours.** `format_to`, `format_to_n`, `formatted_size`, `formatter`, parse and format contexts (20);
`back_inserter`, `ostreambuf_iterator` (98); `std::output_iterator` (20); `std::formattable` (23);
`resize_and_overwrite` (23, one paragraph).

**Anchors.** `src/Record.cpp:18-31,42-55`; `include/logger/Record.h:35-43`; `FileSink.h:33`;
`FileSink.cpp:25-27`; `ConsoleSink.cpp:14-16`; `tests/min_level_tests.cpp:29-41`; `logger_tests.cpp:43-54`;
`RecordTests.cpp:86-98`.

**Back in logger.** (1) Why does the library expose both `format_line_to` and `format_line`? What does
`FileSink::Write` do first, and why does that free no memory? (2) Why `std::string&` plus `back_inserter`
rather than a template over the output iterator? (3) The test formatters return `ctx.begin()` from `parse`.
What does that promise about `{:>10}` on a `Probe`? Why are the two functions lower case in a CamelCase code
base?

Drills:
- 8.1 `void append_csv(str& out, span<const int> values)`. Prefix kept; no reallocation when capacity suffices.
- 8.2 `template <std::output_iterator<char> Out> Out write_point(Out out, const Point& p)`. String, `std::array`, `ostreambuf_iterator`; returned iterator distance.
- 8.3 `sv format_id(span<char> buffer, unsigned id) noexcept` [1]. Exact fit; truncation without overflow; empty buffer.
- 8.4 `str join_lines(span<const Entry> entries)` with one allocation. `AllocationCount` at most 1.
- 8.5 `class LineWriter { explicit LineWriter(std::ostream& out); void Write(const Entry& e); }`. Text; zero allocations over 1000 writes after warm-up.
- 8.6 `std::formatter<Point>` with no spec. `{}`; inside a `vec<Point>`; SA `std::formattable`.
- 8.7 `std::formatter<Severity>` inheriting `std::formatter<sv>` [6]. `[{:<5}]`, `{:*^9}`.
- 8.8 `std::formatter<Rgb>`: `{}` gives `rgb(255, 136, 0)`, `{:x}` gives `#ff8800`; any other spec throws in `parse`. RT; CF `std::format("{:q}", c)`; `vformat` throws.

Design problems:
- 8.A Keep the most recent 64 KiB of diagnostic lines in a fixed block inside an object, with no heap after construction. When full, whole oldest lines go. TXT + PROP.
- 8.B Make a `Duration` struct printable compactly or in a fixed unit, at a width chosen at the call site. A misspelt style is a build error. (Stretch.) RT + CF.
- 8.C One `append_report` serves a caller building a string, one filling a `vec<char>`, and one that only wants the length. CALL.

Experiments:
- 8.X1 `str s; std::format_to(s.begin(), "{}", 12345);`: empty string, no report; ASan overflow once output exceeds 15 characters.
- 8.X2 `format_to` a 12-digit number into `std::array<char, 8>`; then `format_to_n`, then `std::puts`: ASan overflow; `size` is 12; no NUL written.
- 8.X3 Return `ctx.end()` from `parse`; then drop `const` from `format`: compile errors inside `<format>` to read.

Take it to your rebuild: delete `line.clear()` in `FileSink::Write` and run `FileSinkTests`.

### Chapter 9. `format_string`, compile-time checking, `source_location` wrappers (5 h)

**Model.** `std::format_string<Args...>` is a small class whose `consteval` converting constructor parses the
literal against `Args...`; the check is a side effect of an implicit conversion at the call site. A wrapper
takes `format_string<Args...>` and passes the object on. Anything in a default argument or a call-site
conversion belongs to the caller, which is how `source_location` is captured once, at the outermost call.

**Misconceptions.** M1 "my wrapper can take a `string_view` and call `std::format`". M2 "`Args` are decayed
value types" (they are `int&`, `const std::string&`, ...). M3 "deduction will work `Args` out from the
literal". M4 "`source_location::current()` gives the log line" (it gives where it is evaluated). M5 "the
check costs run time".

**Neighbours.** `format_string` and `.get()` (20/23); `type_identity_t`, `source_location` (20); immediate
functions (20; escalation 23); `vformat` back ends; CTAD deduction guides as the alternative capture trick
(one box).

**Anchors.** `include/logger/FormatString.h:22-36`; `logger.h:94-113,117-128,143-180`;
`tests/FormatStringTests.cpp`; `logger_tests.cpp:195-199`; guide step 4.

**Back in logger.** (1) `std::format_string` is an alias that already contains `type_identity_t`;
`FormatString` is a plain struct. Where did logger therefore have to write `type_identity_t` by hand?
(2) Which line ends up in a record made through `audit()` (`logger_tests.cpp:195-199`)? What happens if
`audit` takes `std::format_string<Args...>` instead? (3) `fmt.format` is not a constant expression at
`logger.h:100`. Why does `std::format(fmt.format, ...)` compile when `std::format(someView, ...)` does not?
(4) Which token in `logger.h` makes a stripped call still check its format string?

Drills:
- 9.1 `str tagged(sv tag, std::format_string<Args...> fmt, Args&&... args)`. RT; CF placeholder mismatch; CF run-time string.
- 9.2 `void append_fmt(str& out, std::format_string<Args...> fmt, Args&&... args)` [8]. Appends; lvalues intact.
- 9.3 `void vlog(vec<str>& sink, sv fmt, std::format_args args)` plus a thin `logf` template. Output; the lvalue rule of `make_format_args`.
- 9.4 `struct Located { sv text; std::source_location location; }` with a converting constructor; `void note(vec<Note>& notes, Located message)`. Recorded line is the caller's.
- 9.5 `CheckedFormat`: rebuild logger's `FormatString` from a blank struct. The tests of `FormatStringTests.cpp`; CF ×2.
- 9.6 `void trace(Trace& t, CheckedFormat<std::type_identity_t<Args>...> fmt, Args&&... args)` (stub omits `type_identity_t`). MC; line is the caller's.
- 9.7 `trace_if`, a wrapper over `trace`. Line is the `trace_if` call, not a header line; no copies.
- 9.8 `template <Severity S, typename... Args> void trace_at(...)` with `if constexpr` [5, 6, 8]. A counting `Probe` is never formatted when stripped; CF bad string in a stripped call.

Design problems:
- 9.A `ensure(cond, "index {} out of range [0, {})", i, n)` throws `File.cpp:123: ...` when false and formats nothing when true. A wrong format string must not compile. No macros. RT + CF.
- 9.B `tr("Hello {}", name)`: English literals are checked at compile time; translations loaded at run time are validated against the same argument types and rejected if they do not fit. REF, partly RT.
- 9.C `return fail("cannot open {}: {}", path, reason);` works in any function returning `std::expected<T, Error>` and records where the error was created [2]. CALL + CF.

Experiments:
- 9.X1 Move the `source_location` default argument from the constructor to `trace`, before and after the pack: compile errors.
- 9.X2 Call `source_location::current()` in the member initialiser instead of the default argument: recorded line is inside the header.
- 9.X3 Make the constructor `explicit`; then make it a non-template taking `std::string_view`: both break `trace(t, "x")`; the second needs two user-defined conversions.

### Review B (after chapter 9)

1. Duration literals like `"1h30m"`: checked at build time for literals, at run time for user input.
2. A hexdump into any character destination with no allocation per line.
3. `expect_eq(a, b)` that reports `file:line` and both values, printing `<unprintable>` for types without a
   formatter. (A plain default argument suffices: it tests knowing when the heavy machinery is unnecessary.)
4. A table printer taking (header, member pointer) column pairs.
5. One `constexpr` table per enum giving `to_string`, `from_string`, a formatter and a coverage
   `static_assert`.

Plus a fix-it set from chapters 6-9's experiments and the clinic "compile time or run time?".

### Chapter 10. `chrono` (4.5 h)

**Model.** Durations carry their unit in the type; lossy conversions must be asked for. Time points are
durations since a clock's epoch, and clocks do not mix. `system_clock` is wall time; `steady_clock` is for
intervals and deadlines. The calendar types are named integers until converted to `sys_days`. Testable code
takes time points and clocks as parameters.

**Misconceptions.** M1 "a duration is a number of milliseconds" (`.count()` belongs at the I/O boundary).
M2 "`duration_cast` rounds; `floor` is the same" (they differ for negative values). M3 "`system_clock` is fine
for measuring". M4 "dates validate themselves; adding `months` is calendar arithmetic". M5 "time formatting
needs `strftime`; `system_clock` is local time" (`%S` prints the duration's full precision).

**Neighbours.** `duration`, `time_point`, clocks (11); literals (14); `floor`/`ceil`/`round` (17);
`year_month_day`, `sys_days`, `sys_time`, `hh_mm_ss`, `weekday`, `file_clock`, `clock_cast`, `zoned_time`,
chrono format specs, `chrono::parse` (20). `zoned_time` is stretch only; its tests skip without a tzdb.

**Anchors.** `include/logger/Record.h:18`; `src/Record.cpp:16-31`; `main.cpp:113-116`; `Core.cpp:283,331,379`;
`tests/RecordTests.cpp:18-38`; guide steps 2 and 8.

**Back in logger.** (1) Records carry `system_clock` time points; the demo times its sort with
`steady_clock`. What breaks if they are swapped? (2) `Record.cpp:21-25` uses `floor` twice. What would
`time_point_cast` print for 1969-12-31T23:59:59.999Z? (3) Write the portable one-liner that step 2 mentions
and check it against `RecordTests.cpp`.

Drills:
- 10.1 `constexpr milliseconds total(hours, minutes, seconds)`; `whole_seconds(milliseconds)` (floor); `nearest_second(milliseconds)` (round). SA, including -1500 ms.
- 10.2 `constexpr Hms split(milliseconds d) noexcept` via `hh_mm_ss`. SA.
- 10.3 `constexpr opt<sys_time<milliseconds>> make_utc(int y, unsigned mo, unsigned d, int h, int mi, int s, int ms)` [2]. Epoch; the instant of `RecordTests.cpp:18-23`; Feb 30 and 2023-02-29 rejected.
- 10.4 `str format_iso(system_clock::time_point tp)` with chrono specs; `void append_iso(str& out, ...)` [8]. Logger's `RecordTests` cases plus a pre-1970 instant.
- 10.5 `opt<sys_time<milliseconds>> parse_iso(sv text) noexcept` [1, 2]. Round trip with 10.4; wrong length, separators, invalid dates. (The inverse of logger's `format_timestamp`.)
- 10.6 `template <typename Clock = steady_clock> class Stopwatch { void Restart(); typename Clock::duration Elapsed() const; }`. `FakeClock` advance; a backwards step yields a negative elapsed time.
- 10.7 `constexpr year_month_day last_weekday_of_month(year, month, weekday)`; `constexpr sys_days add_months_clamped(sys_days, int)`. SA; Jan 31 plus one month.
- 10.8 `constexpr sys_time<minutes> minute_bucket(system_clock::time_point tp, minutes width)`. SA; pre-1970.
- 10.9 (stretch) `opt<str> local_wall_time(sys_seconds tp, sv zone)`. Berlin across a DST change; unknown zone; skipped without tzdb.

Design problems:
- 10.A Allow at most N events per rolling window. Testable without waiting, and immune to wall-clock adjustment. CALL.
- 10.B Print `up 3 days, 04:05:06`. For a calendar month and a list of outage intervals, print availability to two decimals. TXT.
- 10.C "Every Monday 09:00 New York time": the next four occurrences as UTC and as Tokyo wall time, correct across the DST change. (Stretch.) TXT.

Experiments:
- 10.X1 Replace both `floor`s by casts in `format_iso`; run the pre-1970 test; repeat in the rebuild's `append_timestamp`: wrong date and milliseconds.
- 10.X2 Format `sys_seconds`, `sys_time<milliseconds>` and `system_clock::time_point` with `{:%T}`: predict the digit counts.
- 10.X3 `2026y/1/31 + months{1}`, its `sys_days`, and `sys_days{2026y/1/31} + months{1}`: Feb 31 (not `ok()`); March 3; compiles and adds 30.44 days.

### Chapter 11. `filesystem` (3.5 h)

**Model.** `path` is a value type for names: pure string work that knows about separators. The free functions
touch a shared, changing file system and can each fail; each has a throwing form and an `error_code` form.
Check-then-act is a race, so act and handle the failure. Iteration order is unspecified.

**Misconceptions.** M1 "a path is a string" (an absolute right-hand side of `/` replaces the left; `+=` adds
no separator). M2 "`exists` then open is the safe way". M3 "wrap everything in `try`" or "ignore errors"
(choose per call; `noexcept` code needs the `error_code` forms). M4 "directory iteration is sorted, recursive
and stable under modification". M5 "`rename` fails if the target exists; `file_size` reflects what I just
wrote".

**Neighbours.** `path`, the operations, iterators, `directory_entry`, `filesystem_error` (17);
`std::error_code`, `std::errc` (11, short section); `file_clock`, `clock_cast` (20); streams taking `path`
(17); `ranges::to` (23).

**Anchors.** `include/logger/FileSink.h:13-34`; `src/FileSink.cpp:13-21`; `main.cpp:88-90`;
`tests/FileSinkTests.cpp:25-39,92-96`; `docs/rebuild-guide/14-wrap-up.md:563-564`.

**Back in logger.** (1) `TearDown` passes an `error_code` it never reads; `SetUp` does not. Why the
asymmetry? (2) Argue for and against `create_directories(filePath.parent_path())` in `FileSink`'s
constructor. Which test changes? (3) What two collisions does the directory name at
`FileSinkTests.cpp:29-30` prevent?

Drills:
- 11.1 `path rotated_name(const path& p, std::size_t n)`; `opt<path> resolve_under(const path& root, sv userSupplied)` [2]. Pure; `app.3.log`; absolute and `..` inputs rejected.
- 11.2 `void write_text(const path& file, sv text)`: create parents, write a temporary, rename. Nested directories; replacement; no temporary left; parent is a file.
- 11.3 `std::expected<str, std::error_code> read_text(const path& file)` [2]. Missing; is a directory; content.
- 11.4 `vec<path> list_files(const path& dir, sv extension)` sorted by filename [3]. Scrambled creation order; a directory named `x.log` excluded; missing directory.
- 11.5 `std::size_t remove_older_than(const path& dir, file_time_type cutoff)` [10]. Times set explicitly with `last_write_time`.
- 11.6 `void rotate(const path& file, std::size_t keep)`. Each generation's content after 1, 2 and `keep + 1` rotations.

Design problems:
- 11.A For a tree, print the 10 largest files and totals per top-level directory. Unreadable entries are counted, not fatal. TXT.
- 11.B Saving a document must leave either the complete old or the complete new file, keep `name.bak`, and leave no temporaries on failure [4]. RT + REF.
- 11.C Find groups of identical files cheaply; most files differ in size. CALL.

Experiments:
- 11.X1 `path("/var/log") / "/etc/passwd"`; `path("logs") += "app.log"`: `/etc/passwd`; `logsapp.log`.
- 11.X2 `file_size` of a file behind an open, unflushed `ofstream`: 0; so a rotating sink counts bytes itself.
- 11.X3 `file_size("missing")` throwing form, `error_code` form, then inside a `noexcept` function: exception text; `ec.message()`; `std::terminate`.

### Chapter 12. Ranges II: views and pipelines (4.5 h)

**Model.** A view is a lazy, cheap-to-copy description of a computation over a range it borrows; nothing runs
until iteration, and each element is pulled through the whole pipeline. Lifetime rules are chapter 1's.
`ranges::to` produces a container. Generic functions accept any range of T through concepts.

**Misconceptions.** M1 "a pipeline computes when written" (lambdas run at iteration, sometimes more than once
per element). M2 "a view keeps its source alive". M3 "views are const-iterable like containers" (`filter`,
`chunk_by` and others are not). M4 "`views::split` yields strings". M5 "any-range parameters mean
`const std::vector<T>&`".

**Neighbours.** `filter`, `transform`, `take`, `drop`, `reverse`, `iota`, `keys`, `values`, `split`, `join`
(20); `zip`, `enumerate`, `chunk`, `chunk_by`, `slide`, `pairwise_transform`, `join_with`, `as_rvalue`,
`ranges::to` (23); `input_range`, `range_value_t`, `viewable_range` (20); `std::generator` (23, one
paragraph).

**Anchors.** Logger has no views; the chapter says so and uses the loops that could have been pipelines:
`src/Level.cpp:26-32`, `Core.cpp:467-488`, `main.cpp:107-112`.

**Back in logger.** (1) Rewrite the loop in `parse_level` as one `ranges::find_if`. Is it clearer?
(2) Sketch `WriteBatch` with `views::cartesian_product`. Does record-major order survive? Why might the loops
with a per-call `try` still be the better code? (3) Write `main.cpp:107-112` as `iota`, `transform`, `to`.
How many allocations?

Drills:
- 12.1 `vec<int> squares_of_evens(span<const int> values)`; `auto lazy_squares(span<const int> values, int& calls)`. Values; `calls == 0` before iteration; `take(2)` stops early.
- 12.2 `vec<sv> fields(sv line, char delim)` via `views::split` [1]. Chapter 1's `split` tests.
- 12.3 `std::map<str, int, std::less<>> parse_counts(sv text)` for `a=1,b=2,bad,c=x` [2]. Malformed pairs dropped.
- 12.4 `vec<double> moving_average(span<const double> values, std::size_t window)`; `vec<milliseconds> gaps(span<const TimePoint> stamps)` [10]. Window 0 and window above size give empty.
- 12.5 `vec<std::pair<Severity, std::size_t>> run_lengths(span<const Severity> levels)`; `vec<vec<int>> pages(span<const int> items, std::size_t pageSize)`.
- 12.6 `template <std::ranges::input_range R> requires ... constexpr auto total(R&& values)`. Vector, array, list, `iota`, a filter view (MC); SA strings rejected.
- 12.7 `auto errors_only(const vec<Entry>& entries)`; `auto matching(const vec<Entry>& entries, str needle)`. Lazy: sees later changes; needle captured by value (ASan).
- 12.8 `vec<std::pair<str, std::size_t>> top_words(sv text, std::size_t n)` [3]. Counts; tie order.

Design problems:
- 12.A From a huge in-memory sequence of lines, return the last N containing a word (ASCII case-insensitive), in original order, without copying lines or scanning more than needed. CALL.
- 12.B Print a histogram: one line per non-empty bucket, bars scaled so the longest is 40 characters. TXT.
- 12.C `print_rows(out, rows, pageSize)` works for a vector, a list, a filtered subset and generated rows. The wrong element type is rejected readably. CALL + SA.

Experiments:
- 12.X1 Return `names | filter(...)` over a local vector; versus `load() | filter(...)`: ASan use-after-free; the second is safe (owning view).
- 12.X2 Count calls of a `transform` lambda placed before a `filter`: more calls than elements.
- 12.X3 `void show(const auto& r)` called with a filter view: compile error; fixed by `auto&&`.

### Review C (after chapter 12)

1. Sessionise timestamped events per user with 30-minute gaps.
2. A backup retention policy as a pure decision over file names and "today", applied separately.
3. Streamed CSV column statistics with malformed cells counted.
4. A directory-tree diff by relative path.
5. A week agenda grouped by day with aligned columns.

Plus a fix-it set from chapters 10-12's experiments and the clinic "pipeline, algorithm or loop?". There is
no review set for concurrency: an unlabelled concurrency problem is still obviously concurrency, and the
second capstone does that job.

### Chapter 13. Mutex and `condition_variable`: the primitive (6 h)

**Model.** Shared state, one mutex guarding it, an invariant. A condition variable holds no state: it is a
waiting room for a predicate over the guarded state. Waiting releases the mutex and sleeps atomically; waking
re-acquires and re-checks. No wake-up is lost if every change that can make a predicate true happens under
the mutex and is followed by a notify. One condition variable per reason to wait.

**Misconceptions.** M1 "a notify is remembered". M2 "`wait` returns only when notified, so an `if` suffices".
M3 "an atomic flag set without the mutex is fine". M4 "`notify_one` is always enough" or "`notify_all` is
always free". M5 "holding the lock while working is the safe choice" (never run unknown code under it).

**Neighbours.** `thread`, `mutex`, `unique_lock`, `condition_variable`, `call_once` (11); `scoped_lock` (17);
`jthread` as auto-join only, `latch`, `barrier`, semaphores (20); TSan.

**Anchors.** `src/Core.h:93-112`; `Core.cpp:163-165,265-296,423-458,562-569,594-609,678-682`;
`tests/logger_tests.cpp:102-142`; guide steps 7 to 9.

**Back in logger.** (1) Could `GateSink` drop its two bools and just `wait(lock)`? Construct the interleaving
that hangs the test. (2) `Core::Log` unlocks before `notify_one` (`Core.cpp:286-287`); `RunOnce` notifies
while holding the mutex (`:442,457`). Both are correct. What does the first save? (3) `Flush` waits for
`written >= target` with a snapshot (`:565-569`). Why not `written == enqueued`? (4) After the wait at
`:290-292` the code does `continue` instead of pushing. Why?

Drills (all also under TSan):
- 13.1 `class Gate { void Open(); void Close(); void Wait(); std::size_t Waiting() const; }`. Open-before-wait returns; a waiter is observed blocked then released; `Open` wakes four.
- 13.2 `class Latch { explicit Latch(std::ptrdiff_t n); void CountDown(); void Wait(); }`. Released exactly after the nth; then swap in `std::latch`.
- 13.3 `template <typename T> class Handoff { void Put(T v); T Take(); }` over `opt<T>` [2, 4]. 10 000 values in order; move-only `T`; zero copies.
- 13.4 `BoundedQueue`: `Push(T)`, `T Pop()`, `opt<T> TryPop()`, `WaitingProducers()`, `WaitingConsumers()`. Producer observed blocked at capacity; 4×4×5000 with per-producer order.
- 13.5 `vec<T> PopAll()` by swap; `void SetCapacity(std::size_t)`. Step 8's raise-the-capacity test without the sleep.
- 13.6 `WorkCounter`: `std::uint64_t Submit(); void Complete(std::size_t n); void WaitUntilDone();`. Returns while another thread keeps submitting.
- 13.7 `Subscribers`: `std::size_t Add(Callback); void Remove(std::size_t id); void Publish(int v);`. A callback that calls `Add`, `Remove` or `Publish` does not deadlock.
- 13.8 `Sequencer`: `void WaitForTurn(std::size_t index); void Done();`. Eight threads run in index order, 200 times.

Design problems:
- 13.A `Get(key)`: the first asker computes (slowly); others asking for the same key wait for that result; other keys do not wait. PROP (compute count per key is 1).
- 13.B `Get()` returns the current settings cheaply; `Update` replaces them; `WaitForChange(lastSeen)` blocks until a newer version exists. No half-updated reads; slow readers do not block updaters. RT + TSan.
- 13.C A pool of N connections: `Acquire()` blocks until one is free and returns something that gives it back automatically, even on an exception, and cannot be copied [4]. PROP + SA.

Experiments:
- 13.X1 `Gate::Wait` with `cv.wait(lock)` and no predicate; then `Handoff::Take` with `if` instead of a loop, two takers: hang; then an intermittent assertion (record n of 20, as the guide does).
- 13.X2 `Open()` sets an atomic and notifies without the mutex; a test hook pauses the waiter between its check and its sleep: deterministic hang; taking the mutex in `Open` fixes it.
- 13.X3 Call callbacks under the lock in `Publish`; run the re-entrant test, then under TSan: hang; a TSan double-lock report is expected.

### Chapter 14. Lifecycles: shutdown, timed waits, `stop_token`, atomics (6 h)

**Model.** A thread-owning object has a state machine under the same mutex as its data. Every predicate is
"condition or state changed", and every state change notifies every condition variable whose predicate
mentions state. Shutdown order: change state under the lock, notify, unlock, join. Deadlines compose;
relative timeouts do not. `jthread`, `stop_token` and `condition_variable_any` standardise wake-on-cancel.
An atomic suits one independent value on a hot path, not an invariant spanning several variables.

**Misconceptions.** M1 "set a flag and join" (the worker is asleep). M2 "joining while holding the lock is
fine". M3 "`wait_for(100ms)` waits 100 ms; timeouts in a loop add up". M4 "atomics make it thread-safe;
relaxed is always wrong". M5 "`jthread` makes shutdown automatic" (only stop-aware waits wake; member order
matters).

**Neighbours.** `wait_for`, `wait_until` (11); `jthread`, `stop_token`, `stop_callback` (20);
`condition_variable_any` (11; stop overloads 20, required for `stop_token`); `atomic`, memory orders (11);
`atomic::wait` (20, one paragraph); `future`, `packaged_task` (11, one page); `move_only_function` (23).

**Anchors.** `src/Core.h:62-68,87,91`; `Core.cpp:45-51,68-86,147-155,253,271-303,341-364,378-382,581-632,638-656`;
guide steps 9, 10, 12. Logger uses no `wait_for`, `jthread` or `stop_token`; the chapter says so.

**Back in logger.** (1) Order and justify the steps of `Core::Stop` (`Core.cpp:599-611`). What fails if the
join precedes the unlock? If two threads call `shutdown()` and `worker` is joined in place? (2) Would
`jthread` or `wait_for` simplify `Core`? (3) `threshold` is a relaxed atomic outside the mutex; `capacity`,
also one number, is a plain field under it. Why? (4) What question does
`writer.load() == std::this_thread::get_id()` answer, and why can the answer never be wrong for the thread
asking?

Drills (all also under TSan):
- 14.1 Add `void Close(); bool Push(T); opt<T> Pop();` to 13.4 [2]. A blocked consumer is released; items pushed before `Close` still arrive; `while (auto x = q.Pop())` ends.
- 14.2 `class Worker { explicit Worker(std::function<void(int)> handler); ~Worker(); bool Post(int job); void Stop(); }`. All jobs handled before `Stop` returns; `Stop` twice and concurrently; destructor without `Stop`.
- 14.3 `opt<T> PopFor(duration timeout)`; `opt<T> PopUntil(time_point<Clock, D> deadline)` [10]. Expired deadline returns at once; one-hour timeout plus a push returns the value; `FakeClock` advance then `Poke`; noise pokes do not extend the deadline.
- 14.4 `StoppableWorker`: 14.2 with `jthread`, `condition_variable_any`, `stop_token`; defaulted destructor. Destroying an idle worker does not hang; `Discarded()`.
- 14.5 `class Gauge` with a relaxed atomic level; `bool first_time() noexcept` via `exchange(true)`; a counter with `fetch_add` (stub does load, add, store). Exactly one of eight racing threads gets `true`; exact count.
- 14.6 (stretch) `Restartable`: `Post` after `Stop` lazily restarts; producers wait while draining. 100 stop and post cycles lose nothing.
- 14.7 (stretch) `std::future<int> Submit(std::move_only_function<int()> task)` [4]. Results; exceptions arrive through the future.

Design problems:
- 14.A Many threads submit records; a worker hands them to a slow `flush(batch)` when 100 are waiting or the oldest is 50 ms old. `Close()` sends the rest. Callers never wait for `flush`. RT with `FakeClock`.
- 14.B `Shutdown(grace)` stops intake, lets queued work finish for up to `grace`, then abandons and counts the rest. State what can and cannot be guaranteed about a stuck handler. RT + REF.
- 14.C Any thread may call `Report(message)` at any rate. At most one per interval reaches a slow sink; the next forwarded message carries `(+N suppressed)`. No background thread. RT with `FakeClock`.

Experiments:
- 14.X1 Join while holding the lock in `Worker::Stop`; then join `worker` in place from two threads: hang; then an exception or TSan report.
- 14.X2 Use plain `condition_variable::wait` in `StoppableWorker`; then declare the `jthread` before the mutex: destructor hangs; then an ASan or TSan use-after-destroy.
- 14.X3 Implement `PopFor` as a loop around `wait_for(lock, timeout)`: the noise-poke test fails by watchdog.

### Chapter 15. Capstones

**Capstone 1: `logstat`, an analyser for logger's own output** (about 400 to 600 lines, 10 to 14 h). Tests
are text against committed fixture logs produced by the real logger; the tutorial does not link logger.

| Milestone | Behaviour | Decisions left to you |
| --- | --- | --- |
| 1 | Parse one line of `2026-10-07T12:34:56.789Z [INFO ] file.cpp:42 message` | entry owns its message or views a file buffer; optional versus expected versus counting failures |
| 2 | Read files. A message containing a newline produces continuation lines. Malformed lines are counted. Rotated generations are read oldest first. | streaming versus load-all; where rotation knowledge lives |
| 3 | Queries: counts per level; top N `file:line`; histogram per time bucket; longest gaps; `--since`, `--until`, `--level`, `--grep` | pipelines versus loops; which parts are `constexpr` |
| 4 | Aligned text report and CSV | a formatter for `Entry`, or free functions |
| 5 | `check` sub-command verifying logger's own guarantee: timestamps never go backwards within a file | how to report violations |

Stretch: a k-way merge of several processes' logs by timestamp.

**Capstone 2: `Scheduler<Clock>`, a timer queue** (about 350 to 500 lines, 10 to 14 h). Chosen because it
needs the one pattern logger lacks (a timed wait re-evaluated when an earlier deadline arrives) and resembles
the logger least.

| Milestone | Behaviour |
| --- | --- |
| 1 | Pure core without threads: add, cancel, next due time, pop everything due at `now`. Tested with plain time points. |
| 2 | Threaded shell: one worker waits until the earliest deadline; inserting an earlier timer wakes it. Tested with `FakeClock` and `Poke`. |
| 3 | Tasks run outside the lock and may schedule or cancel timers. Define cancel-while-running. Exceptions go to an error handler. |
| 4 | Periodic timers without drift; shutdown with drain or discard; destructor. |
| 5 (stretch) | Run tasks on N workers; `Submit` returning futures. |

Optional third: a metrics aggregator on top of the scheduler.

### Take it to your rebuild

Extensions to your own copy of the logger. The first four are the ones step 14 of the guide suggests.

| # | Extension | After chapter | Notes |
| --- | --- | --- | --- |
| 1 | `RotatingFileSink` | 11 | uses drill 11.6; count bytes yourself (11.X2); daily rotation keys on `floor<days>(record.timestamp)`; `Core` unchanged |
| 2 | Coloured console sink | 8 | a `constexpr` level-to-colour table |
| 3 | Thread id or function name in the line | 9 | `RecordTests` change |
| 4 | Dropping mode | 14 | beside the `notFull` wait at `Core.cpp:290-292`; reuses `dropped` and `ReportDrops` |
| 5 | `std::formatter<logger::Level>` and `<logger::Record>` | 8 | lets `[{:<5}]` take the `Level` directly |
| 6 | Add the pre-1970 case to `RecordTests`; try the chrono-spec one-liner | 10 | then decide which to keep |
| 7 | `bool flush_for(timeout)` | 14 | `progress.wait_for` at `Core.cpp:566`; a closed `GateSink` must make it return false |
| 8 | `constexpr parse_level` with `find_if`, and a `static_assert` in `LevelTests` | 6 | moves the function into the header: weigh it |
| 9 | Constrain `add_sink` with `std::derived_from<S, Sink>` | 5 | written, in chapter 5 |
| 10 | A JSON-lines sink using `{:?}` | 8 | custom sink; no `Core` change |
| 11 | Evaluate replacing `std::thread` and `State` with `jthread` on a branch | 14 | analysis; note which tests break |

### Totals for the full tutorial

About 109 core drills and 5 stretch, 42 design problems, 43 experiments, 15 review problems, 3 fix-it sets,
3 clinics, 2 capstones and 11 rebuild extensions. Roughly 95 hours of study, or about 55 on a fast path that
marks about 65 drills and one design problem per chapter as core.
