# Chapter 1: reference solutions

Compiled against the chapter's tests: `ctest --preset solutions -R sol01`.

## Drills

**Drill 1, `d01_trim.h`.** Two searches and one `substr`. The early return for "nothing but blanks" is not
optional: `find_first_not_of` returns `npos` there, and `substr(npos)` throws.

**Drill 2, `d02_split.h`.** The loop never looks at a character it was not given: `find` stops at `size()`.
A version built on `std::strchr` or `std::strtok` passes four of the six tests and reads past the end in the
other two. `text.remove_prefix(at + 1)` needs no check, because `at` is the position of a character that
exists, so `at + 1` is at most `size()`.

**Drill 3, `d03_next_token.h`.** The two slicing functions are used for what each is good at.
`remove_prefix` has no safety net, so its argument goes through `std::min(..., rest.size())`: when only
blanks are left, `find_first_not_of` returns `npos`. `substr`'s count is clamped, so
`rest.substr(0, rest.find_first_of(kBlanks))` needs nothing: no blank found means "all of it".

**Drill 4, `d04_mean.h`** and **drill 5, `d05_scale.h`.** A range-`for` over the span. `static_cast<double>`
on the size, because the warning set does not let an integer become a `double` silently.

**Drill 6, `d06_longest_run.h`.** One pass with two indices; the run that is currently growing starts at
`start`. `length > bestLength` keeps the first of equally long runs, `>=` would keep the last. For an empty
input the loop does not run and `subspan(0, 0)` is an empty span.

**Drill 7, `d07_parse_pairs.h`.** `split` at `';'`, `find('=')` for the first equals sign (so that a value
may contain one), `trim` on both sides. `KeyValue` holds two views and is therefore a view itself: the
vector this function returns must not outlive the text it was given.

## Problem 1: one section of a configuration file

`p01_section_body.h` takes two `std::string_view`s and returns one. Nothing else meets the requirement that
nothing be copied: returning a `std::string` allocates for every lookup, and a
`std::vector<std::string_view>` of lines allocates the vector.

<!-- file: solutions/01-views/p01_section_body.h#contract -->
```cpp
/// The body of section @p name in an INI-like @p text: everything after the line "[name]" up to the
/// next line that starts with '[', or to the end. Empty when there is no such section.
///
/// LIFETIME: the result is a part of @p text. It is valid only while the characters @p text refers
/// to exist and do not move: do not let it outlive the buffer the file was read into.
[[nodiscard]] constexpr std::string_view section_body(std::string_view text,
                                                      std::string_view name) noexcept
```

**The comment is half of the answer.** The function hands back a part of its argument, which is the safe
kind of view-returning function: the result is valid exactly as long as the argument's characters are. The
caller cannot see that from the signature. A caller who writes
`section_body(read_file("app.ini"), "server")` has built the experiment `x02_view_of_temporary` again. The
comment is where they find out.

**Walking lines without allocating.** The loop takes one line off the front of `text` per pass, as drill 3
takes words. A header matches when the line is exactly `[` + name + `]`, which is why `serve` does not find
`[server]`, and why a key called `port` is not a section. The body ends at the next `'['` that begins a line:
either the text starts with one (an empty section) or it follows a `'\n'`.

**What else would be a good answer.** Returning `std::optional<std::string_view>`, to tell "no such section"
from "an empty section". The statement asks for "something empty", and the tests accept that. Whether a
caller needs the distinction is a real design question; chapter 2 gives you the type for it.

**`constexpr`.** The reference is `constexpr` because nothing in it prevents that, and because it lets the
whole function be tested with `static_assert`. It is not required.

## Problem 2: the checksum of a frame

`p02_payload_checksum.h` takes a `std::span<const std::uint8_t>`.

- **One parameter type for three callers.** A vector and a `std::array` convert to it implicitly, and
  `{data, length}` constructs it from a pointer and a size.
- **"Unable to change the frame"** is the `const` inside the angle brackets. `std::span<std::uint8_t>` would
  still compile for the first and second caller, would reject the `const` pointer of the third, and would
  let the function write.
- **The payload is a `subspan`.** Header and trailer are skipped by arithmetic on the view, not by index
  bookkeeping in the loop. The length check comes first, because `subspan` with a count larger than the span
  is undefined (and an abort in this build).

A template (`template <typename Container> ... payload_checksum(const Container& frame)`) also accepts the
vector and the array. It does not accept `{data, length}`, it is instantiated once per container type, and
it accepts a `std::list`, for which "header, payload, trailer" by position costs a walk. The span says what
is actually required: bytes next to each other.

## Problem 3: the signature clinic

<!-- file: solutions/01-views/p03_signatures.h#answers -->
```cpp
// 1. Stores the name in the object.
class Player
{
public:
    explicit Player(std::string name);  // by value: the function keeps it (chapter 4 finishes this)

private:
    std::string name;
};

// 2. Only compares two names.
[[nodiscard]] bool same_name(std::string_view a, std::string_view b) noexcept;

// 3. Opens the file with std::fopen.
[[nodiscard]] std::FILE* open_for_reading(const std::string& path);  // fopen needs a NUL terminator

// 4. Returns the extension of a file name, without the dot.
[[nodiscard]] std::string_view extension_of(
    std::string_view fileName) noexcept;  // a part of its argument

// 5. Returns the name in upper case.
[[nodiscard]] std::string to_upper(
    std::string_view text);  // new characters: someone has to own them

// 6. Adds up sensor readings.
[[nodiscard]] double total(std::span<const double> readings) noexcept;

// 7. Clamps every sample to [-1, 1], in place.
void clamp_samples(std::span<float> samples) noexcept;

// 8. Appends three default entries to a list.
void add_defaults(std::vector<std::string>& entries);  // it grows: a span cannot

// 9. Returns the readings above a threshold.
[[nodiscard]] std::vector<double> above(std::span<const double> readings, double threshold);

// 10. Writes a 16-bit value into a buffer at an offset, big-endian. Reports whether it fitted.
[[nodiscard]] bool put_u16(std::span<std::uint8_t> buffer, std::size_t offset,
                           std::uint16_t value) noexcept;
```

1. **Stores it: `std::string` by value.** The constructor needs its own copy whatever it is given. Taking a
   `std::string_view` would be correct and always copies; taking `const std::string&` likewise. By value lets
   a caller who no longer needs their string hand it over instead. Chapter 4 explains why and what the
   constructor body then says.
2. **Only reads: `std::string_view`, twice.** Literals, strings and parts of strings compare without a copy.
3. **Needs a terminator: `const std::string&`.** `std::fopen` takes a C string. A view cannot promise one.
4. **Returns a part of its argument: `std::string_view` in and out.** The logger's `file_basename` is this
   function.
5. **Returns new characters: `std::string`.** The upper-case text does not exist before the call. Something
   has to own it, and a view owns nothing. The parameter is still a view.
6. **Only reads a sequence: `std::span<const double>`.**
7. **Changes elements in place: `std::span<float>`.** No `const` inside.
8. **Changes the number of elements: `std::vector<std::string>&`.** A span has a fixed length; it cannot
   grow what it looks at.
9. **Returns elements it selected: `std::vector<double>`.** The result is not one contiguous part of the
   input, so it cannot be a span of it. (Chapter 12 has a lazy alternative.)
10. **Writes into a part of a buffer: `std::span<std::uint8_t>`, an offset and the value; returns `bool`.**
    The span carries the buffer's length, which is exactly what the function needs to know whether the value
    fits. `[[nodiscard]]`, because a caller who ignores "it did not fit" has a bug.

## Back in logger

**1.** `file_basename` returns a part of its argument. In `format_line_to` the argument is
`record.location.file_name()`, a pointer to a string literal that the compiler placed in the program's
read-only data: it is valid for the whole run, so a view of it can never dangle. The message is different.
It is formatted on the calling thread into a `std::string`, and the record that carries it is queued and
written later, on another thread, long after the `logger::info(...)` statement has ended. A view there would
point at a string that died with that statement. `Record` therefore owns its message. The comment on
`Record` in `include/logger/Record.h` says the same from the other side: the location's strings are
literals, "so a Record can be copied and kept".

**2.** Without `std::span`, either `WriteBatch` takes a `const std::vector<Record>&` and the single-record
caller builds a one-element vector, which allocates, on the path that runs when the program is already
exiting and that exists partly because memory may be short; or `WriteBatch` takes a pointer and a count as
two parameters; or there are two functions, one looping over the other. The span is the pointer and the
count, as one value that both callers can produce for nothing: `batch` converts implicitly, and
`std::span<const Record>{&moved, 1}` is "this one object, as a sequence".

**3.** It would work today, by accident. Every present caller passes a string literal or the `std::string`
that `std::format` returned, and both happen to be followed by a `'\0'`. But the parameter is a
`std::string_view`, and that type promises the callers that any text will do, including a part of a longer
one. With `std::fputs` the function would be correct only for callers who know a rule that is written
nowhere, and the first one who passes a slice would get extra characters, or a read past a buffer, on the
path that reports errors. `std::fwrite` with `text.size()` writes exactly the characters of the view, so the
signature tells the truth. (The comment above the function gives a second, unrelated reason for making it a
single stdio call: such a call is locked as a unit.)
