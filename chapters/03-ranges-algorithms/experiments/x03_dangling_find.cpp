// Break it on purpose, chapter 3: search a container that is gone by the time you look at the
// result.
//
//   cmake --build --preset clang-debug -t x03_03

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace
{

struct Person
{
    std::string name;
    int         id = 0;
};

std::vector<Person> load_people()
{
    return {{.name = "Ada", .id = 7}, {.name = "Grace", .id = 3}};
}

}  // namespace

int main()
{
    //! [main]
    const std::vector<Person> people = load_people();
    std::cout << std::ranges::find(people, 7, &Person::id)->name
              << '\n';  // fine: `people` is still here

#ifdef TUT_BREAK
    // The vector that load_people() returns is destroyed at the end of this statement...
    const auto found = std::ranges::find(load_people(), 7, &Person::id);
    std::cout << found->name << '\n';  // ...so what would `found` point at?
#endif
    //! [main]
    return 0;
}
