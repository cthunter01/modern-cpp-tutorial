# Chapter 4: Move semantics and value categories

Follow one message through the logger. It is formatted on the calling thread into a `std::string`. That
string is handed to `log_message`, which hands it to `Core::Log`, which puts it into a `Record`, which goes
into the queue, which the worker takes over whole, and at the end a sink writes the characters to a file.
Six stations, on two threads. The characters are written once, when the message is formatted, and never
copied afterwards. Every hand-over is a move.

The guide uses `std::move` 87 times and says what it does in half a sentence. This chapter is what stands
behind those 87: what a move is, who decides whether one happens, and the handful of rules that keep a move
from quietly becoming a copy.

## Goal

- You can say, for any expression, whether a copy or a move will be made from it, and why.
- You write `std::move` where it is needed and nowhere else, and you know the places where it does nothing.
- You can give a class correct copy and move operations, and you know when to write none at all.
- You choose between `T`, `const T&` and `T&&` for a parameter on purpose.

## Where you met it

Step 5 of the guide. The message leaves the function that formatted it:

<!-- logger: include/logger/logger.h:112 -->
```cpp
log_message(level, fmt.location, std::move(message));
```

It arrives in `Core::Log`, whose third parameter is a `std::string&& message`, and goes into a record:

<!-- logger: src/Core.cpp:257-262 -->
```cpp
Record record{
    .timestamp = {},
    .level     = level,
    .location  = location,
    .message   = std::move(message),
};
```

and the record goes into the queue (step 7):

<!-- logger: src/Core.cpp:283-284 -->
```cpp
record.timestamp = std::chrono::system_clock::now();
queue.push_back(std::move(record));
```

Three `std::move`s, three different situations: a local the function is done with, a parameter that is an
rvalue reference, and a local being put into a container. By the end of the chapter you can say why each one
is there and what would happen without it.

## The idea

### A move takes over what a copy would duplicate

A `std::string` of 40 characters is a small object that holds a pointer to a block of memory with the
characters in it. Copying the string means getting a second block and copying 40 characters. Moving it means
copying the pointer and telling the old string that the block is no longer its own.

<!-- file: chapters/04-move-semantics/examples/e01_moves.cpp#copy-and-move -->
```cpp
std::string original(40, 'x');  // 40 characters, in a block on the heap
const char* block = original.data();

std::string copy = original;  // a second block; 40 characters copied
std::string moved =
    std::move(original);  // no new block: `moved` took over the one `original` had

std::cout << (copy.data() == block) << ' ' << (moved.data() == block) << '\n';  // false true
```

The same holds for every type that owns something through a pointer: `std::vector`, `std::unique_ptr`,
`std::function`, a file stream, and any struct made of those. A move costs a few machine words whatever the
size of the content. For a type with nothing behind a pointer, an `int` or a struct of `double`s, there is
nothing to take over, and a move is a copy.

### Who decides: the value category of the argument

There is no "move" instruction. A class that can be moved has two constructors that take another object of
its kind, and ordinary overload resolution picks one:

<!-- file: chapters/04-move-semantics/examples/e01_moves.cpp#overloads -->
```cpp
// What "moving" is underneath: overload resolution. A class offers two ways of being constructed
// from another object of its kind, and the compiler picks by whether the argument can still be
// needed.
struct Text  // NOLINT(cppcoreguidelines-special-member-functions): two constructors are the example
{
    explicit Text(std::string_view content) : characters(content) { }

    Text(const Text& other) : characters(other.characters)
    {
        std::cout << "copied\n";
    }  // from an lvalue
    Text(Text&& other) noexcept : characters(std::move(other.characters))
    {
        std::cout << "moved\n";
    }  // from an rvalue

    std::string characters;
};
```

`const Text&` binds to anything. `Text&&`, an **rvalue reference**, binds only to an *rvalue*: an
expression whose value nobody can refer to again. When both fit, the rvalue reference wins. So the question
"copy or move?" is the question "is the argument an lvalue or an rvalue?", and that is a property of the
expression, decided by the compiler before any code runs:

- An **lvalue** is something with an identity that can be used again: a variable, a parameter, a member,
  an element `v[i]`, `*pointer`. Rule of thumb: it has a name, or you could take its address.
- An **rvalue** is something that is about to disappear: a temporary such as `Text("x")` or `a + b`, and
  the result of a function that returns by value.

<!-- file: chapters/04-move-semantics/examples/e01_moves.cpp#who-decides -->
```cpp
Text named("named");

const Text a(named);             // copied: `named` has a name and may be used again
const Text b(std::move(named));  // moved:  the caller says it will not be
const Text c(make_text());       // neither: the result of the call is built directly as `c`
```

That prints `copied` and `moved`, and nothing for the third line. `named` is an lvalue, so `a` is a copy.
The result of `make_text()` is an rvalue already, and since C++17 it is not even moved: the object is
created directly in `c`.

### `std::move` is a cast

The middle line is the interesting one. `named` is an lvalue; the programmer knows it will not be used
again and wants it treated as an rvalue. `std::move(named)` does exactly that and nothing else: it is a
cast to an rvalue reference. It generates no code and moves nothing.

<!-- file: chapters/04-move-semantics/examples/e01_moves.cpp#cast -->
```cpp
std::string name = "Ada Lovelace";

std::string&& sameObject = std::move(name);  // a cast, and a reference bound to the result
std::cout << name << " / " << sameObject
          << '\n';  // the name, twice: no constructor ran, nothing moved

// What std::move(name) is short for. This line does move, because here a std::string is
// constructed from the result, and the constructor that takes std::string&& is the one that
// fits.
const std::string taken = static_cast<std::string&&>(name);
```

The move happens, if it happens, in whatever receives the result: a constructor or an assignment operator
that has an overload for rvalues. `std::move` only makes that overload eligible. A better name would have
been `rvalue_cast`, and half the mistakes in this chapter come from reading the real name literally.

### What the receiving class does

The move constructor has two jobs: take the resource over, and leave the source in a state its destructor
can cope with. `std::exchange` from chapter 0 does both in one expression. This is the reference solution of
drill 2:

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

After the move the source holds a null pointer, and `delete[]` on a null pointer does nothing. Without
that, both objects would free the same block.

## The details that bite

### `std::move` on a `const` object copies, silently

`std::move` of a `const Text` yields a `const Text&&`. The move constructor wants a `Text&&` and cannot take
it, because moving modifies the source. The copy constructor's `const Text&` binds to anything, so it is
chosen. The code compiles, the word `move` is there on the page, and a copy is made. Declaring things
`const` is a good habit; an object you intend to move from is the exception.

### A named rvalue reference is an lvalue

Inside `void keep(std::string&& text)`, the parameter `text` has a name. It can be used again, as often as
the function likes. So the *expression* `text` is an lvalue, although the *type* of `text` is "rvalue
reference to string". The type decides what a caller may pass; the name decides how the function's own code
treats it. That is why `Core::Log` and every function like it says `std::move(message)` again: the caller's
`std::move` got the string into the parameter, and a second one is needed to get it out.

### A moved-from object is valid, and you know nothing else about it

After `b = std::move(a)`, `a` is still an object. Its destructor will run, and it can be assigned a new
value. Beyond that the standard library promises only that its types are in a "valid but unspecified"
state, with a few exceptions it spells out: a moved-from `std::unique_ptr` is null, a moved-from
`std::thread` owns no thread, and a `std::vector` that another vector was move-constructed from is empty. A
moved-from `std::string` is usually empty and is not required to be. A moved-from `std::optional` still
holds a value, a moved-from one (chapter 2). A moved-from `int` is unchanged.

The rule: after moving from an object, assign to it or let it go. If you need the source to be in a
particular state, do not rely on the move; say what the state should be:

<!-- file: chapters/04-move-semantics/examples/e01_moves.cpp#transfer -->
```cpp
std::vector<std::string> queue{"first", "second", "third"};

// Three ways to take everything out of `queue`.
const std::vector<std::string> a = std::move(queue);  // queue: moved-from (for a vector: empty)
queue                            = {"fourth"};        // assigning to it is always fine

const std::vector<std::string> b = std::exchange(queue, {});  // queue: empty, by definition

std::vector<std::string> c;
queue = {"fifth", "sixth"};
c.swap(queue);  // queue: has what `c` had (nothing), and keeps
                // no storage of its own; `c` has the elements
```

`std::exchange(queue, {})` and `swap` say what is left behind, for any type. The logger uses `swap` for both of its
wholesale transfers: `batch.swap(queue)` when the worker takes the queue, and `removed.swap(sinks)` in
`ClearSinks`.

### `return std::move(local)` makes things worse

A local variable returned by name is treated as an rvalue automatically: it is moved, and more often the
compiler constructs it directly in the caller's object and no move happens at all.

<!-- file: chapters/04-move-semantics/examples/e01_moves.cpp#return-local -->
```cpp
std::vector<std::string> split_words(std::string_view text)
{
    std::vector<std::string> words;
    while (!text.empty())
    {
        const std::size_t space = text.find(' ');
        words.emplace_back(text.substr(0, space));
        text.remove_prefix(space == std::string_view::npos ? text.size() : space + 1);
    }
    return words;  // no std::move: a local returned by name is moved at worst, and usually not even
                   // that
}
```

Writing `return std::move(words);` cannot improve on that, and it forbids the second, better outcome. Both
compilers warn about it. `std::move` in a `return` is right in exactly one common case: when you return a
*member* or a *parameter that is an rvalue reference*, which are not locals.

### What the compiler generates, and what stops it

A class that declares none of the five special member functions gets all of them, each working member by
member. That is the **rule of zero**, and it is the normal case:

<!-- file: chapters/04-move-semantics/examples/e01_moves.cpp#rule-of-zero -->
```cpp
// No destructor, no copy or move operations declared: the compiler generates all of them, and each
// one copies or moves member by member. A struct of movable members is movable.
struct Order
{
    std::string              customer;
    std::vector<std::string> items;
    int                      priority = 0;
};
```

`Order` can be copied and moved, correctly, with no code. The rules that take this away are easy to trip
over:

| You declare | The compiler then |
| --- | --- |
| nothing | generates destructor, both copies and both moves |
| a destructor, a copy constructor or a copy assignment | still generates the copies, and **no moves**: "moving" such an object copies it |
| a move constructor or a move assignment | generates **no copies**: the type is move-only unless you add them |

The second row is the trap: add a destructor that only logs something, and every `std::move` of that class
silently turns into a copy. If you must declare one of the five, declare all five, with `= default` where
the generated one is right. That is the **rule of five**. The logger's `Core` and `Sink` take the third
option and delete all four copy and move operations: there is one core, and a sink is used through a
pointer.

A move constructor should be `noexcept`. `std::vector` needs to relocate its elements when it grows, and it
moves them only if the move constructor promises not to throw; otherwise it copies, so that a failure
half-way leaves the old storage intact. A move that is not `noexcept` is therefore a copy inside every
vector.

## Parameters that keep their argument

A function that stores what it is given (a constructor, a setter, `push_back`) has three choices, and the
logger uses two of them on purpose.

| Parameter | An lvalue argument costs | An rvalue argument costs | Use when |
| --- | --- | --- | --- |
| `const T&`, then copy | one copy | one copy | never, for a parameter that is kept |
| `T` by value, then `std::move` | one copy, one move | one move | the default for a parameter that is kept |
| `T&&`, then `std::move` | does not compile | nothing but the move into place | the function must never copy, or `T` is expensive to move |

<!-- file: chapters/04-move-semantics/examples/e01_moves.cpp#sink-by-value -->
```cpp
class Player
{
public:
    // A parameter the function keeps: take it by value, then move it into place.
    explicit Player(std::string playerName) : name(std::move(playerName)) { }

    void Rename(std::string newName) { name = std::move(newName); }

    [[nodiscard]] const std::string& Name() const noexcept { return name; }

private:
    std::string name;
};
```

<!-- file: chapters/04-move-semantics/examples/e01_moves.cpp#sink-callers -->
```cpp
std::string chosen = "a name that is long enough to need the heap";

const Player first(chosen);  // the caller keeps `chosen`: one copy, into the parameter
const Player second(std::move(chosen));  // the caller gives it up: no copy at all
const Player third("Grace");  // a literal: the string is built once, in the parameter
```

By value lets the *caller* choose: keep the argument and pay for a copy, or give it up with `std::move` and
pay for nothing. `Core::AddSink(std::shared_ptr<Sink> sink)` is this form. `T&&` gives the caller no choice:
`Core::Log(..., std::string&& message)` can only be called with a string that is being given up, so a
message can never be copied by accident on the logger's hottest path.

## When to reach for it, and when not

- **Write `std::move(x)`** when `x` is a local or a parameter that you are passing on for the last time, to
  something that will keep it.
- **Do not write it** on a `const` object, on the name in `return name;` for a local, on a temporary (it is
  an rvalue already), or on an argument the callee only reads.
- **Never touch a moved-from object** except to assign to it or destroy it. clang-tidy's
  `bugprone-use-after-move` finds the places where you did.
- **Write no special member functions** if the members can look after themselves. A class that needs a
  destructor usually wants a member that has one instead: `std::unique_ptr`, `std::vector`, a small handle
  class like the one in design problem 2.
- **Mark move operations `noexcept`**, and let `= default` do it for you where it can.

## Drills

Run each with `ctest --preset clang-debug -R ch04_dNN`, and drills 2 and 8 with `--preset asan` as well.
`tut::Tracked`, from `support/tut/tracked.h`, is a small type that counts how often it is copied and moved;
the tests read those counts.

| # | Target | File | Do |
| --- | --- | --- | --- |
| 1 | `ch04_d01` | `exercises/d01_categories.h` | predict the overload chosen for eight expressions |
| 2 | `ch04_d02` | `exercises/d02_buffer.h` | write the copy and move operations of a `Buffer` |
| 3 | `ch04_d03` | `exercises/d03_named.h` | find out why moving a `Named` copies, and repair it |
| 4 | `ch04_d04` | `exercises/d04_registry.h` | change `Registry::Add` so that giving an item up costs no copy |
| 5 | `ch04_d05` | `exercises/d05_outbox.h` | `Outbox::Send` and `Drain`: strings in and out, never copied |
| 6 | `ch04_d06` | `exercises/d06_make_items.h` | return a vector, and a `Take()` that only works on an rvalue |
| 7 | `ch04_d07` | `exercises/d07_drain_into.h` | move every string of one vector into another |
| 8 | `ch04_d08` | `exercises/d08_scoped_count.h` | a move-only handle that counts up while it lives |

**Drill 1: categories.** No code to write. `exercises/which.h` has two overloads of `which`, one for
lvalues and one for rvalues. For each of eight calls, write into the table which one is chosen. Decide all
eight before you run the tests; the ones you get wrong are the ones to reread a section for.

**Drill 2: `Buffer`.** A class that owns a `new int[]` block. Constructor, destructor and accessors are
given; write the copy constructor, the copy assignment, the move constructor and the move assignment. The
comment on the class says what state a moved-from `Buffer` is in, and the tests check it. Copy assignment
has to cope with `buffer = buffer`, and both assignments have to release what the target owned before. Run
it under `asan`: it reports a block freed twice and a block never freed, and an ordinary build does neither.

**Drill 3: `Named`.** This class works and its tests fail: `std::move` on a `Named` copies its payload.
Nothing in the class says "copy". Find the line that causes it, and repair the class without removing that
line.

**Drill 4: `Registry::Add`.** Also working code: `Add` takes a `const Tracked&` and copies every item,
including those the caller passed with `std::move`. Change it so that a caller who gives an item up pays for
no copy, and a caller who keeps theirs pays for exactly one.

**Drill 5: `Outbox`.** The logger's `Core::Log` and the worker's `batch.swap(queue)` in miniature. `Send`
takes a `std::string&&` and keeps it; `Drain` hands over everything and leaves the outbox empty. The first
test checks, by the address of the characters, that the string that comes out is the one that went in.

**Drill 6: `make_items` and `Builder::Take`.** Return a vector of `Tracked` without a copy. Then `Take`,
whose declaration ends in `&&`: a member function that can only be called on an rvalue,
`std::move(builder).Take()`, and that hands the builder's text over. A member is not a local, so this is
the case where `std::move` in a `return` is right.

**Drill 7: `drain_into`.** `std::ranges::move` (the algorithm, from `<algorithm>`, not the cast) moves each
element of a range to an output. Afterwards `from` must be empty, not full of moved-from strings.

**Drill 8: `ScopedCount`.** An object that adds one to a counter when it is created and takes one off when
it is destroyed, and that can be moved. The stub has no data member. The question to answer first: what
does a `ScopedCount` hold after it has been moved from, so that its destructor knows to do nothing?

## Design problems

**Problem 1: an undo history** (`ch04_p01`, you create `exercises/p01_history.h`).
An editor keeps the last few states of a document for undo. Each state is the whole text, megabytes of it.
Write a class `ch04::History`, constructed with the number of states to keep. `Store(snapshot)` adds a
state; when the history is full, the oldest one is dropped. `Undo()` removes the most recent state and gives
it to the caller, or reports that there is none.

Two requirements beyond that. No state may ever be duplicated: not when it is stored, not while it is in
the history, not when it is handed back. And a caller who wants to keep their own copy of what they store
must have to say so, in a way that is visible in their code; passing a named string to `Store` must not
compile.

**Problem 2: a handle that cannot leak** (`ch04_p02`, you create `exercises/p02_handle.h`).
`exercises/handles.h` is given. It stands for an operating-system API of the usual shape:
`int ch04::os::acquire()` opens a resource and returns its number, and `void ch04::os::release(int)` gives
it back. Every handle must be released exactly once; releasing one twice corrupts a real system.

Write a class `ch04::Handle` that makes this impossible to get wrong. `Handle::Open()` acquires a handle
and returns the object that owns it; `Id()` returns its number. Whatever happens to the object afterwards
(it goes out of scope, it is returned from a function, it is put into a `std::vector` that grows, another
handle is assigned over it), the underlying handle is released exactly once, and there are never two objects
owning the same one.

**Problem 3: a message builder** (no test).
A `MessageBuilder` collects pieces of text with `Add`. When the message is complete, the caller wants it as
a `std::string`, without a copy of the text being made. Design the function that finishes the message, and
decide what state the builder is in afterwards: is using it again an error that the compiler catches, an
error that it does not catch, or something that works? Write the class, and write the sentence in its
comment that tells a caller which. `solutions/04-move-semantics/README.md` compares the possible answers.

## Break it on purpose

**Move from something that is `const`.** `experiments/x03_move_from_const.cpp`:

<!-- file: chapters/04-move-semantics/experiments/x03_move_from_const.cpp#main -->
```cpp
const tut::Tracked        item{5};  // const: a habit, and usually a good one
std::vector<tut::Tracked> kept;
kept.reserve(1);
tut::Tracked::ResetCounts();

kept.push_back(std::move(item));  // "I am done with it, take it"

std::cout << "copies: " << tut::Tracked::Counts().copies
          << ", moves: " << tut::Tracked::Counts().moves << '\n';
```

`cmake --build --preset clang-debug -t x04_03 && build/clang-debug/bin/x04_03` prints:

<!-- file: chapters/04-move-semantics/experiments/x03_move_from_const.expect -->
```
copies: 1, moves: 0
```

Neither compiler says a word. clang-tidy does:

<!-- file: chapters/04-move-semantics/experiments/x03_move_from_const.tidy.expect -->
```
std::move of the const variable 'item' has no effect
performance-move-const-arg
```

**Forget that a named parameter is an lvalue.** `experiments/x05_named_rvalue_reference.cpp`:

<!-- file: chapters/04-move-semantics/experiments/x05_named_rvalue_reference.cpp#function -->
```cpp
void keep(std::vector<std::string>& store, std::string&& text)
{
    store.push_back(text);  // `text` is declared as an rvalue reference...
}
```

<!-- file: chapters/04-move-semantics/experiments/x05_named_rvalue_reference.cpp#main -->
```cpp
std::vector<std::string> store;
std::string              message(40, 'm');
const char* const        characters = message.data();

keep(store, std::move(message));  // the caller gave the string up

std::cout << "the stored string is the caller's block of characters: "
          << (store.front().data() == characters) << '\n';
std::cout << "the caller's string still has " << message.size() << " characters\n";
```

<!-- file: chapters/04-move-semantics/experiments/x05_named_rvalue_reference.expect -->
```
the stored string is the caller's block of characters: false
the caller's string still has 40 characters
```

The caller gave the string up and still has it; the store has a copy. Nothing is broken except the cost,
which is why this mistake survives for years. In the logger it would be one more copy of every message.

**Look at objects after moving from them.** `experiments/x02_use_after_move.cpp`:

<!-- file: chapters/04-move-semantics/experiments/x02_use_after_move.cpp#main -->
```cpp
std::string text(40, 'x');
std::string textTaker = std::move(text);
std::cout << "string:      size " << text.size() << '\n';

std::optional<std::string> maybe(std::string(40, 'y'));
std::optional<std::string> maybeTaker = std::move(maybe);
std::cout << "optional:    has a value: " << maybe.has_value() << '\n';

std::unique_ptr<int> pointer = std::make_unique<int>(7);
std::unique_ptr<int> owner   = std::move(pointer);
std::cout << "unique_ptr:  is null: " << (pointer == nullptr) << '\n';

int number      = 7;
int numberTaker = std::move(number);
std::cout << "int:         " << number << '\n';
```

<!-- file: chapters/04-move-semantics/experiments/x02_use_after_move.expect -->
```
string:      size
optional:    has a value: true
unique_ptr:  is null: true
int:         7
```

On the first line this library prints `size 0`: the moved-from string is empty. That is what libstdc++
does, not what the standard promises. The optional says it has a value, and the value is a moved-from
string. The `unique_ptr` is null, by guarantee. The `int` was "moved" by copying it. clang-tidy reports
reads like these; its first report here is

<!-- file: chapters/04-move-semantics/experiments/x02_use_after_move.tidy.expect -->
```
'text' used after it was moved
bugprone-use-after-move
```

**Help the compiler in a `return`.** `experiments/x04_return_std_move.cpp`:

<!-- file: chapters/04-move-semantics/experiments/x04_return_std_move.cpp#function -->
```cpp
std::vector<int> numbers()
{
    std::vector<int> result{1, 2, 3};
#ifdef TUT_BREAK
    return std::move(result);
#else
    return result;
#endif
}
```

`cmake --build --preset clang-debug -t x04_04` fails, with the same sentence from both compilers:

<!-- file: chapters/04-move-semantics/experiments/x04_return_std_move.expect -->
```
moving a local object in a return statement prevents copy elision
pessimizing-move
```

**A move constructor that might throw.** `experiments/x01_move_without_noexcept.cpp` has two element types
that differ in one word:

<!-- file: chapters/04-move-semantics/experiments/x01_move_without_noexcept.cpp#types -->
```cpp
struct Careful
{
    Careful() = default;
    Careful(const Careful& /*unused*/) { ++g_copies; }
    Careful(Careful&& /*unused*/) { }  // a move constructor, but it might throw
};

struct Confident
{
    Confident() = default;
    Confident(const Confident& /*unused*/) { ++g_copies; }
    Confident(Confident&& /*unused*/) noexcept { }  // the same, with a promise
};
```

Sixteen of each are appended to a vector, which reallocates several times on the way:

<!-- file: chapters/04-move-semantics/experiments/x01_move_without_noexcept.expect -->
```
move may throw:  the vector copied its elements while growing? yes
move is noexcept: the vector copied its elements while growing? no (0 copies)
```

With this library the first count is 15: every reallocation copied every element, although a move
constructor was sitting right there.

## Back in logger

The answers are in `solutions/04-move-semantics/README.md`.

1. `Core::Log` takes its message as `std::string&&` (`src/Core.h:54`). `Core::AddSink` takes its sink as
   `std::shared_ptr<Sink>`, by value (`src/Core.h:48`). Both end with a `std::move` into storage. Why does
   the logger use two different forms? What can a caller of `add_sink` do that a caller of `Log` cannot,
   and why is that wanted in one place and not in the other?
2. `std::thread toJoin = std::move(worker);` (`src/Core.cpp:602`). What state is the member `worker` in
   after that line? Which later line of the logger relies on exactly that state? (Look at
   `StartWorkerLocked`, `src/Core.cpp:341-346`, and at what assigning to a `std::thread` that still owns a
   thread does.)
3. `ClearSinks` empties the list of sinks with `removed.swap(sinks)` (`src/Core.cpp:197-199`), where
   `removed` is a local declared *before* the lock. Why a swap into a local, and not `sinks.clear()`? Why
   is the local declared before the lock and not after it?

## Take it to your rebuild

In your copy of the logger, change the last parameter of `Core::Log` and of `detail::log_message` from
`std::string&& message` to `const std::string& message`, in the two headers and the two source files, and
change nothing else. Predict first: what stops compiling? Then build and run the tests. Finally, find the
line where a message's text is now copied, and say why the `std::move` on that line no longer moves. Put the
signatures back.

## Checkpoint

You should be able to explain:

- what `std::move(x)` does, and what has to exist for anything to be moved as a result;
- why a function with a `std::string&& text` parameter has to write `std::move(text)` to move it on;
- what you may do with an object after moving from it, and three ways to empty a container that leave
  nothing unspecified;
- why adding a destructor to a class can make it slower, and what the rule of five has to do with it;
- why a move constructor without `noexcept` is a copy inside a `std::vector`.

Next: variadic templates and perfect forwarding. `add_sink<S>(Args&&... args)` takes any arguments and
passes them on to a constructor it has never seen, each as the lvalue or rvalue it was. That needs
`std::forward`, which is `std::move`'s conditional cousin.

---

Previous: [Chapter 3: Ranges I](../03-ranges-algorithms/README.md) | [Index](../../README.md) | Next: [Chapter 5: Variadic templates and perfect forwarding](../05-forwarding/README.md)
