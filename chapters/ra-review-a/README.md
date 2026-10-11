# Review A

Chapters 1 to 5 each told you which tool the exercises were about. This set does not. Every problem here
can be solved with what those chapters covered, most need two or three of them together, and none says
which. Choosing is the exercise: it is the part of fluency that drills cannot train, because a drill has
already chosen for you.

About three hours. There is no new material.

## How to use it

- **Read the statement, close the chapters, and design first.** Decide the types of the parameters and the
  results before you write a body. Then write it. Look something up only when you know what you are looking
  for.
- **The tests fix as little as they can.** Each test file starts with a comment that says exactly what it
  assumes: names, and how results are used. Everything else is yours.
- **Then read the reference.** `solutions/ra-review-a/README.md` says what each reference solution reached
  for and why, and what else would have been a good answer. A solution of yours that passes and differs is
  not wrong; the comparison is the point.
- **Run finished work under `--preset asan` too.** Two of the problems are about lifetimes.

The targets are `chra_p01` to `chra_p05` for the problems and `chra_d01` to `chra_d05` for the fix-it set:
`ctest --preset clang-debug -R chra_p01`.

## Problems

**Problem 1: settings** (`chra_p01`, you create `exercises/p01_settings.h`).
An application reads its settings from a text file: lines of the form `key = value`, with any blanks around
key and value. Empty lines and lines that start with `#` are ignored. Write a class `review_a::Settings`
that is built from the text of such a file and has three getters, `Text(key)`, `Number(key)` and
`Flag(key)`, for settings that are a text, a whole number, and `true`/`false`/`on`/`off`.

The application treats three situations differently, for every setting. The setting is not in the file:
use the built-in default, silently. It is there and well formed: use it. It is there and cannot be read as
what was asked for (`port = soon`): tell the user about their mistake. A getter must let its caller tell
all three apart, and `review_a::is_missing(result)` must say, for a result without a value, whether the
key was absent.

The file's text is read into a buffer that is reused for other things as soon as the settings have been
built.

**Problem 2: two event streams** (`chra_p02`, you create `exercises/p02_merge_events.h`).
Two services each deliver their events as a list sorted by id. An event has an `id` and a `payload`, a text
that can be large. Some events are delivered by both services, with the same id and the same payload. Write
`review_a::merge_events(first, second)`, which returns all events in one list, sorted by id, each id once.
The caller has no use for the two lists afterwards and says so at the call. No payload may be copied.

**Problem 3: widgets by name** (`chra_p03`, you create `exercises/p03_factory.h`).
A user-interface description names its widgets: `ok` is a button with the caption "OK", `volume` is a
slider from 0 to 11. `exercises/widgets.h` is given and has the widget classes. Write a class
`review_a::WidgetRegistry`. `Register<T>(name, arguments...)` records that `name` means "a `T` constructed
from these arguments"; the arguments are whatever `T`'s constructor takes. `Create(name)` then makes a new
widget of that kind, every time it is called, and yields nothing for a name that was never registered.

**Problem 4: the highest measurements** (`chra_p04`, you create `exercises/p04_largest.h`).
A monitoring tool holds a list of measurements. Each has the name of a `station`, a `value`, and the raw
`samples` it was computed from, tens of thousands of numbers. Write `review_a::largest(measurements,
count)`, which gives the caller the `count` measurements with the highest values, highest first. The
caller's list must stay in the order it is in, and no measurement may be copied.

**Problem 5: a command line for a game** (`chra_p05`, you create `exercises/p05_dispatcher.h`).
A text adventure reads lines such as `take lamp` and `GO north by northwest`. The first word is a verb, the
rest are its arguments, and there may be any number of blanks between words. A verb may be abbreviated to
any beginning that fits only one verb, in any letter case; a verb typed in full always means itself. Write
a class `review_a::Dispatcher`. `On(verb, handler)` registers what to do for a verb. `Run(line)` takes the
line apart, finds the verb and calls its handler with the arguments, and returns what the handler returns.
A handler only reads the arguments. It answers with a text for the player, or with nothing when it has
nothing to say. `Run` yields nothing, too, when the line is empty or the verb is unknown or ambiguous.

## Fix-it set

Five short pieces of code that compile, look reasonable and are wrong. Each is in `exercises/`, with a test
that shows the defect. Find it, repair it, and say in one sentence what the mistake was; each is one of the
mistakes chapters 1 to 5 warned about.

| # | Target | File | It is supposed to |
| --- | --- | --- | --- |
| 1 | `chra_d01` | `exercises/d01_greeter.h` | remember a name and greet it |
| 2 | `chra_d02` | `exercises/d02_journal.h` | store an entry and report how long it was |
| 3 | `chra_d03` | `exercises/d03_broadcast.h` | call every listener with the same arguments |
| 4 | `chra_d04` | `exercises/d04_use_colour.h` | honour the user's colour setting, if there is one |
| 5 | `chra_d05` | `exercises/d05_drop_negative.h` | remove the negative numbers from a vector |

`ctest --preset tidy -R chra_d02`, after `cmake --preset tidy`, shows what clang-tidy makes of each: it
finds some of the five by itself, and not others.

## Clinic: choose the signature

No code and no test. For each function described below, write its declaration: the parameter types and
the return type, and for 9 the template header. Then compare with
`solutions/ra-review-a/p06_signatures.h` and the reasons in `solutions/ra-review-a/README.md`.

1. A `Document`'s `SetTitle`: keeps the new title in the object.
2. Tells how many words of a text are longer than a given limit.
3. Looks up the population of a city in a table; some cities are not in it.
4. Finds the customer record with a given number, in a table that the caller owns; there may be none.
5. Takes over a decoded image of tens of megabytes, to display it. A caller must not be able to have one
   copied by accident.
6. Sorts a sequence of readings, in place.
7. Returns the words of a line of text. The caller keeps the line alive while it uses them.
8. Returns the words of a line of text. The caller will keep them long after the line is gone.
9. Creates an object of any class derived from `Shape`, from whatever arguments that class's constructor
   takes, and gives the caller sole ownership of it.
10. Calls a callback once for every open connection.

## When you are through

If a problem took long, note *where* the time went: in finding the design, or in writing it down. The first
means rereading the "When to reach for it" section of a chapter; the second means more drills of that
chapter. Which chapters to come back to is the most useful thing this set can tell you.

The tutorial continues with chapter 6, `constexpr` and `consteval`, which is not written yet;
`ROADMAP.md` in the repository's root lists what is planned.

---

Previous: [Chapter 5: Variadic templates and perfect forwarding](../05-forwarding/README.md) | [Index](../../README.md)
