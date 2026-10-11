# Chapter 4: reference solutions

Compiled against the chapter's tests: `ctest --preset solutions -R sol04`.

## Drills

**Drill 1, `d01_categories.h`.** The eight answers with their reasons are in the file. The two that are
missed most often:

- **Expression 5,** `which(std::move(fixed))` with a `const` object: lvalue overload. The cast yields a
  `const Tracked&&`, which cannot bind to `Tracked&&`, and `const Tracked&` takes it.
- **Expression 6,** `which(parameter)` inside `sink(Tracked&& parameter)`: lvalue overload. The parameter's
  type says what callers may pass. Inside the function it is a name, and a name is an lvalue.

**Drill 2, `d02_buffer.h`.**

<!-- file: solutions/04-move-semantics/d02_buffer.h#copy -->
```cpp
// Copying: new storage, and every element copied.
Buffer(const Buffer& other)
  : data(other.size == 0 ? nullptr : new int[other.size]), size(other.size)
{
    std::copy_n(other.data, size, data);
}

// Copy and swap: build the copy first, then exchange contents with it. If the allocation
// throws, *this is untouched; and assigning an object to itself needs no special case.
Buffer& operator=(const Buffer& other)
{
    Buffer copy(other);
    Swap(copy);
    return *this;
}
```

Copy assignment is written as copy-and-swap. Building the copy first means that an allocation failure
leaves `*this` as it was, and `buffer = buffer` needs no test: it copies and swaps, wastefully and
correctly. The more direct version (check for self-assignment, delete, allocate, copy) is fine too, as long
as it deletes only after the new block exists.

<!-- file: solutions/04-move-semantics/d02_buffer.h#move -->
```cpp
// Moving: take the pointer, and leave the source in a state its destructor can live with.
Buffer(Buffer&& other) noexcept
  : data(std::exchange(other.data, nullptr)), size(std::exchange(other.size, 0))
{
}

Buffer& operator=(Buffer&& other) noexcept
{
    if (this != &other)
    {
        delete[] data;  // what *this owned is no longer needed
        data = std::exchange(other.data, nullptr);
        size = std::exchange(other.size, 0);
    }
    return *this;
}
```

Move assignment has one job more than the move constructor: the target owns a block already, and it has
to go, or it leaks (`asan` reports that). The self-check protects `buffer = std::move(buffer)`, which would
otherwise delete the block and then take the pointer to it. Both are `noexcept`, and the last test shows
what that buys: the vector moves its buffers when it grows.

The class exists to practise the rule of five. With a `std::vector<int>` member in place of the pointer it
needs none of the five.

**Drill 3, `d03_named.h`.** The culprit is `~Named()`. A user-declared destructor stops the compiler from
generating the move constructor and move assignment, the copy operations are still generated, and
`Named(std::move(original))` selects the copy constructor.

<!-- file: solutions/04-move-semantics/d03_named.h#five -->
```cpp
// The destructor above is user-declared, and that alone stops the compiler from generating the
// move operations: "moving" a Named then silently copies it. Asking for all four by name
// restores them. This is the rule of five in its mildest form: nothing to write, four lines to
// declare.
Named(const Named&)                = default;
Named& operator=(const Named&)     = default;
Named(Named&&) noexcept            = default;
Named& operator=(Named&&) noexcept = default;
```

Defaulting only the two move operations is not enough: declaring a move operation removes the copies, and
the last test, which copies a `Named`, would no longer compile. Hence all four.

**Drill 4, `d04_registry.h`.**

<!-- file: solutions/04-move-semantics/d04_registry.h#add -->
```cpp
/// Keeps @p item. By value: the caller decides what that costs. An lvalue argument is copied
/// into `item`, an rvalue argument is moved into it, and from there it is moved into the
/// vector.
void Add(tut::Tracked item) { items.push_back(std::move(item)); }
```

Two overloads, `Add(const Tracked&)` and `Add(Tracked&&)`, pass the tests as well and save one move for an
lvalue argument. They are what `std::vector::push_back` itself has. For a function with one parameter that
is reasonable; for a constructor with three parameters to keep it would be eight overloads, and by value
stays one function.

**Drill 5, `d05_outbox.h`.**

<!-- file: solutions/04-move-semantics/d05_outbox.h#send -->
```cpp
/// Takes @p message over. The parameter is an rvalue reference, so a caller has to give the
/// string up, visibly: Send(std::move(text)) or Send(make_text()).
void Send(std::string&& message)
{
    // `message` has a name, so it is an lvalue here, whatever its type. Without std::move this
    // line would copy.
    pending.push_back(std::move(message));
}
```

<!-- file: solutions/04-move-semantics/d05_outbox.h#drain -->
```cpp
/// Hands over everything that is waiting. The outbox is empty afterwards.
[[nodiscard]] std::vector<std::string> Drain() noexcept
{
    // exchange: the old content goes to the caller and `pending` is left in a state that is
    // known, not merely "valid but unspecified".
    return std::exchange(pending, {});
}
```

`return std::move(pending);` passes the tests too: `std::vector` is one of the few types that promise to
be empty after being move-constructed from. A `std::string` in its place would promise nothing of the kind.
`std::exchange` says what is left behind, whatever the type, and the reader does not have to know which
types make which promise.

**Drill 6, `d06_make_items.h`.** `make_items` reserves, constructs the items in place with `emplace_back`,
and returns the vector by name.

<!-- file: solutions/04-move-semantics/d06_make_items.h#take -->
```cpp
/// The finished text. `&&` after the parameter list: this function can only be called on a
/// Builder that is an rvalue, std::move(builder).Take(), which says at the call that the
/// builder is being given up.
[[nodiscard]] std::string Take() && noexcept { return std::move(text); }
```

`text` is a member, not a local, so `return text;` would copy: the automatic move in a `return` applies to
locals and parameters only. The `&&` qualifier then makes the consequence visible at the call:
`builder.Take()` on a named builder does not compile.

**Drill 7, `d07_drain_into.h`.** `std::ranges::move(from, std::back_inserter(to))` and `from.clear()`. The
`clear` is not tidying up: without it `from` still has its elements, each a moved-from string.
`to.insert(to.end(), std::make_move_iterator(from.begin()), std::make_move_iterator(from.end()))` is the
other common spelling.

**Drill 8, `d08_scoped_count.h`.**

<!-- file: solutions/04-move-semantics/d08_scoped_count.h#move -->
```cpp
// A moved-from ScopedCount holds nullptr: it has handed its duty on, and its destructor does
// nothing.
ScopedCount(ScopedCount&& other) noexcept : counter(std::exchange(other.counter, nullptr)) { }

ScopedCount& operator=(ScopedCount&& other) noexcept
{
    if (this != &other)
    {
        Release();  // whatever *this was counting stops being counted
        counter = std::exchange(other.counter, nullptr);
    }
    return *this;
}
```

The member is an `int*`, and null is the moved-from state: "I am no longer counting". The destructor and
the move assignment both go through one `Release()`, which checks for it. Copying is deleted by name, which
documents the intent; declaring the move operations would have deleted it anyway.

## Problem 1: an undo history

<!-- file: solutions/04-move-semantics/p01_history.h#store -->
```cpp
/// Takes @p snapshot over. An rvalue reference, on purpose: a snapshot is megabytes, and a
/// caller who wants to keep theirs has to write the copy where everybody can see it,
/// Store(std::string(text)).
void Store(std::string&& snapshot)
{
    if (limit == 0)
    {
        return;
    }
    if (snapshots.size() == limit)
    {
        snapshots.pop_front();  // the oldest goes
    }
    snapshots.push_back(std::move(snapshot));
}
```

**`std::string&&`, not by value.** By value would let `history.Store(document)` compile and copy megabytes
without a trace at the call. The statement asks for the opposite, and an rvalue-reference parameter is how
a function refuses lvalues. The caller who wants to keep their text writes
`history.Store(std::string(document))`, and the copy is there for every reader to see. This is the
logger's `Core::Log` signature, for the same reason.

<!-- file: solutions/04-move-semantics/p01_history.h#undo -->
```cpp
/// The most recent snapshot, which is removed from the history, or nothing when it is empty.
[[nodiscard]] std::optional<std::string> Undo()
{
    if (snapshots.empty())
    {
        return std::nullopt;
    }
    std::string latest =
        std::move(snapshots.back());  // out of the container before it is popped
    snapshots.pop_back();
    return latest;  // moved into the optional: `latest` is a local being returned
}
```

**Move out, then pop.** `return snapshots.back();` followed by a `pop_back` cannot be written, and
`auto latest = snapshots.back();` copies. The snapshot is moved out of the container into a local, the
moved-from element is removed, and the local is returned by name, which moves it into the optional.

**The container** is a `std::deque`, because the history drops from the front and adds at the back, and a
deque does both without shifting the rest. A `std::vector` with `erase(begin())` is also correct, and each
shift is a handful of cheap moves, not copies: for "the last few states" the difference does not matter.
What matters is that neither container ever copies a `std::string` it holds, because `std::string`'s move
constructor is `noexcept`.

## Problem 2: a handle that cannot leak

<!-- file: solutions/04-move-semantics/p02_handle.h#move -->
```cpp
Handle(Handle&& other) noexcept : id(std::exchange(other.id, kNone)) { }

Handle& operator=(Handle&& other) noexcept
{
    if (this != &other)
    {
        Close();
        id = std::exchange(other.id, kNone);
    }
    return *this;
}
```

This is drill 8 with a different resource: one member, a value that means "I own nothing", and a `Close()`
that both the destructor and the move assignment use.

- **Not copyable.** Two `Handle`s with the same id would release it twice. The copy operations are deleted,
  so the mistake is a compile error.
- **Movable, and `noexcept`.** That is what lets a `Handle` be returned from `Open()` and live in a vector
  that reallocates.
- **A private constructor and a factory.** `Handle::Open()` is the only way to get a `Handle` that owns
  something, so there is no `Handle` wrapped around a number somebody made up.

The other good answer is to let the standard library do it: a `std::unique_ptr` with a custom deleter can
own a handle that is a pointer, and for an `int` handle a small class like this one is the usual solution.
Once it exists, every class that needs such a resource has a `Handle` member and writes no destructor at
all. That is how the rule of five turns into the rule of zero one level up.

## Problem 3: a message builder

There are three defensible designs.

| Finishing is | After it, the builder | The compiler catches reuse |
| --- | --- | --- |
| `std::string Take()`, implemented with `std::exchange(text, {})` | is empty, and may be used for the next message | nothing to catch: reuse is allowed |
| `std::string Take() &&`, implemented with `std::move(text)` | is moved-from; the call was `std::move(builder).Take()` | no, but the `std::move` at the call says it, and clang-tidy flags a later use |
| `std::string Build() const`, returning a copy | is unchanged | nothing to catch |

The third does not meet the requirement that no copy be made. The reference takes the first:

<!-- file: solutions/04-move-semantics/p03_message_builder.h#builder -->
```cpp
/// Builds a message piece by piece.
///
/// Finishing is Take(). It hands the text over without copying it and leaves the builder EMPTY,
/// which is a defined state: the builder can be used for the next message straight away.
class MessageBuilder
{
public:
    MessageBuilder& Add(std::string_view piece)
    {
        text += piece;
        return *this;
    }

    [[nodiscard]] bool Empty() const noexcept { return text.empty(); }

    /// The message so far. The builder is empty afterwards.
    [[nodiscard]] std::string Take() noexcept { return std::exchange(text, {}); }

private:
    std::string text;
};
```

It costs nothing more than the second and leaves no state that a caller has to know not to touch. The
second, drill 6's form, is the better choice when a finished builder really should not be used again, for
example when it also holds settings that only make sense for one message. What matters is that the
comment says which one it is. A `Take()` that returns `std::move(text)` without the `&&` and without a word
about the builder's state afterwards is the design to avoid: it works, and the next caller has to guess.

## Back in logger

**1.** `AddSink` takes its `std::shared_ptr<Sink>` by value because both kinds of caller are legitimate.
`logger::add_sink<FileSink>(...)` keeps the pointer it made and returns it to the user; the copy into the
parameter is the point, since the user and the logger now share the sink. A caller who has no further use
for their pointer passes it with `std::move` and pays for no reference-count update. `Log` takes
`std::string&&` because there only one kind of caller is wanted: the message has just been formatted and
nobody else needs it. An rvalue reference makes it impossible to pass a string that would have to be
copied, on the path that every log call takes.

**2.** After `std::thread toJoin = std::move(worker);` the member `worker` owns no thread: it is the same
as a default-constructed `std::thread`, and `worker.joinable()` is false. That is one of the moved-from
states the standard does specify. `StartWorkerLocked` depends on it: `worker = std::thread(...)` assigns to
`worker`, and assigning to a `std::thread` that still owns a running thread calls `std::terminate`. The
comment at `src/Core.cpp:345` says so: "`worker` is never joinable here: it is default-constructed, or
Stop() moved from it."

**3.** `sinks.clear()` would destroy the sinks right there, while the mutex is held, and a sink's
destructor is the user's code: it may take time, and it may log, which needs the same mutex. The swap moves
all the sinks into `removed` in constant time and leaves `sinks` empty; the sinks themselves are destroyed
when `removed` goes out of scope. For that to happen *after* the lock is released, `removed` has to be
destroyed after `lock`, and locals are destroyed in the reverse order of their declaration. So `removed` is
declared first. The comment above it says the same in one line.

## Take it to your rebuild

Nothing stops compiling. `const std::string&` accepts the rvalue that `format_and_log` passes, and inside
`Core::Log` the line `.message = std::move(message),` still compiles: `std::move` of a `const` string gives
a `const std::string&&`, the move constructor cannot take it, and the copy constructor does. That is the one
copy, a silent one, on every log call. The tests pass, because nothing they check has changed. The
`std::string&&` in the real signature is what turns this mistake into one that cannot be made.
