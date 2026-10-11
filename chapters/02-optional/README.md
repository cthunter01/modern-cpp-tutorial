# Chapter 2: `std::optional`

Some functions have no answer to give: a search finds nothing, a text is not a number, a setting was never
set. `std::optional<T>` is the return type for them. It holds either a `T` or nothing, and it makes the
caller say which case they are handling. The logger's `parse_level` returns one, and the logger uses the
same type a second way that is less obvious and worth understanding: for a complaint that may or may not
exist.

## Goal

- You return `std::optional` wherever a C++11 function would have returned -1, an empty string, a `bool`
  with an out-parameter, or a null pointer to a new object.
- You know the four ways to get at the value, and which of them check.
- You can chain steps that may each have nothing to give, and you know when the caller needs
  `std::expected` instead.

## Where you met it

Step 1 of the guide, in the first function you typed:

<!-- logger: include/logger/Level.h:87-89 -->
```cpp
/// Parses "off", "error", "warn", "info", "debug" or "perf", ignoring ASCII case ("Warn", "WARN"
/// and "warn" all give Level::Warn); nullopt for anything else, including surrounding whitespace.
[[nodiscard]] std::optional<Level> parse_level(std::string_view text) noexcept;
```

Step 11 used it at start-up, where the environment variable is read:

<!-- logger: src/Core.cpp:126-136 -->
```cpp
if (const auto parsed = parse_level(raw))
{
    threshold.store(*parsed, std::memory_order_relaxed);
}
else
{
    startupError = std::format(
        "LOGGER_LEVEL=\"{}\" is not one of error, warn, info, debug, "
        "perf or off; the level stays {}",
        raw, level_name(threshold.load(std::memory_order_relaxed)));
}
```

`startupError` in that listing is a `std::optional<std::string>` member, and step 12 added a function that
returns one. The guide's only explanation of the type is half a sentence. This chapter is the rest.

## The idea

C++ before 2017 had three ways for a function to say "maybe there is no answer", each with a flaw:

<!-- file: chapters/02-optional/examples/e01_optional.cpp#sentinel -->
```cpp
// The old ways of saying "there may be no answer".
int  stock_or_minus_one(std::string_view name);     // a value that means "none": which one is safe?
bool get_stock(std::string_view name, int& stock);  // an out-parameter, and a result easy to ignore
const int* find_stock(
    std::string_view name);  // a pointer: fine for "one of mine", not for a new value
```

A sentinel steals a value from the answer's own type, and somebody has to know which one: chapter 0's
`first_repeat` returns -1, and cannot say that -1 itself was repeated. An out-parameter needs a variable
declared beforehand and a `bool` that is easy to ignore. A pointer works when the answer is an object that
already exists somewhere, and forces an allocation or a static when it is not.

`std::optional<T>` is a `T` and a flag in one object. The `T` lives inside the optional, not on the heap;
the flag says whether it is there. A function returns either a value or `std::nullopt`:

<!-- file: chapters/02-optional/examples/e01_optional.cpp#return -->
```cpp
std::optional<int> stock_of(std::string_view name)
{
    for (const Item& item : kShelf)
    {
        if (item.name == name)
        {
            return item.stock;  // an int converts to an optional that holds it
        }
    }
    return std::nullopt;  // the optional that holds nothing
}
```

and the caller has to go through the optional to reach the value:

<!-- file: chapters/02-optional/examples/e01_optional.cpp#use -->
```cpp
if (const std::optional<int> stock = stock_of("pear"))  // true: there is an answer
{
    std::cout << "pear: " << *stock << " in stock\n";  // and the answer is 0
}

const std::optional<int> missing = stock_of("mango");
std::cout << std::boolalpha << missing.has_value() << '\n';  // false
std::cout << missing.value_or(-1) << '\n';  // -1: a default, chosen by the caller
```

The first line is the idiom to learn by heart, and it is the one the logger uses: declare the optional in
the condition, test it, and use `*` inside the braces where the test is known to have passed.

| You write | You get | If it is empty |
| --- | --- | --- |
| `if (opt)`, `opt.has_value()` | whether there is a value | `false` |
| `*opt`, `opt->member` | the value, unchecked | undefined behaviour |
| `opt.value()` | the value, checked | throws `std::bad_optional_access` |
| `opt.value_or(fallback)` | the value, or `fallback` | `fallback` |
| `opt == x` | whether it holds a value equal to `x` | `false` |
| `opt == std::nullopt` | whether it is empty | `true` |
| `opt = x`, `opt.emplace(args...)` | now holds a value | |
| `opt.reset()`, `opt = std::nullopt` | now empty | |

`std::optional` is C++17. As a data member it says "this object may not have one":

<!-- file: chapters/02-optional/examples/e01_optional.cpp#member -->
```cpp
struct Customer
{
    std::string                name;
    std::optional<std::string> company;  // a private customer has none
};
```

<!-- file: chapters/02-optional/examples/e01_optional.cpp#member-use -->
```cpp
Customer customer{.name = "Ada", .company = std::nullopt};

customer.company = "Analytical Engines Ltd";    // now it holds a string
std::cout << customer.company->size() << '\n';  // -> reaches into the value, like a pointer

customer.company.reset();  // nothing again
std::cout << customer.company.value_or("(private)") << '\n';
```

## The details that bite

### `*` does not check

`*opt` and `opt->` assume there is a value, exactly as `*pointer` assumes the pointer is not null. On an
empty optional they are undefined behaviour: for an `optional<unsigned>`, typically a read of whatever
bytes happen to be in the unused storage. `opt.value()` is the checked form and throws.

This does not mean `value()` everywhere. Inside `if (opt) { ... }` the check has been made, and `*opt` is
the right spelling; `value()` there checks a second time and adds an exception path that can never be
taken. Use `value()` where no test is in sight and an empty optional would be a bug in the caller. The first
three experiments below show all three behaviours, and that this tutorial's debug builds, like the logger's,
turn the undefined one into an abort.

### `if (opt)` asks whether there is a value, not what it is

`std::optional<bool>` and `std::optional<int>` make this a trap, because their values look like conditions
themselves. `stock_of("pear")` holds 0, and is true as a condition. `parse_bool("false")` holds `false`,
and is true as a condition. There are two questions, "is there an answer?" and "what is it?", and `if (opt)`
only ever asks the first.

Comparison does the right thing, which is often the shortest correct way to ask both at once:

<!-- file: chapters/02-optional/examples/e01_optional.cpp#compare -->
```cpp
std::cout << (stock_of("pear") == 0) << ' '              // true:  holds a value equal to 0
          << (stock_of("mango") == 0) << ' '             // false: holds nothing
          << (stock_of("mango") == std::nullopt) << ' '  // true
          << (stock_of("mango") < stock_of("pear"))
          << '\n';  // true:  nothing sorts before anything
```

The logger's tests rely on this: `EXPECT_EQ(logger::parse_level("off"), Level::Off)` compares an optional
with a plain value, and `EXPECT_EQ(logger::parse_level(""), std::nullopt)` with emptiness.

### `value_or` computes its argument first

`opt.value_or(expression)` is an ordinary function call, so `expression` is evaluated before `value_or`
runs, whether or not the optional is empty. With a constant that costs nothing. With a function call the
function is called every time, and its result thrown away whenever there was a value. When the default is
expensive or has side effects, write `opt ? *opt : compute()`, or use `or_else` from the next section.

### An optional owns its value

The value is inside the optional. Copying an optional copies the value; an optional of a large type is a
large object. Three consequences:

- **Returning one needs no ceremony.** `return value;` and `return std::nullopt;` are all there is. Neither
  `std::make_optional` nor `std::move` is needed.
- **There is no optional reference.** `std::optional<T&>` does not exist before C++26. For "one of my
  existing objects, or none", return a pointer: `const Profile*`, null when there is none. Design problem 2
  needs this.
- **Moving from an optional does not empty it.** After `auto taken = std::move(opt);` the optional `opt`
  still holds a value: a moved-from `T`. To take the value *and* leave the optional empty, the logger uses
  the tool from chapter 0:

<!-- logger: src/Core.cpp:229-232 -->
```cpp
std::optional<std::string> Core::TakeStartupErrorLocked() noexcept
{
    return std::exchange(startupError, std::nullopt);
}
```

### It says "nothing", never "why"

An empty optional carries no information. For a search that is all there is to say. For a parser it throws
away the reason, and a caller who must tell the user what was wrong with their input cannot. That is what
`std::expected` is for; see below.

The logger turns the meaning around, and it is worth seeing why that is legitimate. `validate`-style
functions, and the logger's `StartWorkerLocked` and `startupError`, use `std::optional<std::string>` where
the string is a *complaint*: empty means "all is well", a value means "this went wrong". The type still
says what it always says, "a string, or nothing". Whether nothing is the good case or the bad one is the
function's contract, so the function's comment has to state it.

## Chaining: `and_then`, `transform`, `or_else`

Real lookups come in sequences: find the setting, parse it as a number, scale it. Each step can come up
empty, and by hand each needs its own `if`:

<!-- file: chapters/02-optional/examples/e01_optional.cpp#chain-by-hand -->
```cpp
int retries = 1;
if (const auto text = setting("retries"))
{
    if (const auto number = parse_int(*text))
    {
        retries = *number * 2;
    }
}
```

C++23 gave `std::optional` three member functions that do the testing for you:

<!-- file: chapters/02-optional/examples/e01_optional.cpp#chain -->
```cpp
const auto doubled = [](int n) { return n * 2; };

std::cout << setting("retries").and_then(parse_int).transform(doubled).value_or(1)
          << '\n';  // 6
std::cout << setting("timeout").and_then(parse_int).transform(doubled).value_or(1)
          << '\n';  // 1: not a number
std::cout << setting("colour").and_then(parse_int).transform(doubled).value_or(1)
          << '\n';  // 1: no such key
```

| Call | The function you pass takes | and returns | When the optional is empty |
| --- | --- | --- | --- |
| `opt.and_then(f)` | the value | an optional (the step may fail too) | `f` is not called; the result is empty |
| `opt.transform(f)` | the value | a plain value (the step cannot fail) | `f` is not called; the result is empty |
| `opt.or_else(f)` | nothing | an optional of the same type | `f` is called and supplies the result |

`and_then` is for the next step that may have no answer, `transform` for one that always has, `or_else`
for a fallback that should only be computed when needed. They are no more than the nested `if`s, and
whether a chain or the `if`s read better depends on the length: two steps are usually clearer by hand,
four are clearer chained.

## When "nothing" is not enough: `std::expected`

`std::expected<T, E>` (C++23) holds either a `T` or an `E`, the reason there is no `T`. It is used almost
exactly like an optional:

<!-- file: chapters/02-optional/examples/e01_optional.cpp#expected -->
```cpp
enum class ReadError : std::uint8_t
{
    Missing,
    NotANumber,
};

std::expected<int, ReadError> read_setting(std::string_view key)
{
    const std::optional<std::string_view> text = setting(key);
    if (!text)
    {
        return std::unexpected(ReadError::Missing);
    }
    const std::optional<int> number = parse_int(*text);
    if (!number)
    {
        return std::unexpected(ReadError::NotANumber);
    }
    return *number;
}
```

<!-- file: chapters/02-optional/examples/e01_optional.cpp#expected-use -->
```cpp
for (const std::string_view key : {"retries", "timeout", "colour"})
{
    if (const std::expected<int, ReadError> value = read_setting(key))
    {
        std::cout << key << " = " << *value << '\n';
    }
    else
    {
        std::cout << key << ": "
                  << (value.error() == ReadError::Missing ? "not set" : "not a number") << '\n';
    }
}
```

The differences: a failure is returned as `std::unexpected(reason)`, and the caller reads it with
`.error()`. `if (result)`, `*result`, `->`, `value()`, `value_or` and the three chaining functions exist as
for optional. The logger does not use it: its one parser can fail in one way only. You will use it in
drill 8 and meet it again in chapters 9 and 11.

## When to reach for it, and when not

| Situation | Use |
| --- | --- |
| A function may have no result, and there is one obvious reason | `std::optional<T>` |
| A function may fail in several ways the caller must tell apart | `std::expected<T, E>` |
| A failure that the caller cannot sensibly handle on the spot | an exception |
| "One of my existing objects, or none" | a pointer |
| A member that an object may not have | `std::optional<T>` |
| A value computed on first use | `std::optional<T>` as the cache |
| A function parameter that may be left out | usually an overload or a default argument; an `optional` parameter makes every caller write `std::nullopt` or wrap their value |
| A container that may be empty | the empty container; `optional<vector>` adds a second kind of empty |

## Drills

Run each with `ctest --preset clang-debug -R ch02_dNN`.

| # | Target | File | Write |
| --- | --- | --- | --- |
| 1 | `ch02_d01` | `exercises/d01_find_index.h` | `find_index`: where a value is in a sequence, if it is |
| 2 | `ch02_d02` | `exercises/d02_parse_uint.h` | `parse_uint`: a text as a number, if it is one |
| 3 | `ch02_d03` | `exercises/d03_parse_bool.h` | `parse_bool`: `true`, `false`, or no answer |
| 4 | `ch02_d04` | `exercises/d04_port_or.h` | `port_or`: a configured port, or an expensive fallback |
| 5 | `ch02_d05` | `exercises/d05_validate.h` | `validate`: the first complaint about some settings, if any |
| 6 | `ch02_d06` | `exercises/d06_mailbox.h` | `Mailbox`: put one message, take it once |
| 7 | `ch02_d07` | `exercises/d07_word_index.h` | `WordIndex`: count on first use, and only once |
| 8 | `ch02_d08` | `exercises/d08_parse_endpoint.h` | `parse_endpoint`: `host:port`, or the reason it is not |

**Drill 1: `find_index`.** This is chapter 0's sentinel problem, solved. The parameter is the
`std::span<const int>` of chapter 1. Look at the second test: a hit at index 0.

**Drill 2: `parse_uint`.** Use `std::from_chars` from `<charconv>`: it takes a pointer to the first
character, a pointer behind the last, and a variable to fill, and returns a struct with the position where
it stopped and an error code (`std::errc{}` means none). Take that struct apart with a structured binding.
`from_chars` needs no terminator, no locale and no allocation, which makes it the right parser for a
`std::string_view`. It stops at the first character that does not belong to a number and is content with
that; whether `"12x"` is a number is your decision, and the tests have made it.

**Drill 3: `parse_bool`.** Three outcomes in a type that has room for exactly three. The test to read is
`FalseIsAnAnswer`.

**Drill 4: `port_or`.** `lookup` is given and returns an optional view of the entry; your `parse_uint` turns
a view into an optional number. Chain them. The fallback is a function that is expensive to call, and the
tests count the calls: `value_or(fallback())` fails two of them.

**Drill 5: `validate`.** The return type is an optional *complaint*: read the comment on the stub for which
case is the good one. This is the shape of the logger's `StartWorkerLocked`.

**Drill 6: `Mailbox`.** The stub has no data members: deciding what a mailbox must remember is the first
step. `Take` has to hand over the message and leave the mailbox empty, in a way that makes the *second*
`Take` return nothing rather than an empty message.

**Drill 7: `WordIndex`.** The counter is slow, so the count is computed when first asked for and then
remembered. The last test is the reason this drill is in this chapter: a text with zero words.

**Drill 8: `parse_endpoint`.** The same kind of function as drill 2, with a caller who wants to know what
was wrong. `Endpoint`, `ParseError` and the return type are given. Return a failure with
`std::unexpected(ParseError::...)`. Your `parse_uint` is included.

## Design problems

**Problem 1: the average of an hour of readings** (`ch02_p01`, you create `exercises/p01_readings.h`).
A weather station asks its temperature sensor once a minute, and sometimes the sensor delivers nothing.
Write a class `ch02::Readings` that is told about each minute, with `Add(value)` for a reading that was
delivered and `AddMissing()` for a minute without one, and that reports the average of the delivered
readings with `Average()`. The caller must be able to tell an hour whose average was exactly 0.0 degrees
from an hour in which nothing was delivered.

The tests use the result of `Average()` in two ways only: as a condition, and with `*`.

**Problem 2: a staff directory** (`ch02_p02`, you create `exercises/p02_directory.h`).
A directory maps numeric ids to profiles. A profile has a name; some people also have a nickname, and
everyone except the person at the top has a manager, given by id. Any id can occur, including 0. Write a
struct `ch02::Profile` with members `name`, `nickname` and `managerId` in that order, and a class
`ch02::Directory` with

- `Add(id, profile)`;
- `DisplayName(id)`: the nickname if there is one, otherwise the name, and `<unknown #id>` for an id that
  is not in the directory;
- `ManagerName(id)`: the display name of that person's manager, and `nobody` whenever a link in that chain
  is missing: the id is unknown, the person has no manager, or the manager's id is not in the directory.

The tests build profiles with designated initialisers and write `{}` for a nickname or a manager that does
not exist.

**Problem 3: one line of a configuration file** (`ch02_p03`, you create `exercises/p03_load_port.h`).
`ch02::load_port(line)` reads a line of the form `port = N`, with any blanks around the key and the number,
and gives the caller the port. The user who wrote the file is shown one of three messages when that is not
possible, so the caller must be able to tell these cases apart:

| The line | Message |
| --- | --- |
| is not a setting of `port` at all | `this line does not set the port` |
| sets `port` to something that is not an unsigned number | `the port must be a number` |
| sets `port` to a number outside 1 to 65535 | `the port must be between 1 and 65535` |

Write `load_port` and `ch02::describe_failure(result)`, which returns the message for a result that is not
a port. The tests use the result of `load_port` as a condition and with `*`, and pass it to
`describe_failure`. You may include your `d02_parse_uint.h`.

## Break it on purpose

**Use the value without asking.** `experiments/x01_deref_empty.cpp`:

<!-- file: chapters/02-optional/experiments/x01_deref_empty.cpp#main -->
```cpp
const std::optional<unsigned> port = parse_port("http");  // not a number: the optional is empty

std::cout << "listening on port " << *port << std::endl;  // nobody asked
```

```sh
cmake --build --preset clang-debug -t x02_01 && build/clang-debug/bin/x02_01
```

The program is stopped by the standard library, with a message that ends in

<!-- file: chapters/02-optional/experiments/x01_deref_empty.expect -->
```
Assertion 'this->_M_is_engaged()' failed.
```

That check exists because the build defines `_GLIBCXX_ASSERTIONS`. `experiments/x02_deref_empty_unchecked.cpp`
is the same program with the checks switched off, as in an optimised release build
(`cmake --build --preset clang-debug -t x02_02 && build/clang-debug/bin/x02_02`):

<!-- file: chapters/02-optional/experiments/x02_deref_empty_unchecked.expect -->
```
listening on port
and nothing stopped me
```

Between `port` and the line break it prints a number: 0 when built with GCC 16, some other value with
Clang 23. That is the unused storage inside the optional, and the program would go on to listen on that
port. Nothing is reported: not by either compiler, not by the library, and not by AddressSanitizer, under
which the same file also runs to the end. To AddressSanitizer this is a read of memory the program owns.

clang-tidy does see it. Its check follows optionals through the function and reports any access that is
not guarded by a test:

<!-- file: chapters/02-optional/experiments/x01_deref_empty.tidy.expect -->
```
unchecked access to optional value
bugprone-unchecked-optional-access
```

**Ask with `value()` instead.** `experiments/x03_value_of_empty.cpp`:

<!-- file: chapters/02-optional/experiments/x03_value_of_empty.cpp#main -->
```cpp
const std::optional<unsigned> port;  // empty

std::cout << "listening on port " << port.value() << std::endl;  // checked: throws
```

Nobody catches the exception, so the program ends with

<!-- file: chapters/02-optional/experiments/x03_value_of_empty.expect -->
```
std::bad_optional_access
bad optional access
```

The same in every build type: `value()` is the form whose behaviour on an empty optional is defined.

**Test an `optional<bool>` as if it were a `bool`.** `experiments/x04_optional_bool.cpp`:

<!-- file: chapters/02-optional/experiments/x04_optional_bool.cpp#main -->
```cpp
const std::optional<bool> verbose = parse_bool("false");  // the user asked for quiet

if (verbose)  // true: there IS an answer
{
    std::cout << "verbose output is ON\n";
}
else
{
    std::cout << "verbose output is off\n";
}
```

<!-- file: chapters/02-optional/experiments/x04_optional_bool.expect -->
```
verbose output is ON
```

The user wrote `false`, and gets verbose output. The fix depends on what "no answer" should mean:
`if (verbose == true)` treats it as off, `if (verbose.value_or(true))` as on.

**Give `value_or` an expensive default.** `experiments/x05_value_or_eager.cpp` has an optional that holds a
value, and three ways of asking for "the value, or else the default":

<!-- file: chapters/02-optional/experiments/x05_value_or_eager.cpp#main -->
```cpp
const std::optional<unsigned> configured = 8080;  // there is a value: no default is needed

const unsigned eager = configured.value_or(load_default_port());
std::cout << "value_or: port " << eager << ", the default was fetched " << g_loads
          << " time(s)\n";

g_loads             = 0;
const unsigned lazy = configured ? *configured : load_default_port();
std::cout << "?:        port " << lazy << ", the default was fetched " << g_loads
          << " time(s)\n";

g_loads = 0;
const unsigned chained =
    configured.or_else([] { return std::optional<unsigned>(load_default_port()); }).value();
std::cout << "or_else:  port " << chained << ", the default was fetched " << g_loads
          << " time(s)\n";
```

<!-- file: chapters/02-optional/experiments/x05_value_or_eager.expect -->
```
value_or: port 8080, the default was fetched 1 time(s)
?:        port 8080, the default was fetched 0 time(s)
or_else:  port 8080, the default was fetched 0 time(s)
```

## Back in logger

The answers are in `solutions/02-optional/README.md`.

1. `StartWorkerLocked` (`src/Core.cpp:341-364`) returns `std::nullopt` when the worker thread was started and
   a string when it was not. Why does it not return `bool`? And why does it hand the message back to its
   caller, who reports it some thirty lines later, when it could call `Report` itself? (The function's
   name is a hint.) There is a third outcome in that function; find it.
2. Write `TakeStartupErrorLocked` (`src/Core.cpp:229-232`) without `std::exchange`, using `std::move`. Now
   leave out the line that you had to add. What does the *second* caller of your function get, and what does
   `SetErrorHandler` (`src/Core.cpp:207-227`) do with it?
3. `Level` already has an enumerator `Off`. Why does `parse_level` not return `Level::Off` for a text it
   cannot read, or a new enumerator `Level::Invalid`? Consider `LOGGER_LEVEL=wran`, and consider the line
   `if (L <= current_level())` in `include/logger/logger.h`.

## Checkpoint

You should be able to explain:

- what `if (opt)` asks, and two types for which that is easy to get wrong;
- what each of `*opt`, `opt.value()` and `opt.value_or(x)` does on an empty optional, and which one to use
  inside `if (opt) { ... }`;
- why `std::exchange(opt, std::nullopt)` and `std::move(opt)` leave `opt` in different states;
- when a function should return `std::expected` instead, and what the caller loses with `std::optional`.

Next: ranges. `parse_level` has one more line you have not been told about, the call to
`std::ranges::equal` with its five arguments. After the next chapter you can write the whole function
yourself.

---

Previous: [Chapter 1: Views that do not own](../01-views/README.md) | [Index](../../README.md) | Next: [Chapter 3: Ranges I](../03-ranges-algorithms/README.md)
