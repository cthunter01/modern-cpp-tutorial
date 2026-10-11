# Chapter 2: reference solutions

Compiled against the chapter's tests: `ctest --preset solutions -R sol02`.

## Drills

**Drill 1, `d01_find_index.h`.** `return i;` and `return std::nullopt;`. An index of 0 converts to an
optional that holds 0, which is a different thing from an empty one.

**Drill 2, `d02_parse_uint.h`.** `std::from_chars` reports two things, and both are needed. The error code
catches an empty text, a text that does not start with a digit, and a number too large for `unsigned`. The
position catches everything `from_chars` was content to stop in front of: `"12x"`, `"12 "`, `"1.5"`.
A leading `+` or `-` is not part of an unsigned number for `from_chars`, so `"+1"` and `"-1"` fail by the
first rule. A version built on `std::stoul` accepts `" 12"`, `"12x"` and `"-1"` (which wraps round), and
throws for the rest.

**Drill 3, `d03_parse_bool.h`.** `return false;` is "the answer is false"; `return std::nullopt;` is "no
answer".

**Drill 4, `d04_port_or.h`.** `lookup(config, "port").and_then(parse_uint)` is the whole search: a missing
entry and an entry that is not a number both come out empty. The fallback is then called only on that
path, with `port ? *port : fallback()`. The same in one chain is
`...and_then(parse_uint).or_else([&] { return std::optional<unsigned>(fallback()); }).value()`, which is
correct and harder to read.

**Drill 5, `d05_validate.h`.** A sequence of checks that each `return` a message; `return std::nullopt;` at
the end. A string literal or a `std::string` converts to an engaged `std::optional<std::string>`.

**Drill 6, `d06_mailbox.h`.** The member is a `std::optional<std::string>`: "a message, or none" is exactly
what the mailbox has to remember, and an empty string is a possible message, so a plain `std::string`
cannot do it. `Take` is `return std::exchange(slot, std::nullopt);`. The tempting
`return std::move(slot);` fails `AMessageIsCollectedOnce`: the moved-from optional is still engaged.

**Drill 7, `d07_word_index.h`.** The cache is a `std::optional<std::size_t>`. A `std::size_t` initialised to
0, with "0 means not counted yet", passes three tests and fails the fourth: an empty text really has zero
words, and is counted again on every call.

**Drill 8, `d08_parse_endpoint.h`.** `rfind(':')`, because the host part may contain colons. Each way of
failing returns `std::unexpected` with its own enumerator; the success case returns an `Endpoint`, which
converts. `parse_uint` already rejects everything that is not a number, so only the range is left to check.

## Problem 1: the average of an hour of readings

<!-- file: solutions/02-optional/p01_readings.h#average -->
```cpp
/// The mean of the readings that were delivered, or nothing when none was: an average of no
/// values does not exist, and 0.0 is a temperature.
[[nodiscard]] std::optional<double> Average() const noexcept
{
    if (valid == 0)
    {
        return std::nullopt;
    }
    return sum / static_cast<double>(valid);
}
```

**The return type is the answer.** "No reading was delivered" is not a number, and every `double` you
could borrow for it is a temperature somebody might measure: 0.0, -1.0, even -273.15 on a broken sensor.
`std::optional<double>` keeps the two apart, and the caller cannot use the average without going through
the test.

**What `Readings` stores** is a sum and two counts. The statement does not ask for the individual readings,
so the reference does not keep them. Had it needed them, the natural element type would be
`std::optional<double>` again, one per minute.

**What about NaN?** A `double` has a built-in "not a number", and `Average()` could return
`std::numeric_limits<double>::quiet_NaN()` for an empty hour. It compiles, it needs no new type, and it is a
sentinel with two extra problems: nothing makes the caller check for it, and it spreads silently through
every calculation it touches, so the mistake shows up far from its cause.

## Problem 2: a staff directory

<!-- file: solutions/02-optional/p02_directory.h#profile -->
```cpp
struct Profile
{
    std::string                name;
    std::optional<std::string> nickname;   // most people have none
    std::optional<int>         managerId;  // the person at the top has none; 0 is somebody's id
};
```

**Two members that may be absent, two optionals.** The test `IdZeroIsAPersonLikeAnyOther` is there for the
design that stores `int managerId` and uses 0, or -1, for "none". With 0 the directory cannot express that
the person with id 0 manages somebody. With -1 it works until an id is negative, and every reader of the
code has to know the convention. For the nickname an empty string would do in practice; the optional says
the same without a convention.

**Lookups return a pointer.**

<!-- file: solutions/02-optional/p02_directory.h#find -->
```cpp
/// The profile of @p id, or nullptr. "A reference to something that may not exist" is a
/// pointer: std::optional cannot hold a reference before C++26.
[[nodiscard]] const Profile* Find(int id) const
{
    const auto found = profiles.find(id);
    return found != profiles.end() ? &found->second : nullptr;
}
```

The profile already exists, inside the map. "That one, or none" is what a pointer says, and it costs
nothing. `std::optional<Profile>` would copy the profile, strings included, on every lookup;
`std::optional<Profile&>` does not exist before C++26.

**`ManagerName` is a chain of three things that may be missing.**

<!-- file: solutions/02-optional/p02_directory.h#manager -->
```cpp
/// The display name of the manager of @p id, or "nobody" when @p id is unknown, has no manager,
/// or names a manager who is not in the directory.
[[nodiscard]] std::string ManagerName(int id) const
{
    const Profile* const person = Find(id);
    if (person == nullptr || !person->managerId)
    {
        return "nobody";
    }
    const Profile* const manager = Find(*person->managerId);
    return manager != nullptr ? DisplayNameOf(*manager) : "nobody";
}
```

Written with early returns it stays flat. `DisplayNameOf` is `profile.nickname.value_or(profile.name)`:
a cheap default, so `value_or` is the right tool there.

## Problem 3: one line of a configuration file

<!-- file: solutions/02-optional/p03_load_port.h#types -->
```cpp
enum class PortError : std::uint8_t
{
    NotAPortSetting,  ///< the line is not `port = ...`
    NotANumber,       ///< the value is not an unsigned number
    OutOfRange,       ///< a number, but not between 1 and 65535
};

using PortResult = std::expected<unsigned, PortError>;
```

**Three reasons for failure need a type that can carry a reason.** A `std::optional<unsigned>` would tell
the caller that there is no port and nothing more, and the user would get one vague message for three
different mistakes. `std::expected<unsigned, PortError>` is used exactly as the tests use it, `if (result)`
and `*result`, and has `.error()` for the rest.

**The reason is an enumerator, not a message.** `load_port` says *what* went wrong; `describe_failure` turns
that into words. A caller that wants to react differently (skip lines that are not about the port, stop on
a bad number) can, without comparing strings, and the wording can change without touching the parser.
Returning `std::expected<unsigned, std::string>` with the finished message is a reasonable answer too when
the message is all anybody will ever want.

**Other good answers.** A small struct with an `std::optional<unsigned> port` and a `PortError`, given
`explicit operator bool` and `operator*`, passes the tests: it is `std::expected` written by hand. Throwing
three exception types does not fit: a wrong line in a file the user edited is an expected outcome, and the
caller wants to handle it right where the call is.

## Back in logger

**1.** A `bool` would say that starting the thread failed and lose what the system said about why, which is
the part the user needs (`e.what()` is in the message). The message is not reported on the spot because of
the suffix in the function's name: `StartWorkerLocked` runs while the logger's mutex is held, and reporting
means calling the error handler, which is the user's code. The logger never calls user code under its
mutex (guide step 5). So the function returns the complaint, the caller keeps it in a local
`std::optional<std::string>`, and `Report(*startFailure)` runs after the lock has been released. An
optional is the natural type for "a complaint to deliver later, if there is one". The third outcome is in
the inner `catch`: the thread could not be started *and* the message could not be built, because
`std::format` itself failed for lack of memory. Then the function returns `std::nullopt` although something
went wrong. The state has still been set to `Exiting`, so the logger behaves correctly; only the report is
lost.

**2.**

<!-- not-compiled: a variant of a logger function, to compare with the original in src/Core.cpp -->
```cpp
std::optional<std::string> Core::TakeStartupErrorLocked() noexcept
{
    std::optional<std::string> taken = std::move(startupError);
    startupError.reset();
    return taken;
}
```

Without the `reset()`, `startupError` is still engaged after the move: it holds a moved-from, empty string.
The first caller gets the complaint. The second caller gets an optional that tests as true and holds `""`,
and `SetErrorHandler` reports that empty text to the newly installed handler as if it were a complaint about
`LOGGER_LEVEL`. `std::exchange(startupError, std::nullopt)` does the taking and the emptying in one
expression, which is why there is nothing to forget.

**3.** `Level::Off` is a real setting with a real effect. If `parse_level` returned it for anything it
could not read, `LOGGER_LEVEL=wran` would silently switch all logging off, the opposite of what the person
who made the typo wanted, with no way for the caller to notice. A new enumerator `Level::Invalid` would be
a sentinel inside the type: `Level` is ordered, the threshold test is `L <= current_level()`, and `Invalid`
would have to sit somewhere in that order and be handled in `level_name`, in every `switch`, and in
`set_level`. `std::optional<Level>` keeps "not a level" out of `Level` altogether: every `Level` in the
program is a valid one.
