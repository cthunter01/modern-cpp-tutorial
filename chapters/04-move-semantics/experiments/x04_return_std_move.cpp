// Break it on purpose, chapter 4: "help" the compiler by moving a local in a return statement.
//
//   cmake --build --preset clang-debug -t x04_04

#include <iostream>
#include <utility>
#include <vector>

namespace
{

//! [function]
std::vector<int> numbers()
{
    std::vector<int> result{1, 2, 3};
#ifdef TUT_BREAK
    return std::move(result);
#else
    return result;
#endif
}
//! [function]

}  // namespace

int main()
{
    std::cout << numbers().size() << '\n';
    return 0;
}
