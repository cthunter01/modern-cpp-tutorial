// Break it on purpose, chapter 3: put the projection where the comparator belongs.
//
//   cmake --build --preset clang-debug -t x03_04

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace
{

struct Person
{
    std::string name;
    int         age = 0;
};

}  // namespace

int main()
{
    std::vector<Person> people{{.name = "Grace", .age = 85}, {.name = "Ada", .age = 36}};

    //! [main]
    std::ranges::sort(people, {}, &Person::age);  // comparator: the default; projection: the age

#ifdef TUT_BREAK
    std::ranges::sort(people, &Person::age);  // the age, in the comparator's place
#endif
    //! [main]
    std::cout << people.front().name << '\n';
    return 0;
}
