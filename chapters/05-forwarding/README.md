# Chapter 5: Variadic templates and perfect forwarding

`logger::info("x = {} and y = {}", x, y)` takes any number of arguments of any types.
`logger::add_sink<logger::FileSink>(logFile)` constructs an object of a class the logger has never heard of,
from arguments it does not look at. Both are built from the same two tools: a *parameter pack*, which stands
for "some arguments, however many", and `std::forward`, which passes each of them on exactly as it arrived.
The guide uses both from step 4 on and explains neither.

This is the chapter the last one prepared: `std::forward` is `std::move` with a condition, and the condition
is the value category you learnt to read there.

## Goal

- You can write a function template that takes any number of arguments, and count them, expand them and
  fold them.
- You know when `T&&` is a forwarding reference and when it is an rvalue reference, and what `T` is deduced
  as.
- You write `std::forward<Args>(args)...` in the right place, with the dots in the right place, and you
  know the cases where forwarding is the wrong thing to do.
- You can say what a template is for with a `requires` clause.

## Where you met it

Step 5 of the guide. This function is the whole pattern in five lines:

<!-- logger: include/logger/logger.h:51-59 -->
```cpp
/// Constructs an S in place, adds it and returns it:
///     auto file = logger::add_sink<logger::FileSink>("app.log");
template <typename S, typename... Args>
std::shared_ptr<S> add_sink(Args&&... args)
{
    std::shared_ptr<S> sink = std::make_shared<S>(std::forward<Args>(args)...);
    add_sink(std::shared_ptr<Sink>(sink));
    return sink;
}
```

and every logging function has the same shape:

<!-- logger: include/logger/logger.h:164-168 -->
```cpp
template <typename... Args>
void info(FormatString<std::type_identity_t<Args>...> fmt, Args&&... args) noexcept
{
    detail::log_at<Level::Info, Args...>(fmt, std::forward<Args>(args)...);
}
```

By the end of this chapter you can read every token of both except one, the
`FormatString<std::type_identity_t<Args>...>`, which gets its first explanation here and its full one in
chapter 9.

## The idea

### A pack is zero or more of something

<!-- file: chapters/05-forwarding/examples/e01_forwarding.cpp#pack -->
```cpp
// `typename... Ts` is a template parameter pack: zero or more types. `const Ts&... values` is a
// function parameter pack: one parameter per type.
template <typename... Ts>
std::size_t how_many(const Ts&... /*values*/)
{
    return sizeof...(Ts);  // the number of elements: the only thing you can ask a pack directly
}
```

`how_many()` is 0 and `how_many(1, "two", 3.0)` is 3. A pack is not a container. It cannot be indexed,
and there is no loop over it, because its elements may all have different types and the whole thing is
resolved at compile time. There are exactly three things you can do with one.

**Count it**, with `sizeof...`, as above.

**Expand it.** A pattern containing the pack's name, followed by `...`, is written out once per element,
with commas between:

<!-- file: chapters/05-forwarding/examples/e01_forwarding.cpp#expansion -->
```cpp
// A pack is used by expanding it: a pattern followed by `...` is repeated once for each element,
// separated by commas.
template <typename... Ts>
std::size_t total_length(const Ts&... texts)
{
    return total(length_of(texts)...);  // total(length_of(t1), length_of(t2), length_of(t3))
}
```

The pattern can be as large as you like; everything to the left of the dots, back to the start of the
expression that contains the pack's name, is repeated. Where the dots go therefore decides what is repeated,
and that is the most common mistake with packs.

**Fold it** (C++17), which puts an operator between the elements where an expansion puts commas:

<!-- file: chapters/05-forwarding/examples/e01_forwarding.cpp#fold -->
```cpp
// A fold expression puts an operator between the elements instead of a comma.
template <typename... Ts>
bool all_positive(const Ts&... values)
{
    return ((values > 0) && ...);  // (v1 > 0) && (v2 > 0) && ...     true for an empty pack
}

template <typename... Ts>
void print_all(const Ts&... values)
{
    ((std::cout << values << ' '), ...);  // the comma operator: do this for each, in order
    std::cout << '\n';
}
```

A fold must be in parentheses. `(pack op ...)` and `(... op pack)` differ in which side is grouped first;
`(init op ... op pack)` adds a starting value, which also defines the result for an empty pack. Without one,
an empty pack is allowed only for `&&` (true), `||` (false) and the comma. The logger has no fold; the
second form, with the comma, is how you say "do this for each argument", and it turns up in every codebase
that has packs.

### `T&&` with a deduced `T` binds to anything

Chapter 4: `std::string&&` binds to rvalues only. But when the type in front of `&&` is a template
parameter that is being deduced *for this very function*, a special rule applies and the parameter accepts
lvalues as well. It is called a **forwarding reference**. What is deduced records which kind arrived:

| The argument is | `T` is deduced as | so the parameter `T&&` is |
| --- | --- | --- |
| an lvalue of type `X` | `X&` | `X& &&`, which collapses to `X&` |
| an rvalue of type `X` | `X` | `X&&` |

A reference to a reference cannot be written by hand, but it can arise through a template parameter, and
then it *collapses*: if either is an lvalue reference, the result is an lvalue reference. The point of the
whole arrangement is the first column of the second row: **`T` remembers the value category**. It is a
reference type exactly when the caller passed an lvalue.

### `std::forward` gives the category back

Inside the function the parameter has a name, so it is an lvalue (chapter 4). Passing it on by name
therefore always passes an lvalue, even when the caller gave an rvalue:

<!-- file: chapters/05-forwarding/examples/e01_forwarding.cpp#receive -->
```cpp
std::string_view receive(const std::string& /*text*/)
{
    return "as an lvalue";
}

std::string_view receive(
    std::string&& /*text*/)  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
{
    return "as an rvalue";
}
```

<!-- file: chapters/05-forwarding/examples/e01_forwarding.cpp#relay -->
```cpp
// T&& with T deduced right here is a forwarding reference: it accepts lvalues and rvalues alike.
template <typename T>
std::string_view relay_plainly(T&& text)  // NOLINT(cppcoreguidelines-missing-std-forward)
{
    return receive(text);  // `text` is a name, so receive() always sees an lvalue
}

template <typename T>
std::string_view relay(T&& text)
{
    return receive(
        std::forward<T>(text));  // an lvalue stays an lvalue, an rvalue becomes one again
}
```

<!-- file: chapters/05-forwarding/examples/e01_forwarding.cpp#relay-calls -->
```cpp
const std::string name = "Ada";

std::cout << relay_plainly(name) << ", " << relay_plainly(std::string("Grace")) << '\n';
std::cout << relay(name) << ", " << relay(std::string("Grace")) << '\n';
```

This prints `as an lvalue, as an lvalue` and then `as an lvalue, as an rvalue`.

`std::forward<T>(text)` is a cast, like `std::move`. It casts to an rvalue **if `T` is not a reference
type**, that is, if the caller passed an rvalue, and leaves an lvalue alone otherwise. `std::move` always
casts; `std::forward` casts back to whatever the argument was. The `<T>` has to be written out: it is where
the remembered category comes from.

### The pattern

Put a pack and forwarding together and you have the shape of `make_unique`, `make_shared`, `emplace_back`
and `add_sink`:

<!-- file: chapters/05-forwarding/examples/e01_forwarding.cpp#make -->
```cpp
// The pattern behind make_unique, make_shared, emplace_back and the logger's add_sink: accept any
// arguments, and hand each one on to a constructor exactly as it came.
template <typename T, typename... Args>
std::unique_ptr<T> make_boxed(Args&&... args)
{
    return std::unique_ptr<T>(
        new T(std::forward<Args>(args)...));  // NOLINT(cppcoreguidelines-owning-memory)
}
```

`Args&&... args` is a pack of forwarding references, one per argument, each deduced on its own.
`std::forward<Args>(args)...` is an expansion whose pattern is `std::forward<Args>(args)`, so it becomes
`std::forward<A1>(a1), std::forward<A2>(a2)` and so on. Every argument reaches the constructor of `T` as
what it was: lvalues are copied by that constructor if it copies, rvalues are moved by it if it moves, and
this function decides nothing. That is what "perfect forwarding" means.

Now `add_sink` again. `S` comes first and is written by the caller, `add_sink<FileSink>`; the pack `Args`
comes after it and is deduced from the arguments. The arguments are forwarded to `std::make_shared<S>`,
which forwards them to `S`'s constructor. The result is converted to a `std::shared_ptr<Sink>` for the
logger and returned as a `std::shared_ptr<S>` to the caller, who can therefore call the sink's own member
functions.

### Packs in class templates

<!-- file: chapters/05-forwarding/examples/e01_forwarding.cpp#class-pack -->
```cpp
// A class template can have a pack too. This is all a typed table row needs.
template <typename... Columns>
struct Row
{
    std::tuple<Columns...> cells;
};
```

`std::tuple`, `std::variant` and `std::function<R(Args...)>` are class templates with a pack. So is the
logger's `FormatString<Args...>`, whose only purpose is to carry the argument types to
`std::format_string<Args...>`.

### Saying what a template is for: `requires`

A template accepts every type, and reports a type that does not fit only when some line in its body fails
to compile, deep inside. A constraint moves that check to the declaration:

<!-- file: chapters/05-forwarding/examples/e01_forwarding.cpp#requires -->
```cpp
// A requires clause states what a template is for. std::integral is a concept from <concepts>: a
// compile-time yes-or-no question about a type.
template <typename T>
    requires std::integral<T>
T half(T value)
{
    return value / 2;
}

// The same, shorter: the concept in place of `typename`.
template <std::floating_point T>
T half(T value)
{
    return value / T{2};
}
```

A **concept** is a named compile-time question about types, answered with yes or no. `<concepts>` has the
standard ones: `std::integral`, `std::floating_point`, `std::same_as<T>`, `std::convertible_to<T>`,
`std::constructible_from<Args...>`, `std::derived_from<Base>`, `std::invocable<Args...>`. A template whose
constraint is not satisfied is not an error; it is simply not a candidate, and overload resolution goes on
without it. That is how the two `half` functions above coexist, and it is what drill 8 needs.

The logger has one constraint, on the constructor of `FormatString`:
`requires std::convertible_to<const S&, std::string_view>`. It says "this constructor is for things that
can be viewed as text", and keeps it out of the way for everything else.

### A first look at `std::type_identity_t`

<!-- file: chapters/05-forwarding/examples/e01_forwarding.cpp#type-identity -->
```cpp
// Deduction looks at every parameter that mentions T, and they have to agree: scale(2.5, 2) would
// deduce double from the first argument and int from the second, and fail. type_identity_t<T> is T,
// spelt so that the compiler does not try to deduce from it.
template <typename T>
T scale(T value, std::type_identity_t<T> factor)
{
    return value * factor;
}
```

When a template parameter appears in several function parameters, each one takes part in deduction and
they must agree. Wrapping one of them in `std::type_identity_t<...>` (C++20) removes it from deduction: `T`
is then deduced from the others alone, and this parameter simply has that type, with the usual
conversions. `scale(2.5, 2)` deduces `double` from the first argument and converts the `2`. The logger's
functions do the same to their first parameter, so that `Args` is deduced from the arguments and the
string literal is then converted. Why that matters is chapter 9.

## The details that bite

### `T&&` is a forwarding reference only when `T` is deduced right there

`template <typename T> void f(T&& x)`: yes. `void f(auto&& x)`: yes, it is the same thing in short form.
But `const T&&`, `std::vector<T>&&`, and `T&&` in a member function of a class template whose `T` belongs
to the class: all plain rvalue references. In the last case `T` was fixed when somebody wrote `Box<int>`,
and nothing is deduced at the call. That is why `std::vector<T>::push_back(T&&)` is an rvalue-reference
overload, and `emplace_back`, which forwards, is a template of its own.

### `std::move` and `std::forward` are not interchangeable

`std::move` on a forwarding reference turns every argument into an rvalue, including the lvalues the caller
meant to keep, and the caller finds them emptied. Plain names without either pass everything as an lvalue,
which copies what could have been moved and does not compile at all for types that cannot be copied. The
rule is by the type of the parameter: **`std::move` for a parameter declared `X&&`, `std::forward<T>` for a
parameter declared `T&&` with a deduced `T`.**

### The dots go after the pattern

`std::forward<Args>(args)...` forwards each argument separately. `std::forward<Args...>(args...)` expands
both packs *inside* one call to `std::forward`, which takes one type and one argument: it compiles when the
pack has exactly one element and fails for any other number. The third experiment shows the message.

### Forward once, and not in a loop

A forwarded rvalue may be moved from by whoever receives it. After `std::forward<Args>(args)...` the
arguments must be considered gone, exactly as after `std::move`. A function that uses its arguments twice,
or in a loop, must not forward them the first time. Design problem 1 is about exactly this.

### A forwarding constructor swallows the copy constructor

`template <typename U> explicit Label(U&& value)` accepts anything, and for a non-const `Label` lvalue it
is an exact match with `U = Label&`, which beats the copy constructor's `const Label&`. So `Label b(a);`
selects the template and tries to build the text from a `Label`. Every class with a constructor taking one
unconstrained forwarding reference has this problem. The cure is a `requires` clause that says what the
constructor is for.

### Not everything can be forwarded

Forwarding deduces a type for each argument and passes a value of that type on. A few things lose their
meaning when a type is deduced for them:

- **A braced list** `{1, 2, 3}` has no type of its own, so nothing can be deduced. Name the type.
- **The literal `0`** used as a null pointer is deduced as `int`, and an `int` variable does not convert to
  a pointer. Write `nullptr`, which has a type of its own.
- **Any literal** travels as the type it was deduced as. `make_boxed<std::vector<int>>(3, 7)` passes the
  `3` as an `int`, and with this tutorial's warnings the conversion to the vector's unsigned size type
  inside `make_boxed` is reported, which a direct `std::vector<int>(3, 7)` is not.

### To forward a result, the return type must not be `auto`

A wrapper that returns what the wrapped function returns has to keep references as references. `auto`
never deduces a reference: it would turn an `int&` into a copy. `decltype(auto)` (C++14) deduces exactly
the type of the returned expression, references included, and also works when that type is `void`.

## When to reach for it, and when not

- **Use a pack** when a function really takes "any number of things", or passes arguments through to
  something it does not know. For "several values of one type", a `std::span` or an
  `std::initializer_list` is simpler and is not a template.
- **Use `Args&&...` with `std::forward`** when the arguments are passed on exactly once. Constructing
  something in place is the model case.
- **Use `const Args&...`** when the function only reads the arguments (prints them, compares them, sums
  them). No forwarding is involved and none is needed.
- **Constrain** any template that could be called with types it is not meant for, above all constructors
  that take a single forwarding reference.
- **Write an ordinary function** when you know the parameter types. Forwarding gives up implicit
  conversions at the call and gives error messages from inside the callee.

## Drills

Run each with `ctest --preset clang-debug -R ch05_dNN`.

| # | Target | File | Do |
| --- | --- | --- | --- |
| 1 | `ch05_d01` | `exercises/d01_count_and_sum.h` | `count_args` and `sum_all`: count a pack, fold a pack |
| 2 | `ch05_d02` | `exercises/d02_join_with.h` | `join_with`: write any arguments, with a separator between |
| 3 | `ch05_d03` | `exercises/d03_deduce.h` | predict what `T` is, and which `&&` forwards |
| 4 | `ch05_d04` | `exercises/d04_is_one_of.h` | `is_one_of`: is a value equal to any of the others? |
| 5 | `ch05_d05` | `exercises/d05_call_logged.h` | `call_logged`: call any function with any arguments, unchanged |
| 6 | `ch05_d06` | `exercises/d06_fixed_stack.h` | `FixedStack::Emplace`: construct an element in place |
| 7 | `ch05_d07` | `exercises/d07_make_drawn.h` | `make_drawn`: the logger's `add_sink`, for shapes |
| 8 | `ch05_d08` | `exercises/d08_label.h` | repair a constructor that broke copying |
| 9 | `ch05_d09` | `exercises/d09_tee.h` | `Tee`: a class with a pack of sinks (longer; optional) |

**Drill 1: `count_args` and `sum_all`.** One line each. The sum of no arguments is 0, which tells you which
form of fold to use. A second test, `ch05_d01.c1`, asks the same at compile time.

**Drill 2: `join_with`.** Write each item to a `std::ostringstream` with `<<`, and the separator between
them. A fold over the comma operator does "for each item"; the separator is the puzzle. Two approaches
work: remember whether you are at the first item, or take the first item as a parameter of its own and fold
over the rest.

**Drill 3: deduction.** No code. Part 1: for five calls of `f(T&& x)`, write down what `T` is. Part 2: for
six declarations, say whether `x` is a forwarding reference. Fill in all eleven before you run the tests.

**Drill 4: `is_one_of`.** `is_one_of(state, State::Stopped, State::Exiting)` reads better than two
comparisons joined by `||`, and is one fold. The logger writes such conditions out by hand (see
`src/Core.cpp:568`).

**Drill 5: `call_logged`.** The complete forwarding wrapper: callable and arguments forwarded, result
returned unchanged. The stub declares the function as returning `void`, which is not right, so this drill
does not compile until you have worked out the return type; that is part of the exercise. `std::invoke`
calls anything callable. Six tests check each property separately.

**Drill 6: `FixedStack::Emplace`.** The elements live in a `std::array` of `std::optional<T>`, and an
optional has an `emplace` of its own that takes constructor arguments. Forward to it. The second test
counts: constructing in place means zero copies *and* zero moves.

**Drill 7: `make_drawn`.** `add_sink` with the names changed. `exercises/shapes.h` is given. Write it
without looking at the logger; then compare.

**Drill 8: `Label`.** Working code with a defect: a `Label` that is not `const` can no longer be copied.
`ch05_d08.c1` is the test that fails to compile. Read the beginning of its error message to see which
constructor the compiler chose for `Label copy(original);`, then add the one line that keeps the template
out of it. The other test tells you whether your constraint also rejects what it should.

**Drill 9: `Tee`** (optional). A class template with a pack of sink types. It holds a reference to each
sink and writes every line to all of them, in order. `std::tuple<Sinks&...>` holds the references, and
`std::apply` calls a function with a tuple's elements as separate arguments, which gives you a pack to
fold over.

## Design problems

**Problem 1: try again** (`ch05_p01`, you create `exercises/p01_retry.h`).
Operations on a network fail and are tried again. Write `ch05::retry(attempts, operation, arguments...)`:
it calls `operation(arguments...)` until that returns `true`, at most `attempts` times, and returns whether
it succeeded. The arguments may be of any number and type. Some are expensive to copy, and `retry` itself
must add no copies. An operation may take an argument by reference and change it, and the caller must see
the change. Every attempt, not only the first, must be given the arguments in the state the previous
attempt left them in.

**Problem 2: one event type for everything** (`ch05_p02`, you create `exercises/p02_event.h`).
A user interface has events. A button's `clicked` carries two numbers, the position. A text field's
`changed` carries the new text. A window's `closed` carries nothing. Write one class template
`ch05::Event<...>` that serves all of them: `Subscribe(listener)` registers something to be called, and
`Emit(values...)` calls every registered listener, in the order they subscribed, with the event's values.
An event may have any number of listeners, and every one of them must receive the values as the emitter
passed them.

**Problem 3: time anything** (`ch05_p03`, you create `exercises/p03_timed.h`).
Write `ch05::timed(timings, label, function, arguments...)`. It calls `function(arguments...)` and returns
whatever that returns: a value, a reference as a reference, or nothing if the function returns nothing.
Before the call it appends `"<label>: begin"` to `timings`, a `std::vector<std::string>`, and after the
call `"<label>: end"`, also when the function throws. (Real timestamps come in chapter 10.)

## Break it on purpose

**`std::move` where `std::forward` belongs.** `experiments/x01_move_instead_of_forward.cpp`:

<!-- file: chapters/05-forwarding/experiments/x01_move_instead_of_forward.cpp#function -->
```cpp
template <typename F, typename... Args>
decltype(auto) call(F&& function, Args&&... args)
{
    return std::invoke(std::forward<F>(function),
                       std::move(args)...);  // every argument, unconditionally
}
```

<!-- file: chapters/05-forwarding/experiments/x01_move_instead_of_forward.cpp#main -->
```cpp
tut::Tracked kept{7};  // the caller passes it by name and intends to go on using it

const int value = call([](tut::Tracked item) { return item.Value(); }, kept);

std::cout << "the function saw " << value << '\n';
std::cout << "the caller's object was moved from: " << kept.IsMovedFrom() << '\n';
```

`cmake --build --preset clang-debug -t x05_01 && build/clang-debug/bin/x05_01` prints:

<!-- file: chapters/05-forwarding/experiments/x01_move_instead_of_forward.expect -->
```
the function saw 7
the caller's object was moved from: true
```

The caller passed a name and never wrote `std::move`. The wrapper took the object anyway.

**Neither `std::move` nor `std::forward`.** `experiments/x02_no_forward.cpp` passes the arguments on by
name:

<!-- file: chapters/05-forwarding/experiments/x02_no_forward.cpp#function -->
```cpp
template <typename F, typename... Args>
decltype(auto) call(F&& function, Args&&... args)
{
    return std::forward<F>(function)(args...);  // each argument as an lvalue
}
```

<!-- file: chapters/05-forwarding/experiments/x02_no_forward.cpp#main -->
```cpp
    std::cout << call(twice, 21) << '\n';  // fine: an int can be copied

#ifdef TUT_BREAK
    std::cout << call(read, std::make_unique<int>(9)) << '\n';  // a unique_ptr cannot
#endif
```

With an `int` it works and copies. `cmake --build --preset clang-debug -t x05_02` shows what happens with
an argument that cannot be copied; Clang's message:

<!-- file: chapters/05-forwarding/experiments/x02_no_forward.clang.expect -->
```
call to deleted constructor of 'std::unique_ptr<int>'
```

GCC says `use of deleted function` and names the same copy constructor.

**The dots in the wrong place.** `experiments/x03_forward_whole_pack.cpp`:

<!-- file: chapters/05-forwarding/experiments/x03_forward_whole_pack.cpp#function -->
```cpp
template <typename... Args>
int relay(Args&&... args)
{
    return sum(std::forward<Args...>(args...));  // not std::forward<Args>(args)...
}
```

<!-- file: chapters/05-forwarding/experiments/x03_forward_whole_pack.cpp#main -->
```cpp
    std::cout << sum(1, 2) << ' ' << relay(1)
              << '\n';  // relay(1) compiles: one argument, one spelling

#ifdef TUT_BREAK
    std::cout << relay(1, 2) << '\n';
#endif
```

`relay(1)` compiles, which is why this mistake survives a first test. With two arguments
(`cmake --build --preset clang-debug -t x05_03`) both compilers report:

<!-- file: chapters/05-forwarding/experiments/x03_forward_whole_pack.expect -->
```
no matching function for call to
forward
```

GCC shows what the expansion produced: `forward<int, int>(int&, int&)`.

**Forward a braced list.** `experiments/x04_braced_argument.cpp`:

<!-- file: chapters/05-forwarding/experiments/x04_braced_argument.cpp#main -->
```cpp
    const std::vector<int> direct({1, 2, 3});  // fine: the constructor says what the braces are

    const auto named =
        std::make_unique<std::vector<int>>(std::initializer_list<int>{1, 2, 3});  // fine

#ifdef TUT_BREAK
    const auto forwarded =
        std::make_unique<std::vector<int>>({1, 2, 3});  // what type is {1, 2, 3}?
    std::cout << forwarded->size() << '\n';
#endif
```

<!-- file: chapters/05-forwarding/experiments/x04_braced_argument.expect -->
```
no matching function for call to
make_unique
```

`std::make_unique` cannot deduce a type for `{1, 2, 3}`. The line above it shows the cure.

**Forward a `0` to a pointer.** `experiments/x05_zero_as_pointer.cpp`:

<!-- file: chapters/05-forwarding/experiments/x05_zero_as_pointer.cpp#type -->
```cpp
struct Node
{
    explicit Node(Node* parentNode) : parent(parentNode) { }

    Node* parent;
};
```

<!-- file: chapters/05-forwarding/experiments/x05_zero_as_pointer.cpp#main -->
```cpp
    const Node root(
        0);  // compiles: the literal 0 is a null pointer constant (write nullptr anyway)

#ifdef TUT_BREAK
    const auto child = std::make_unique<Node>(0);  // the 0 is deduced as an int, and travels as one
    std::cout << (child->parent == nullptr) << '\n';
#endif
```

`cmake --build --preset clang-debug -t x05_05` fails, and look at *where*:

<!-- file: chapters/05-forwarding/experiments/x05_zero_as_pointer.clang.expect -->
```
unique_ptr.h
no matching constructor for initialization of
Node
```

The error is reported inside the standard library's `unique_ptr.h`, at the line where `make_unique` calls
`new Node(...)` with the forwarded `int`. Your own line appears further down, in the notes, after "in
instantiation of". This is what forwarding costs: a mistake at the call is diagnosed in the callee. Reading
such a message from the bottom up, to the first line that is in your own file, is a skill this chapter's
exercises will have given you some practice in.

## Back in logger

The answers are in `solutions/05-forwarding/README.md`.

1. In `add_sink<S, Args...>` (`include/logger/logger.h:53-59`): which template argument does the caller
   write, and which are deduced? What would stop working if the parameter were `Args... args`? And if it
   were `const Args&... args`? Why does the function return `std::shared_ptr<S>` and not
   `std::shared_ptr<Sink>`? (`src/main.cpp:99` uses the difference.)
2. `detail::log_at` is declared `template <Level L, typename... Args>` and called as
   `detail::log_at<Level::Info, Args...>(fmt, std::forward<Args>(args)...)`
   (`include/logger/logger.h:117-128` and `:167`). Of `L` and `Args`, which could have been deduced from
   the function arguments, and which could not? Its parameter is spelt `Args&&... args`: with `Args` given
   explicitly, is that still a forwarding reference? What makes the forwarding work anyway?
3. For the call `logger::info("{}", name)`, where `name` is a `std::string` variable: what is `Args`? Is
   `name` copied anywhere between the call and the moment `std::format` reads it?

## Take it to your rebuild

`add_sink<S>` accepts any type `S`. Call `logger::add_sink<int>(42)` in your copy and read the error: it
points into `logger.h`, at the line inside `add_sink` that converts the new pointer to a
`std::shared_ptr<Sink>`, and says nothing about your call being the mistake. Constrain the template so that
the error is reported at the call and names the requirement: `S` must be derived from `Sink`. `<concepts>`
has the concept for it. For the full treatment, also require that an `S` can be constructed from
`Args...`.

## Checkpoint

You should be able to explain:

- the three things that can be done with a parameter pack, and what `std::forward<Args>(args)...` expands
  to for two arguments;
- what `T` is deduced as when a function `f(T&& x)` is called with an lvalue, and what `x`'s type is then;
- what `std::forward<T>(x)` does that `std::move(x)` does not, and where the `<T>` gets its information;
- why a function that calls its callback three times must not forward its arguments to it;
- why a constructor `template <typename U> Label(U&&)` is chosen over the copy constructor, and what a
  `requires` clause changes about that.

Next comes the first review set: problems that do not tell you which chapter they belong to.

---

Previous: [Chapter 4: Move semantics](../04-move-semantics/README.md) | [Index](../../README.md) | Next: [Review A](../ra-review-a/README.md)
