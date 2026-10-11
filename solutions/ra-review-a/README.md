# Review A: reference solutions

Compiled against the review's tests: `ctest --preset solutions -R solra`. For each problem: what it was
about, what the reference reaches for, and what else would have been a good answer.

## Problem 1: settings

*Chapters 2 and 1: a result with a reason, and a view that must not be kept.*

**Three outcomes need a type with room for three.** "There, and here it is", "not there" and "there, but
unreadable" cannot fit into a `std::optional<int>`, which has two states. The reference returns a
`std::expected` whose error says which of the two failures it was:

<!-- file: solutions/ra-review-a/p01_settings.h#result -->
```cpp
/// Why a getter has no value to give.
enum class Lookup : std::uint8_t
{
    Missing,    ///< the key is not in the settings: use your default
    Malformed,  ///< the key is there and its value cannot be read as the type asked for: tell the
                ///< user
};

template <typename T>
using Setting = std::expected<T, Lookup>;

/// Whether a getter came back empty because the key was not there (as opposed to being unreadable).
template <typename T>
[[nodiscard]] constexpr bool is_missing(const Setting<T>& setting) noexcept
{
    return !setting && setting.error() == Lookup::Missing;
}
```

`std::optional<std::optional<int>>` also has three states and also passes the tests; nobody reading
`if (result && !*result)` will know which is which. A `std::optional<int>` together with a separate
`Has(key)` function passes too, and makes every caller ask twice.

**The settings own their text.** The statement says the buffer is reused, and the last test overwrites it.
A map of `std::string_view` keys and values would be faster to build and would be chapter 1's
`x02_view_of_temporary`, with a delay. The constructor takes a `std::string_view`, because it only reads
the text, and copies what it keeps:

<!-- file: solutions/ra-review-a/p01_settings.h#parse -->
```cpp
/// Reads @p text. Nothing of @p text is referred to afterwards: keys and values are copied.
explicit Settings(std::string_view text)
{
    while (!text.empty())
    {
        const std::size_t      newline = text.find('\n');
        const std::string_view line    = Trimmed(text.substr(0, newline));
        text.remove_prefix(newline == std::string_view::npos ? text.size() : newline + 1);

        const std::size_t equals = line.find('=');
        if (line.empty() || line.front() == '#' || equals == std::string_view::npos)
        {
            continue;
        }
        const std::string_view key = Trimmed(line.substr(0, equals));
        if (!key.empty())
        {
            values.insert_or_assign(std::string(key),
                                    std::string(Trimmed(line.substr(equals + 1))));
        }
    }
}
```

The parsing itself is chapter 1's drills: take a line off the front, trim, find the `=`. Views are the
right tool *inside* the constructor, where the text is certainly alive.

**`Number` is drill 2 of chapter 2**, with one more way to fail:

<!-- file: solutions/ra-review-a/p01_settings.h#number -->
```cpp
[[nodiscard]] Setting<int> Number(std::string_view key) const
{
    const auto found = values.find(key);
    if (found == values.end())
    {
        return std::unexpected(Lookup::Missing);
    }
    const std::string_view text   = found->second;
    int                    value  = 0;
    const char* const      first  = text.data();
    const char* const      last   = first + text.size();  // NOLINT(*-pointer-arithmetic)
    const auto [stoppedAt, error] = std::from_chars(first, last, value);
    if (error != std::errc{} || stoppedAt != last)
    {
        return std::unexpected(Lookup::Malformed);
    }
    return value;
}
```

The map is declared with `std::less<>` as its comparison, which lets `find` take the `std::string_view`
key directly; with the default comparison every lookup would build a `std::string` first.

## Problem 2: two event streams

*Chapters 4 and 3: giving up ownership, and algorithms on sorted ranges.*

<!-- file: solutions/ra-review-a/p02_merge_events.h#merge -->
```cpp
/// All events of @p first and @p second, sorted by id, each id once. Both inputs must be sorted by
/// id. They are taken by value: the caller hands them over with std::move, and no payload is
/// copied.
[[nodiscard]] inline std::vector<Event> merge_events(std::vector<Event> first,
                                                     std::vector<Event> second)
{
    std::vector<Event> merged;
    merged.reserve(first.size() + second.size());

    // A move iterator hands out rvalues, so the algorithm moves each event to its place in `merged`
    // where it would otherwise copy it.
    std::merge(std::make_move_iterator(first.begin()), std::make_move_iterator(first.end()),
               std::make_move_iterator(second.begin()), std::make_move_iterator(second.end()),
               std::back_inserter(merged),
               [](const Event& a, const Event& b) { return a.id < b.id; });

    // Equal ids are neighbours now. unique() keeps the first of each run and returns the leftover
    // part.
    const auto leftover = std::ranges::unique(merged, {}, &Event::id);
    merged.erase(leftover.begin(), leftover.end());
    return merged;
}
```

- **The parameters are by value.** "The caller has no use for the lists afterwards and says so at the
  call" is `merge_events(std::move(a), std::move(b))`, and by-value parameters take the vectors over
  without copying anything. `std::vector<Event>&&` says the same more strictly, and is a good answer.
  `const std::vector<Event>&` cannot work: there is nothing to move a payload out of.
- **Moving through an algorithm.** `std::merge` copies from its inputs. Wrapping the iterators in
  `std::make_move_iterator` makes dereferencing them yield rvalues, so each event is moved instead. A
  hand-written loop with two indices and `std::move(first[i])` is the same thing and just as good.
- **Duplicates.** After the merge, equal ids are neighbours, and `unique` with a projection drops all but
  the first of each run. Then the leftover part is erased: chapter 3's "`remove` does not remove".

The last test compares the addresses of the payloads before and after. A solution that copies even once
on the way fails it.

## Problem 3: widgets by name

*Chapter 5, and one rule from it in particular: forward once.*

<!-- file: solutions/ra-review-a/p03_factory.h#register -->
```cpp
/// Records that @p name stands for "a T constructed from these arguments". The arguments are
/// stored here, as copies or moved in, and every Create(name) constructs a new T from the
/// stored ones.
template <typename T, typename... Args>
    requires std::derived_from<T, Widget> &&
             std::constructible_from<T, const std::decay_t<Args>&...>
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward): they are forwarded, into the capture below
void Register(std::string name, Args&&... args)
{
    // The arguments have to outlive this call, so they go into a tuple that the lambda owns.
    // std::forward: what the caller gave up is moved into the tuple, the rest is copied. Each
    // Create() then passes the stored values as const lvalues: they are needed again next time.
    recipes.insert_or_assign(
        std::move(name),
        [stored = std::make_tuple(std::forward<Args>(args)...)] -> std::unique_ptr<Widget> {
            return std::apply([](const auto&... each) { return std::make_unique<T>(each...); },
                              stored);
        });
}
```

**What has to be stored.** `Register` returns, and `Create` is called later, any number of times. So the
arguments must outlive the call, and they must still be whole after the first `Create`. That rules out
capturing references to them (the caller's variables may be gone, or changed: the fourth test changes one)
and it rules out forwarding them into the widget (the first `Create` would move from them: the third test
creates three).

**How.** The arguments are forwarded *into storage*, once: `std::make_tuple(std::forward<Args>(args)...)`
copies what the caller kept and moves what the caller gave up. From there each `Create` passes them to the
constructor as const lvalues, with `std::apply` turning the tuple back into separate arguments. C++20 can
capture a pack directly, `[...stored = std::forward<Args>(args)]`, and then the body is simply
`return std::make_unique<T>(stored...);`. Both are the same design.

**The recipe's type** is `std::function<std::unique_ptr<Widget>()>`: something that can be called with
nothing and returns a widget. Each `Register<T>` creates a different lambda type, and `std::function`
gives them one type to be stored under.

**The `requires` clause** turns `Register<Button>("ok", 42)` into an error at that line. Without it the
error appears inside the lambda, inside `std::apply`, inside `std::make_unique`.

## Problem 4: the highest measurements

*Chapters 1 and 3: refer, do not copy; and sort what you refer with.*

<!-- file: solutions/ra-review-a/p04_largest.h#largest -->
```cpp
/// The @p count measurements with the highest values, highest first (fewer when there are not that
/// many). The result points into @p measurements: it is valid while that storage is.
[[nodiscard]] inline std::vector<const Measurement*> largest(
    std::span<const Measurement> measurements, std::size_t count)
{
    std::vector<const Measurement*> order;
    order.reserve(measurements.size());
    for (const Measurement& measurement : measurements)
    {
        order.push_back(&measurement);
    }

    // Sort pointers, compare what they point at. partial_sort puts the `count` best in front, in
    // order, and does not bother to sort the rest.
    count             = std::min(count, order.size());
    const auto middle = order.begin() + static_cast<std::ptrdiff_t>(count);
    std::ranges::partial_sort(order, middle, std::ranges::greater{},
                              [](const Measurement* measurement) { return measurement->value; });
    order.resize(count);
    return order;
}
```

"The list must stay as it is and no measurement may be copied" leaves one thing to sort: something small
that refers to the measurements. The reference uses pointers, a `std::vector<const Measurement*>`, and
sorts them by what they point to, through a projection. Indices into the list, or
`std::reference_wrapper<const Measurement>`, are equally good. All three are "views" in the sense of
chapter 1, and carry its obligation: the result is valid while the caller's list is, which the function's
comment has to say.

The parameter is a `std::span<const Measurement>`. `partial_sort` orders only as many as were asked for.
Sorting everything with `std::ranges::sort` and then shrinking is correct and does more work than needed.

A tempting wrong answer returns a `std::span<const Measurement>` of the first `count` elements after
sorting the list itself: one line, and it reorders the caller's data, which the third test notices.

## Problem 5: a command line for a game

*All five chapters: views, an optional result, an algorithm with projections, a sink parameter, a callable.*

<!-- file: solutions/ra-review-a/p05_dispatcher.h#types -->
```cpp
/// What a handler is given: the words after the verb. It may look at them and may not keep
/// them: they are parts of the line that was passed to Run().
using Words = std::span<const std::string_view>;

/// A handler answers with a reply for the player, or with nothing.
using Handler = std::function<std::optional<std::string>(Words)>;
```

- **What a handler receives:** "a list of words that it only reads" is a
  `std::span<const std::string_view>`. The words are parts of the line; the list is a view of a local
  vector. Nothing is copied to take a line apart, and nothing is valid after `Run` returns, which is fine
  for something a handler "only reads".
- **What a handler returns:** "a text, or nothing" is a `std::optional<std::string>`. The text is new, so
  it is owned.
- **How a handler is stored:** `std::function`, taken by value in `On` and moved into place, as chapter 4
  prescribes for anything a function keeps.

<!-- file: solutions/ra-review-a/p05_dispatcher.h#run -->
```cpp
/// The reply of the handler whose verb the first word of @p line stands for: the verb that
/// equals it, or else the only verb that begins with it, ignoring letter case. Nothing when the
/// line is empty, when no verb or more than one fits, or when the handler has nothing to say.
[[nodiscard]] std::optional<std::string> Run(std::string_view line) const
{
    // The words are views into `line`: nothing is copied to take the line apart.
    std::vector<std::string_view> words;
    while (true)
    {
        line.remove_prefix(std::min(line.find_first_not_of(' '), line.size()));
        if (line.empty())
        {
            break;
        }
        words.push_back(line.substr(0, line.find(' ')));
        line.remove_prefix(words.back().size());
    }
    if (words.empty())
    {
        return std::nullopt;
    }

    const Entry* const entry = Find(words.front());
    if (entry == nullptr)
    {
        return std::nullopt;
    }
    return entry->handler(Words(words).subspan(1));
}
```

The tokenising loop is chapter 1's `next_token`, including the `std::min` that keeps `remove_prefix` in
range.

<!-- file: solutions/ra-review-a/p05_dispatcher.h#find -->
```cpp
/// The entry @p typed stands for, or nullptr.
[[nodiscard]] const Entry* Find(std::string_view typed) const
{
    const auto isExactly = [typed](const Entry& entry) {
        return std::ranges::equal(entry.verb, typed, {}, Lower, Lower);
    };
    const auto startsWith = [typed](const Entry& entry) {
        return std::ranges::starts_with(entry.verb, typed, {}, Lower, Lower);
    };
    if (const auto exact = std::ranges::find_if(verbs, isExactly); exact != verbs.end())
    {
        return &*exact;
    }
    if (std::ranges::count_if(verbs, startsWith) != 1)
    {
        return nullptr;
    }
    return &*std::ranges::find_if(verbs, startsWith);
}
```

And the search is chapter 3's problem 3, returning a pointer to an entry of its own table: "one of mine,
or none".

## Fix-it set

| # | The defect | Chapter | Found by clang-tidy |
| --- | --- | --- | --- |
| 1 | `Greeter` keeps a `std::string_view` member: it is a view of the caller's variable, and follows it when that changes | 1 | no |
| 2 | `Journal::Add` reads `entry.size()` after `std::move(entry)` | 4 | yes: `bugprone-use-after-move` |
| 3 | `broadcast` forwards its arguments inside a loop: the first listener may move from them | 5 | yes: `bugprone-use-after-move`, "used after it was forwarded" |
| 4 | `use_colour` tests `if (setting)`, which asks whether there is a setting, not what it says | 2 | no |
| 5 | `drop_negative` calls `erase` with one iterator, which erases one element | 3 | no |

Each repaired file has a comment at the place where the defect was. Three of the five get past the
compilers, the sanitizers and clang-tidy: only a test, or a reader who knows what to look for, finds them.
That is the argument for knowing these five by heart.

Number 1 deserves a second look, because nothing in it is undefined behaviour. The caller's variable is
alive the whole time; the view is perfectly valid; it simply shows the new content. A dangling view is the
dramatic failure of a stored view. This is the quiet one.

## Clinic

<!-- file: solutions/ra-review-a/p06_signatures.h#answers -->
```cpp
// 1. Keeps the title of a document.
class Document
{
public:
    void SetTitle(std::string newTitle);  // kept: by value, then moved into the member

private:
    std::string title;
};

// 2. Tells how many words of a text are longer than a limit.
[[nodiscard]] std::size_t count_long_words(std::string_view text, std::size_t limit) noexcept;

// 3. Looks up the population of a city; some cities are not in the table.
[[nodiscard]] std::optional<std::int64_t> population_of(std::string_view city);

// 4. Finds the customer record with a given number in a table the caller owns; it may not exist.
struct Customer;
[[nodiscard]] const Customer* find_customer(std::span<const Customer> table, int number) noexcept;

// 5. Takes over a decoded image, tens of megabytes, to show it. Callers must not copy one by
// accident.
void show(Image&& image);

// 6. Sorts readings in place.
void sort_readings(std::span<double> readings) noexcept;

// 7. Returns the words of a line; the caller keeps the line alive while using them.
[[nodiscard]] std::vector<std::string_view> words_of(std::string_view line);

// 8. Returns the words of a line that the caller will keep after the line is gone.
[[nodiscard]] std::vector<std::string> owned_words_of(std::string_view line);

// 9. Creates an object of any Shape type from whatever its constructor needs, and owns it.
template <typename S, typename... Args>
[[nodiscard]] std::unique_ptr<S> make_shape(Args&&... args);

// 10. Calls a callback once for every open connection.
void for_each_connection(const std::function<void(const Connection&)>& callback);
```

1. **Keeps it: by value.** The caller chooses between a copy and a move (chapter 4).
2. **Only reads a text: `std::string_view`.** The count is a `std::size_t`; nothing can fail.
3. **May have no answer: `std::optional`.** One obvious reason for "no", so no error type (chapter 2).
4. **One of the caller's objects, or none: a pointer.** Not an optional, which would copy the record, and
   not a reference, which cannot say "none". The table is a `std::span<const Customer>` (chapters 1, 2).
5. **Must never be copied by accident: `Image&&`.** By value would compile for a caller who forgot
   `std::move` and copy tens of megabytes (chapter 4, design problem 1).
6. **Changes elements in place: `std::span<double>`,** without `const` (chapter 1).
7. **Parts of the argument, used while it lives: views.** A `std::vector<std::string_view>`; the vector is
   new and owned, its elements point into the line (chapter 1).
8. **Kept after the argument is gone: owning strings.** The same function with the other lifetime
   requirement, and the return type is the only place where the difference shows.
9. **Constructs something from arguments it passes through: `Args&&...`,** forwarded, and
   `std::unique_ptr<S>` for sole ownership, with the exact type `S` (chapter 5). A constraint
   `requires std::derived_from<S, Shape>` belongs on it.
10. **Takes a callable it only calls: `const std::function<...>&`,** or a template parameter
    `F&& callback`. It is not kept, so not by value; and it is called several times, so it is not
    forwarded (chapter 5).
