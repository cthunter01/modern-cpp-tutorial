# Chapter 5: reference solutions

Compiled against the chapter's tests: `ctest --preset solutions -R sol05`.

## Drills

**Drill 1, `d01_count_and_sum.h`.** `sizeof...(Ts)` and `(0 + ... + values)`. The `0 +` is what makes the
sum of nothing well defined; `(... + values)` alone does not compile for an empty pack.

**Drill 2, `d02_join_with.h`.** The reference takes the first item as a parameter of its own:

<!-- file: solutions/05-forwarding/d02_join_with.h -->
```cpp
#pragma once

#include <sstream>
#include <string>
#include <string_view>

namespace ch05
{

/// With nothing to join, the result is empty.
[[nodiscard]] inline std::string join_with(std::string_view /*separator*/)
{
    return {};
}

/// The arguments written one after the other, as operator<< writes them, with @p separator between
/// them.
template <typename First, typename... Rest>
[[nodiscard]] std::string join_with([[maybe_unused]] std::string_view separator, const First& first,
                                    const Rest&... rest)
{
    std::ostringstream out;
    out << first;
    // A fold over the comma operator: "do this for each element of the pack, in order". Each of the
    // remaining items is written with the separator in front of it. With one item there is no rest,
    // the fold expands to nothing, and `separator` is not used: hence the attribute.
    ((out << separator << rest), ...);
    return out.str();
}

}  // namespace ch05
```

"First, then separator and item for each of the rest" needs no flag. The empty case is an ordinary
overload. The other approach keeps one pack and a `bool`:
`((out << (std::exchange(first, false) ? "" : separator) << items), ...);`. It is correct, and for the
call with no items both `first` and `separator` end up unused in that one instantiation, which the
tutorial's warning set reports. `[[maybe_unused]]` is the attribute for "unused in some instantiations";
the logger's `log_at` has it on both parameters because of `if constexpr`.

**Drill 3, `d03_deduce.h`.** The answers and their reasons are in the file. Part 2, declaration 4, is the
one to remember: `template <typename T> struct Box { void Push(T&& x); };` is not a forwarding reference.
`T` belongs to the class and was fixed when `Box<int>` was named; at the call of `Push` nothing is deduced.

**Drill 4, `d04_is_one_of.h`.** `((value == candidates) || ...)`. A fold over `||` is `false` for an empty
pack, so no starting value is needed.

**Drill 5, `d05_call_logged.h`.** `decltype(auto)` as the return type and
`return std::invoke(std::forward<F>(function), std::forward<Args>(args)...);`. The test file is written so
that each shortcut stops it from compiling, at a line that shows what the shortcut costs:

| Shortcut | The line in the tests that no longer compiles, and why |
| --- | --- |
| return type `auto` | the `static_assert` in `AReferenceResultStaysAReference`: an `int&` result has become an `int` |
| `args...` without `std::forward` | the call with `std::make_unique<int>(9)`: the argument arrives as an lvalue and would have to be copied |
| `std::move(args)...` | the call with `counter` and a function taking `int&`: an rvalue does not bind to it |
| parameters `Args... args`, by value | the same call: `std::forward<int>` of a by-value parameter is an rvalue too |

Where the last two do compile they are worse than a compile error: `std::move` empties the caller's
variables (the first experiment), and by-value parameters let the function work on copies.

`std::forward<F>(function)` matters for callables whose call operator is different for rvalues; it is rare
and costs nothing to get right.

**Drill 6, `d06_fixed_stack.h`.**

<!-- file: solutions/05-forwarding/d06_fixed_stack.h#emplace -->
```cpp
/// Constructs a new top element from @p args, in place, and returns it. The stack must not be
/// full.
template <typename... Args>
T& Emplace(Args&&... args)
{
    // The arguments travel on to T's constructor untouched; no T is built here and then moved.
    T& created = slots.at(count).emplace(std::forward<Args>(args)...);
    ++count;
    return created;
}
```

`slots.at(count) = T(std::forward<Args>(args)...)` passes the first test and fails the second: it builds a
`T` and moves it into the slot. `emplace` builds the `T` in the slot's own storage.

**Drill 7, `d07_make_drawn.h`.**

<!-- file: solutions/05-forwarding/d07_make_drawn.h#make-drawn -->
```cpp
/// Constructs a shape of type S from @p args, adds it to @p canvas and returns it:
///     auto circle = make_drawn<Circle>(canvas, 2.0);
template <typename S, typename... Args>
std::shared_ptr<S> make_drawn(Canvas& canvas, Args&&... args)
{
    std::shared_ptr<S> shape = std::make_shared<S>(std::forward<Args>(args)...);
    canvas.shapes.push_back(shape);  // a copy of the pointer, converted to shared_ptr<Shape>
    return shape;                    // the caller gets the exact type back
}
```

The logger's `add_sink`, with one difference in the middle: `push_back(shape)` converts the
`shared_ptr<S>` to a `shared_ptr<Shape>` implicitly, where the logger writes
`std::shared_ptr<Sink>(sink)` because it calls an overloaded function and wants to name the overload.

**Drill 8, `d08_label.h`.**

<!-- file: solutions/05-forwarding/d08_label.h#constrained -->
```cpp
// Without the requires clause this constructor accepts *everything*, including another Label,
// and for a non-const Label it is a better match than the copy constructor. The clause says
// what the constructor is for, and the compiler then leaves it out of every other case.
template <typename U>
    requires std::constructible_from<std::string, U>
explicit Label(U&& value) : text(std::forward<U>(value))
{
}
```

`std::constructible_from<std::string, U>` says exactly what the constructor does with its argument. With
it, `Label copy(original);` no longer finds the template viable (a `std::string` cannot be constructed from
a `Label&`) and falls back to the copy constructor. The other common constraint,
`requires (!std::same_as<std::remove_cvref_t<U>, Label>)`, only carves out the one case that hurt; it
fixes the copy and still claims that a `Label` can be built from a `double`.

**Drill 9, `d09_tee.h`.** A `std::tuple<Sinks&...>` member, initialised from the constructor's pack, and
`std::apply([&](Sinks&... each) { (each.Write(line), ...); }, sinks);`. The lambda's parameters are a pack
again, and the fold over the comma calls the sinks in order. The capture is `[&]` and not `[line]` because
with no sinks the fold is empty and Clang warns that `line` is captured and not used.

## Problem 1: try again

<!-- file: solutions/05-forwarding/p01_retry.h#retry -->
```cpp
/// Calls `operation(args...)` up to @p attempts times, until it returns true. Returns whether it
/// did.
///
/// The arguments are passed to every attempt as lvalues, whatever the caller passed: an attempt may
/// fail, and the next one needs them intact. So nothing is forwarded here, on purpose.
template <typename F, typename... Args>
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward): on purpose, as the comment above says
bool retry(int attempts, F&& operation, Args&&... args)
{
    for (int attempt = 0; attempt < attempts; ++attempt)
    {
        if (std::invoke(operation, args...))  // not std::forward<Args>(args)...: see above
        {
            return true;
        }
    }
    return false;
}
```

**The answer is what is missing.** The signature is the forwarding one, `Args&&... args`, because that is
what binds to anything without copying. The body does not forward. `std::forward<Args>(args)...` would hand
an rvalue argument to the first attempt to be moved from, and the second attempt would get the remains: the
test `AnArgumentTheCallerGaveUpIsStillWholeOnTheSecondAttempt` is about that. Passing `args...` gives every
attempt lvalues. An operation that takes a parameter by value then copies it, once per attempt, which is
correct: it consumes its argument, and there has to be one to consume each time.

clang-tidy's `cppcoreguidelines-missing-std-forward` flags this function, and it is right to ask. The
`NOLINT` in the file carries the answer.

**Also good:** `const Args&... args`, if operations never need to modify an argument. The statement says
they may.

## Problem 2: one event type for everything

<!-- file: solutions/05-forwarding/p02_event.h#event -->
```cpp
/// Something that happens, and carries values of the types Ts to everyone who asked to be told.
template <typename... Ts>
class Event
{
public:
    using Listener = std::function<void(const Ts&...)>;

    void Subscribe(Listener listener) { listeners.push_back(std::move(listener)); }

    /// Tells every listener, in the order they subscribed. Each of them sees the same, intact
    /// values: they are passed by const reference, never forwarded, because the second listener
    /// needs them too.
    void Emit(const Ts&... values) const
    {
        for (const Listener& listener : listeners)
        {
            listener(values...);
        }
    }

private:
    std::vector<Listener> listeners;
};
```

- **A class template with a pack:** `Event<int, int>`, `Event<std::string>`, `Event<>`.
- **`Emit` takes `const Ts&...` and forwards nothing.** This is problem 1 again: there may be several
  listeners, and each one needs the values. A design that forwards to the listeners, or moves into the
  first one, fails `EveryListenerSeesTheWholeValue`.
- **Listeners are `std::function<void(const Ts&...)>`.** A listener that declares its parameter by value
  can still be stored: it is called with a const reference and makes its own copy, which is that
  listener's choice.

A template member `template <typename... Us> void Emit(Us&&... values)` is the over-engineered answer: it
accepts more at the call and then has to do exactly what this version does.

## Problem 3: time anything

<!-- file: solutions/05-forwarding/p03_timed.h#timed -->
```cpp
/// Calls `function(args...)` and returns what it returns, a reference as a reference and nothing as
/// nothing. "<label>: begin" is appended to @p timings before the call and "<label>: end" after it,
/// also when the call throws.
template <typename F, typename... Args>
decltype(auto) timed(std::vector<std::string>& timings, std::string_view label, F&& function,
                     Args&&... args)
{
    const detail::Stamp stamp(timings, label);  // its destructor writes the "end" entry
    return std::invoke(std::forward<F>(function), std::forward<Args>(args)...);
}
```

**The difficulty is "after the call".** A first attempt is: record begin, call, store the result, record
end, return the result. That breaks three ways: a `void` result cannot be stored in a variable, a reference
result needs care to stay a reference, and when the function throws, the end is never recorded.

An object whose destructor records the end solves all three at once. The `return` statement evaluates the
call, then the local `stamp` is destroyed, then the function returns: "end" is written after the call in
every case, including an exception. Nothing is stored, so `void` and references need no special treatment
beyond `decltype(auto)`. This is the same idea as `std::scoped_lock`, and as drill 8 of chapter 4: put what
must happen at the end into a destructor.

Here the arguments *are* forwarded, unlike in problems 1 and 2: the function is called once.

## Back in logger

**1.** The caller writes `S` (`add_sink<FileSink>`); `Args` is deduced from the arguments. With
`Args... args`, by value, every argument would be copied or moved into `add_sink` first and then passed on
as an lvalue: a second copy for everything, and a sink whose constructor takes a move-only argument could
not be created at all. With `const Args&... args` nothing is copied on the way in, but everything arrives
at the sink's constructor as a const lvalue: rvalues the caller gave up are copied there instead of moved,
and again a move-only argument does not compile. The return type is `std::shared_ptr<S>` so that the caller
keeps the exact type: at `src/main.cpp:99` the demo keeps `counter`, a `std::shared_ptr<CountingSink>`, and
later calls a member function on it that `Sink` does not have.

**2.** `L` cannot be deduced: it is not the type of any function argument, it is a value, chosen by which
of `error`, `warn`, `info`... was called. `Args` could have been deduced from `args`, and is written out
only because `L` comes before it in the template parameter list and is given explicitly. With `Args` given
explicitly, `Args&&` in `log_at` is not a forwarding reference in the strict sense: nothing is deduced
there. It works because `Args` was deduced one level up, in `info`, by real forwarding references, and so
already carries the reference: for an lvalue `std::string` argument `Args` is `std::string&`, and
`std::string& &&` collapses to `std::string&`. The parameter types of `log_at` come out exactly as those of
`info`, and `std::forward<Args>` does the right thing in both.

**3.** `Args` is one type, `std::string&`. `name` is never copied: `info` takes it by reference, forwards
the reference to `log_at`, which forwards it to `format_and_log`, which forwards it to `std::format`, which
reads the characters and writes them into the message. The comment in `include/logger/logger.h` says it:
"arguments are taken by reference and never stored". What is stored, and moved from thread to thread, is
the formatted message.

## Take it to your rebuild

<!-- not-compiled: a declaration to put into your copy of the logger's logger.h -->
```cpp
template <typename S, typename... Args>
    requires std::derived_from<S, Sink> && std::constructible_from<S, Args...>
std::shared_ptr<S> add_sink(Args&&... args)
```

With it, `logger::add_sink<int>(42)` is rejected at the call. GCC says there is no matching function for
`add_sink<int>(int)`, lists the one candidate, and gives the reason in words: `constraints not satisfied`,
`required for the satisfaction of 'derived_from<S, logger::Sink>' [with S = int]`. Without the constraint
the same mistake is reported at `logger.h:57`, the line that converts `shared_ptr<int>` to
`shared_ptr<Sink>`.

`logger.h` only forward-declares `Sink`, and the constraint works all the same: it is checked at each call,
where `S` is a complete class and its base class with it. The rest of the logger compiles unchanged.
