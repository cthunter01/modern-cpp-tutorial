# Chapter 1: Views that do not own

`std::string_view` and `std::span` are the types the logger hands text and records around in. Both are the
same idea: a pointer and a length, referring to storage that belongs to somebody else. They cost nothing to
copy and nothing to cut into pieces, they accept data from almost any source, and they will let you keep
looking at storage that no longer exists without a word of complaint. This chapter is about using the first
three properties without being caught by the fourth.

## Goal

- You choose `std::string_view` and `std::span<const T>` for read-only parameters without thinking, and know
  the cases where they are the wrong choice.
- You can say, for any view in a program, who owns what it refers to and until when.
- You can take text apart (trim, split, tokenise, parse key=value) without copying a character.

## Where you met it

Step 1 of the guide: `level_name` returns a `std::string_view` and `parse_level` takes one. Step 2 gave you
the first function that returns a part of its argument:

<!-- logger: include/logger/Record.h:24-29 -->
```cpp
/// The part of @p path after the last '/' or '\\' (whichever the compiler used), or all of it.
[[nodiscard]] constexpr std::string_view file_basename(std::string_view path) noexcept
{
    const auto pos = path.find_last_of("/\\");
    return pos == std::string_view::npos ? path : path.substr(pos + 1);
}
```

Step 5 introduced `std::span`, in one sentence, as the parameter of the function that writes a batch:

<!-- logger: src/Core.h:81-82 -->
```cpp
void WriteBatch(std::span<const Record>                   batch,
                const std::vector<std::shared_ptr<Sink>>& targets) noexcept;
```

and step 14 used one to make `argv` safe to index: `const std::span args(argv, static_cast<std::size_t>(argc));`.
The guide never says what either type is. That is this chapter.

## The idea

A `std::string_view` is two machine words: a pointer to the first character and a count. A `std::span<T>` is
the same for elements of type `T`. Neither contains the characters or the elements. Neither allocates,
copies or frees anything, ever. Constructing one, copying one and destroying one are as cheap as doing the
same to a pointer and an integer, because that is what they are.

What they replace is a choice C++11 forced on you for every read-only parameter:

- `const std::string&` accepts a `std::string` for free and nothing else for free. Pass a literal, a part
  of a string or characters in a buffer, and a temporary `std::string` is built first: an allocation and a
  copy, so that the function can look at characters that already existed.
- `const char*` accepts anything that ends in a `'\0'`, loses the length, and cannot refer to a part of a
  text.
- `const std::vector<T>&` accepts a vector. An array, a part of a vector or a single object need a vector
  built around them first.
- A pointer and a count as two parameters accept everything, and nothing stops a caller from passing a count
  that belongs to a different pointer.

A view is the fourth option made into a type. One function:

<!-- file: chapters/01-views/examples/e01_views.cpp#text-param -->
```cpp
std::size_t count_spaces(std::string_view text) noexcept
{
    std::size_t count = 0;
    for (const char c : text)
    {
        count += (c == ' ') ? 1U : 0U;
    }
    return count;
}
```

and every kind of caller, none of which copies a character:

<!-- file: chapters/01-views/examples/e01_views.cpp#text-sources -->
```cpp
const std::string         owned = "a std::string owns its characters";
const std::array<char, 9> raw{'n', 'o', ' ', 'n', 'u', 'l', ' ', 'h', 'i'};

std::cout << count_spaces("a literal, in the program's read-only data") << '\n';
std::cout << count_spaces(owned) << '\n';  // implicit conversion
std::cout << count_spaces(std::string_view(owned).substr(0, 13)) << '\n';  // "a std::string"
std::cout << count_spaces({raw.data(), raw.size()}) << '\n';               // pointer and length
```

The same for sequences. `std::span<const int>` reads as "some `int`s next to each other in memory, which I
may look at":

<!-- file: chapters/01-views/examples/e01_views.cpp#span-param -->
```cpp
int sum(std::span<const int> values) noexcept
{
    int total = 0;
    for (const int value : values)
    {
        total += value;
    }
    return total;
}
```

<!-- file: chapters/01-views/examples/e01_views.cpp#span-sources -->
```cpp
const std::vector<int>   vector{1, 2, 3, 4};
const std::array<int, 3> array{10, 20, 30};
const int                builtIn[] = {100, 200};  // NOLINT(*-avoid-c-arrays): on purpose
const int                single    = 7;

std::cout << sum(vector) << '\n';              // 10
std::cout << sum(array) << '\n';               // 60
std::cout << sum(builtIn) << '\n';             // 300
std::cout << sum({&single, 1}) << '\n';        // 7     one object is a sequence of one
std::cout << sum({vector.data(), 2}) << '\n';  // 3     pointer and length
std::cout << sum(std::span(vector).subspan(1, 2)) << '\n';  // 5     elements 1 and 2
```

The second half of the idea is slicing. Because a view is only a start and a length, a part of a view is
another view, made by arithmetic:

<!-- file: chapters/01-views/examples/e01_views.cpp#text-slicing -->
```cpp
const std::string_view line = "  timeout = 30s  ";

const std::size_t      equals = line.find('=');           // 10
const std::string_view key    = line.substr(0, equals);   // "  timeout "
std::string_view       value  = line.substr(equals + 1);  // " 30s  "

value.remove_prefix(1);  // "30s  "   move the start forward
value.remove_suffix(2);  // "30s"     move the end back

std::cout << '[' << key << "] [" << value << "]\n";
std::cout << std::boolalpha << value.ends_with('s') << ' ' << line.starts_with("  time") << ' '
          << line.contains("out") << '\n';
```

This prints `[  timeout ] [30s]` and `true true true`, and no character was copied to do it:

<!-- file: chapters/01-views/examples/e01_views.cpp#text-part-of -->
```cpp
// Nothing was copied: `value` is three of the characters of the literal above.
std::cout << (value.data() == &line[12]) << '\n';
```

`std::span` slices the same way, with `first(n)`, `last(n)` and `subspan(offset, count)`:

<!-- file: chapters/01-views/examples/e01_views.cpp#span-slicing -->
```cpp
const std::array<int, 6>   samples{0, 1, 2, 3, 4, 5};
const std::span<const int> all(samples);

std::cout << sum(all.first(2)) << ' '         // 0 1
          << sum(all.last(2)) << ' '          // 4 5
          << sum(all.subspan(2)) << ' '       // 2 3 4 5
          << sum(all.subspan(2, 2)) << '\n';  // 2 3
std::cout << std::as_bytes(all).size() << " bytes\n";
```

`starts_with` and `ends_with` are C++20, `contains` is C++23, `std::span` is C++20, `std::string_view` is
C++17. `std::as_bytes` gives the same memory as a `std::span<const std::byte>`, which is how binary data is
handed to something that writes bytes.

## The details that bite

### A view keeps nothing alive

This is the whole price. A reference into a container at least looks like a reference. A `std::string_view`
looks like a string, is passed around by value like a string, can be returned and stored like a string, and
is valid only while some other object, which it does not mention, stays alive and stays where it is.

There are exactly three ways to get it wrong, and the "Break it on purpose" section runs each of them:

- **Return a view of a local.** The local dies at the closing brace; the view goes home with the caller.
- **Take a view of a temporary.** `std::string_view v = make_string();` or, less visibly,
  `first_word(make_string())`: the string lives until the end of the statement, the view lives on.
- **Keep a view while the owner changes.** A `std::string` or `std::vector` that grows moves its contents;
  every view of the old storage now points at freed memory.

The rule that prevents all three: **a view may be a parameter, a local variable, or a return value that is a
part of a parameter. It is not something to store.** A function that needs the text after it returns takes
or makes a `std::string`. This is why the logger's `Record` has a `std::string message` although every
function that only reads a message takes a `std::string_view`.

Returning a part of a parameter is safe in a precise sense: it gives the caller nothing they did not already
have. `file_basename(path)` is valid exactly as long as `path` was. Say so in the function's comment; it is
part of the interface. The logger's does: "The part of `path` after the last '/'".

### A view is not a C string

A `std::string` promises a `'\0'` after its last character. A `std::string_view` promises nothing about what
follows its last character: it may be the rest of a longer text, or the end of a buffer. `data()` returns a
pointer to the first character and that is all it is.

So `data()` must never go alone to anything that looks for a terminator: `std::strlen`, `std::printf("%s")`,
`std::fopen`, `std::atoi`. Always pass the size along, as the logger does when it writes a message to
`stderr`:

<!-- logger: src/Core.cpp:38-41 -->
```cpp
void write_stderr(std::string_view text) noexcept
{
    static_cast<void>(std::fwrite(text.data(), 1, text.size(), stderr));
}
```

When a function has to call an API that needs a C string, a view is the wrong parameter type. Take a
`const std::string&`: it has `c_str()`, and a caller holding only a view has to build the string, visibly,
at the place where the cost belongs.

### Pass views by value, and put `const` on the element

A view is two words. Passing it by `const&` passes a pointer to two words, which is slower and gains nothing.
Views are parameters by value.

`const` in front of a span does not do what it does in front of a container:

<!-- file: chapters/01-views/examples/e01_views.cpp#span-const -->
```cpp
std::vector<int> numbers{1, 2, 3, 4, 5};

const std::span<int> fixedView(numbers);  // the view cannot be re-pointed...
fixedView[0] = 99;                        // ...but the elements are still ints, and writable

const std::span<const int> readOnly(numbers);  // const elements: this is the read-only view
// readOnly[0] = 1;                             // does not compile

fill(std::span(numbers).last(2), 0);  // 99 2 3 0 0
```

`const std::span<int>` is a view that cannot be re-pointed, onto elements anybody may write. The read-only
view is `std::span<const int>`: constness is a property of what is viewed, so it goes inside the angle
brackets. A function that only reads takes `std::span<const T>`, and that is also what makes it accept a
`const std::vector<T>`. A function that modifies elements in place takes `std::span<T>`:

<!-- file: chapters/01-views/examples/e01_views.cpp#span-mutable -->
```cpp
void fill(std::span<int> values, int with) noexcept
{
    for (int& value : values)
    {
        value = with;
    }
}
```

`std::string_view` has no such choice: it is always read-only.

### Slicing is free, and only partly checked

The slicing functions differ in what they do when you ask for more than there is, and the difference is not
obvious from their names:

| Call | When out of range |
| --- | --- |
| `text.substr(pos, count)` | `count` is clamped to what is left. `pos > size()` throws `std::out_of_range`. |
| `text.remove_prefix(n)`, `text.remove_suffix(n)` | `n > size()` is undefined behaviour. |
| `text[i]`, `text.front()`, `text.back()` | Undefined behaviour. |
| `values.first(n)`, `values.last(n)`, `values.subspan(offset, count)` | Undefined behaviour. |
| `values[i]`, `values.front()`, `values.back()` | Undefined behaviour. |

"Undefined behaviour" here means a read or a view outside the storage, with no diagnostic. This tutorial's
debug builds, like the logger's, define `_GLIBCXX_ASSERTIONS`, which turns every row marked undefined into an
immediate abort with a message. That is why a drill of yours may stop with `Assertion '...' failed` where you
expected a wrong answer: the library caught you one step earlier.

The clamping of `substr`'s count is useful. `text.substr(0, text.find(' '))` is "up to the first space, or
all of it": when there is no space, `find` returns `npos`, the largest possible count, and `substr` clamps
it. `remove_prefix` has no such safety net, so `text.remove_prefix(text.find(' '))` on a text without a space
is a bug.

### `std::span` is for elements that are next to each other

A span is a pointer and a count, so it can only describe storage that is one block: a `std::vector`, a
`std::array`, a built-in array, a `std::string`, a part of any of those, or one object. It cannot describe a
`std::list`, a `std::map` or a `std::deque`. For "anything I can iterate over" the tool is a range, which is
chapter 12.

Within its domain it is the general read-only sequence parameter, and `const std::vector<T>&` no longer is.
The logger's `WriteBatch` shows why. It is called with the whole queue:

<!-- logger: src/Core.cpp:446-447 -->
```cpp
// Record-major: each record goes to every sink, in the order the sinks were added.
WriteBatch(batch, targets);
```

and, on the path that runs after `main` has returned, with one record that lives on the stack:

<!-- logger: src/Core.cpp:380 -->
```cpp
WriteBatch(std::span<const Record>{&moved, 1}, targets);
```

One function body serves both, and the second call allocates nothing.

## When to reach for it, and when not

| The function... | Parameter | Return type |
| --- | --- | --- |
| only reads a text | `std::string_view`, by value | |
| passes the text to a C or OS function | `const std::string&` | |
| keeps the text after it returns | `std::string`, by value (chapter 4 finishes this) | |
| returns a part of its argument | | `std::string_view`; say so in the comment |
| returns a text it built | | `std::string` |
| only reads a sequence | `std::span<const T>`, by value | |
| changes elements in place | `std::span<T>` | |
| adds or removes elements | `std::vector<T>&` | |
| returns a part of its argument | | `std::span<const T>` |
| returns elements it produced | | `std::vector<T>` |

And as a data member: almost never. A struct of views, like the `KeyValue` of drill 7, is itself a view and
has to be treated as one. A long-lived object with a view in it is a bug waiting for its owner to go away.

## Drills

Run each with `ctest --preset clang-debug -R ch01_dNN`, and when a drill passes, once more with
`--preset asan`. Several of these tests can only fail there.

| # | Target | File | Write |
| --- | --- | --- | --- |
| 1 | `ch01_d01` | `exercises/d01_trim.h` | `trim`: a text without the blanks at its ends |
| 2 | `ch01_d02` | `exercises/d02_split.h` | `split`: the pieces between delimiters |
| 3 | `ch01_d03` | `exercises/d03_next_token.h` | `next_token`: take the first word off a text |
| 4 | `ch01_d04` | `exercises/d04_mean.h` | `mean`: the average of a sequence of doubles |
| 5 | `ch01_d05` | `exercises/d05_scale.h` | `scale`: multiply a sequence in place |
| 6 | `ch01_d06` | `exercises/d06_longest_run.h` | `longest_run`: the longest stretch of equal values |
| 7 | `ch01_d07` | `exercises/d07_parse_pairs.h` | `parse_pairs`: `a=1; b = 2` into keys and values |

**Drill 1: `trim`.** `find_first_not_of` and `find_last_not_of` take a set of characters and give you the
two ends; `substr` does the rest. Mind the text that consists of blanks only. The tests check that the result
points into the input, not just that it compares equal.

The drill has a second test, `ch01_d01.c1`. It compiles `tests/d01_c1_constexpr.cpp`, a file of
`static_assert`s: `trim` is `constexpr`, so its answers can be demanded at compile time, where a wrong one is
a build error. Until your `trim` is right, that file does not compile and the test shows you why. The
logger tests `file_basename` the same way. Chapter 6 is about `constexpr`; here it is only a second way of
asking.

**Drill 2: `split`.** Loop: find the delimiter, push the piece in front of it, drop piece and delimiter from
the front, repeat; the last piece is whatever is left. Empty pieces count, so `"a,,b"` has three and `""` has
one. Two of the tests hand you characters that are not followed by a `'\0'`. In an ordinary build a `split`
that relies on one may pass; under `asan` it is reported.

**Drill 3: `next_token`.** The parameter is a `std::string_view&`: the function consumes from the caller's
view. Skip the blanks, cut the word, leave `rest` directly behind it. The last test calls the function on a
text that is already empty, three times. Here `remove_prefix` with a count you have not checked ends the
program.

**Drill 4: `mean`.** Four lines. The exercise is the test file: read `tests/d04_mean_tests.cpp` and see how
many different things one `std::span<const double>` accepts.

**Drill 5: `scale`.** The mutable counterpart. Look at the third test, the one with the `const std::span`.

**Drill 6: `longest_run`.** Return a part of the input, with `subspan`. Of several equally long runs the
first wins, which decides whether your comparison is `>` or `>=`.

**Drill 7: `parse_pairs`.** Combine: split at `';'`, find the `'='`, trim both sides. The stub includes your
own `d01_trim.h` and `d02_split.h`. Every key and value in the result is a part of the input, and the input
is a `std::string_view`: nothing here owns a character.

## Design problems

**Problem 1: one section of a configuration file** (`ch01_p01`, you create `exercises/p01_section_body.h`).
A configuration file has been read into memory as one text. It consists of sections: a line `[name]`, then
the lines that belong to it, up to the next line that starts with `[` or to the end of the text. Lines end in
`'\n'`. Write `ch01::section_body(text, name)`, which returns the body of the named section for further
parsing, or something empty if there is no such section. Files are large and sections are looked up often:
finding one must not copy the file or any part of it. State, where a caller will read it, what they have to
keep alive for the result to stay valid.

The tests measure two things beside the answer: that your function performs no allocation, and that what it
returns lies inside the caller's text.

**Problem 2: the checksum of a frame** (`ch01_p02`, you create `exercises/p02_payload_checksum.h`).
A frame of a serial protocol is a 4-byte header, a payload of any length, and a 2-byte trailer. Write
`ch01::payload_checksum(frame)`, which returns the sum of the payload bytes modulo 256, and 0 for a frame too
short to have a payload. Frames reach the function in three forms: in a `std::vector<std::uint8_t>`, in a
`std::array<std::uint8_t, 64>`, and as a pointer to the first byte with a length (the tests write that call
as `payload_checksum({data, length})`). There is to be one implementation, and it must be unable to change
the frame.

**Problem 3: a signature clinic** (no file, no test). For each function described below, write down its
declaration: name, parameter types, return type. Nothing else. Then compare with
`solutions/01-views/p03_signatures.h` and read the reasons in `solutions/01-views/README.md`.

1. A `Player`'s constructor: stores the player's name in the object.
2. Tells whether two names are the same.
3. Opens a file for reading with `std::fopen` and returns the `FILE*`.
4. Returns the extension of a file name, without the dot.
5. Returns a text converted to upper case.
6. Adds up sensor readings.
7. Clamps every audio sample to the range -1 to 1, in place.
8. Appends three default entries to a list of names.
9. Returns the readings that are above a threshold.
10. Writes a 16-bit value into a byte buffer at a given offset, and reports whether it fitted.

## Break it on purpose

**Return a view of a local.** `experiments/x01_return_local_view.cpp` has this function:

<!-- file: chapters/01-views/experiments/x01_return_local_view.cpp#function -->
```cpp
std::string_view full_name(const std::string& first, const std::string& last)
{
    const std::string joined =
        first + " " + last;  // a local: it is destroyed when the function returns
    return joined;           // ... and this is a view of its characters
}
```

```sh
cmake --build --preset clang-debug -t x01_01
```

Clang refuses to build it:

<!-- file: chapters/01-views/experiments/x01_return_local_view.clang.expect -->
```
address of stack memory associated with local variable 'joined' returned
-Wreturn-stack-address
```

It is a warning that the tutorial's `-Werror` turns into an error, and it is right. GCC 16 says nothing
about the same file:

```sh
cmake --build --preset gcc-debug -t x01_01 && build/gcc-debug/bin/x01_01
```

<!-- file: chapters/01-views/experiments/x01_return_local_view.gcc.expect -->
```
the name has 34 characters
and it is:
```

The size is right, because the size is stored in the view. After `and it is:` comes whatever the allocator
has since written into the freed block, followed by the tail of the name that it has not yet overwritten. Do
not rely on a compiler to find this for you: Clang recognises this simplest form only.

**A view of a temporary, through a function.** In `experiments/x02_view_of_temporary.cpp` each function is
correct on its own:

<!-- file: chapters/01-views/experiments/x02_view_of_temporary.cpp#functions -->
```cpp
std::string make_greeting(const std::string& name)
{
    return "Hello, " + name + ", and welcome back";  // a new string, returned by value
}

std::string_view first_word(std::string_view text)
{
    return text.substr(0, text.find(' '));  // correct: a part of its argument
}
```

The mistake is in how they are combined:

<!-- file: chapters/01-views/experiments/x02_view_of_temporary.cpp#main -->
```cpp
// make_greeting() returns a std::string that nobody keeps. It lives until the end of this
// statement.
const std::string_view word = first_word(make_greeting("Ada"));

std::cout << "the first word has " << word.size() << " characters\n";
std::cout << "and it is: " << word << '\n';  // the characters were freed one line ago
```

Neither compiler warns. `cmake --build --preset clang-debug -t x01_02 && build/clang-debug/bin/x01_02` runs
to the end, prints the right size and then garbage. Under AddressSanitizer
(`cmake --build --preset asan -t x01_02 && build/asan/bin/x01_02`) it stops at the read:

<!-- file: chapters/01-views/experiments/x02_view_of_temporary.asan.expect -->
```
ERROR: AddressSanitizer: heap-use-after-free
READ of size 6
freed by thread T0 here:
```

The fix is one word: `const std::string greeting = make_greeting("Ada");` on its own line, so that the owner
has a name and a lifetime you can see. A function that returns a view of its parameter is safe; a call to it
with a temporary argument, whose result outlives the statement, is not, and nothing in the types tells them
apart.

**Hand `data()` to something that looks for a terminator.** `experiments/x03_strlen_of_slice.cpp`:

<!-- file: chapters/01-views/experiments/x03_strlen_of_slice.cpp#main -->
```cpp
const std::string_view line = "name=Ada Lovelace";
const std::string_view key  = line.substr(0, line.find('='));

std::cout << "key is '" << key << "' and key.size() is " << key.size() << '\n';
std::cout << "strlen(key.data()) is " << std::strlen(key.data()) << '\n';
std::printf("printf with %%s prints '%s'\n", key.data());
```

`cmake --build --preset clang-debug -t x01_03 && build/clang-debug/bin/x01_03` prints:

<!-- file: chapters/01-views/experiments/x03_strlen_of_slice.expect -->
```
key is 'name' and key.size() is 4
strlen(key.data()) is 17
printf with %s prints 'name=Ada Lovelace'
```

Nothing crashed, because the characters behind the key happen to belong to the same literal. The answer is
simply wrong. clang-tidy, which the `tidy` preset runs, knows this mistake:

<!-- file: chapters/01-views/experiments/x03_strlen_of_slice.tidy.expect -->
```
result of a `data()` call may not be null terminated
bugprone-suspicious-stringview-data-usage
```

`experiments/x04_unterminated_buffer.cpp` is the same mistake where nothing friendly follows the view:

<!-- file: chapters/01-views/experiments/x04_unterminated_buffer.cpp#main -->
```cpp
const std::vector<char> received{'P', 'I', 'N', 'G'};  // four bytes off a socket: no terminator
const std::string_view  message(received.data(), received.size());

std::cout << "message.size() is " << message.size() << '\n';
std::cout << "strlen(message.data()) is " << std::strlen(message.data()) << '\n';
```

Under `asan` (`cmake --build --preset asan -t x01_04 && build/asan/bin/x01_04`):

<!-- file: chapters/01-views/experiments/x04_unterminated_buffer.asan.expect -->
```
ERROR: AddressSanitizer: heap-buffer-overflow
READ of size 5
is located 0 bytes after 4-byte region
```

`strlen` read the four bytes and went on.

**Keep a span while its vector grows.** `experiments/x05_span_after_growth.cpp`:

<!-- file: chapters/01-views/experiments/x05_span_after_growth.cpp#main -->
```cpp
std::vector<int>           readings{10, 20, 30};
const std::span<const int> window(
    readings);  // a pointer to readings' storage, and the number 3

readings.push_back(
    40);  // the vector moves to bigger storage; `window` still points at the old one

std::cout << "window has " << window.size() << " elements\n";
std::cout << "the first one is " << window.front() << '\n';
```

The ordinary build prints `window has 3 elements` and then a first element that is not 10. The `asan` build
reports:

<!-- file: chapters/01-views/experiments/x05_span_after_growth.asan.expect -->
```
ERROR: AddressSanitizer: heap-use-after-free
READ of size 4
freed by thread T0 here:
```

This is chapter 0's dangling reference again, with a type that hides it better. Note what the span got
right: its size is still 3. It is a faithful description of where the vector's elements used to be.

## Back in logger

Answer these from the logger's source; the answers are in `solutions/01-views/README.md`.

1. `file_basename` returns a view into what, exactly? `format_line_to` calls it on
   `record.location.file_name()` (`src/Record.cpp:46`). Why is that safe, and why does `Record` nevertheless
   store its message as a `std::string` and not as a `std::string_view`?
2. `WriteBatch` is called with a whole vector (`src/Core.cpp:447`) and with one record on the stack
   (`src/Core.cpp:380`). If `std::span` did not exist, what would the two calls or the function have to look
   like, and what would the second call then cost?
3. `write_stderr` (`src/Core.cpp:38-41`) uses `std::fwrite(text.data(), 1, text.size(), stderr)`. Look at
   its callers: each passes a literal or a `std::string`. Would the shorter `std::fputs(text.data(), stderr)`
   work today? Why is it still the wrong function to call?

## Checkpoint

You should be able to explain:

- the three ways of ending up with a view of storage that is gone, and the one rule that avoids all of them;
- why `const std::span<int>` does not protect the elements, and what does;
- when `const std::string&` is still the right parameter type;
- what `text.substr(0, text.find(' '))` returns when there is no space, and why the same trick with
  `remove_prefix` is a bug.

Next: `std::optional`, for functions like `parse_level` that may have no answer to give.

---

Previous: [Chapter 0: Warm-up](../00-warm-up/README.md) | [Index](../../README.md) | Next: [Chapter 2: `std::optional`](../02-optional/README.md)
