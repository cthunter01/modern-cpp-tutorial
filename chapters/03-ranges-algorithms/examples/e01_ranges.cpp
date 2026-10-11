// The listings of chapter 3: range algorithms, comparators and projections.

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{

//! [person]
struct Person
{
    std::string name;
    int         age = 0;
    int         id  = 0;
};
//! [person]

std::vector<Person> people()
{
    return {
        {.name = "Grace", .age = 85, .id = 3},
        {.name = "Ada", .age = 36, .id = 7},
        {.name = "Linus", .age = 54, .id = 1},
        {.name = "Barbara", .age = 85, .id = 4},
    };
}

void show(std::string_view label, const std::vector<Person>& list)
{
    std::cout << label << ':';
    for (const Person& person : list)
    {
        std::cout << ' ' << person.name;
    }
    std::cout << '\n';
}

void whole_range()
{
    //! [whole-range]
    std::vector<int> values{5, 3, 8, 1};

    std::sort(values.begin(), values.end());  // C++98: two iterators   NOLINT(modernize-use-ranges)
    std::ranges::sort(values);                // C++20: the range itself
    //! [whole-range]
    std::cout << values.front() << ' ' << values.back() << '\n';
}

void sorting()
{
    std::vector<Person> list = people();

    //! [sort-lambda]
    // Without projections: a comparator that digs the member out, twice.
    std::ranges::sort(list, [](const Person& a, const Person& b) { return a.age < b.age; });
    //! [sort-lambda]
    show("by age (lambda)", list);

    //! [sort-projection]
    std::ranges::sort(list, {}, &Person::age);                      // by age, ascending
    std::ranges::sort(list, std::ranges::greater{}, &Person::age);  // by age, descending
    std::ranges::sort(list, {}, &Person::name);                     // by name
    //! [sort-projection]
    show("by name", list);

    //! [sort-key]
    // A projection can be any callable. This one computes a sort key: oldest first, then by name.
    std::ranges::sort(list, {},
                      [](const Person& p) { return std::pair(-p.age, std::string_view(p.name)); });
    //! [sort-key]
    show("oldest first, then by name", list);
}

void searching()
{
    const std::vector<Person> list = people();

    //! [find]
    // "The element whose id is 7": a value and a projection.
    const auto ada = std::ranges::find(list, 7, &Person::id);
    if (ada != list.end())
    {
        std::cout << ada->name << " has id 7\n";
    }

    // "The first element for which this is true": a predicate. It may have a projection too.
    const auto senior = std::ranges::find_if(list, [](int age) { return age > 60; }, &Person::age);
    std::cout << senior->name << " is the first over 60\n";
    //! [find]

    //! [questions]
    const auto isOver60 = [](int age) { return age > 60; };

    std::cout << std::ranges::count_if(list, isOver60, &Person::age) << ' '    // 2
              << std::ranges::any_of(list, isOver60, &Person::age) << ' '      // 1 (true)
              << std::ranges::all_of(list, isOver60, &Person::age) << ' '      // 0 (false)
              << std::ranges::contains(list, "Linus", &Person::name) << '\n';  // 1          (C++23)
    //! [questions]

    //! [max]
    // The projection decides what is compared. What comes back is the element, not the age.
    const auto oldest =
        std::ranges::max_element(list, {}, &Person::age);              // an iterator into `list`
    const Person youngest = std::ranges::min(list, {}, &Person::age);  // a copy of the element
    std::cout << oldest->name << ' ' << youngest.name << '\n';
    //! [max]
}

//! [invoke]
// What an algorithm does with a projection: std::invoke. For a pointer to a data member that reads
// the member, for a pointer to a member function it calls it, for anything else it is an ordinary
// call.
void invoking()
{
    const Person ada{.name = "Ada", .age = 36, .id = 7};

    std::cout << std::invoke(&Person::age, ada) << ' '             // ada.age
              << std::invoke(&std::string::size, ada.name) << ' '  // ada.name.size()
              << std::invoke([](const Person& p) { return p.id; }, ada)
              << '\n';  // the lambda, called
}
//! [invoke]

//! [iequals]
constexpr char to_lower_ascii(char c) noexcept
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

constexpr bool same_ignoring_case(std::string_view a, std::string_view b) noexcept
{
    //                        range 1, range 2, comparison, projection for 1, projection for 2
    return std::ranges::equal(a, b, {}, to_lower_ascii, to_lower_ascii);
}
//! [iequals]

void removing()
{
    //! [remove]
    std::vector<int> values{4, 0, 8, 0, 0, 15};

    // remove does not remove. It moves the elements to keep to the front and returns the rest of
    // the range, which the container still has to be told to drop.
    const auto leftover = std::ranges::remove(values, 0);
    values.erase(leftover.begin(), leftover.end());

    // Since C++20 a container can do both steps itself:
    std::erase_if(values, [](int value) { return value > 10; });
    //! [remove]
    std::cout << values.size() << '\n';
}

void sorted_ranges()
{
    //! [sorted]
    std::vector<Person> list = people();
    std::ranges::sort(list, {}, &Person::age);  // 36 54 85 85

    // On a sorted range, searching is a binary search. The projection has to be the one it was
    // sorted by.
    const auto firstOver50 = std::ranges::lower_bound(list, 50, {}, &Person::age);
    std::cout << firstOver50->name << ' ' << std::ranges::is_sorted(list, {}, &Person::age) << '\n';
    //! [sorted]
}

}  // namespace

int main()
{
    whole_range();
    sorting();
    searching();
    invoking();
    std::cout << std::boolalpha << same_ignoring_case("Warn", "WARN") << ' '
              << same_ignoring_case("war", "warn") << std::noboolalpha << '\n';
    removing();
    sorted_ranges();
    return 0;
}
