// Break it on purpose, chapter 5: put the dots in the wrong place.
//
//   cmake --build --preset clang-debug -t x05_03

#include <iostream>
#include <utility>

namespace
{

int sum(int a)
{
    return a;
}

int sum(int a, int b)
{
    return a + b;
}

//! [function]
template <typename... Args>
int relay(Args&&... args)
{
    return sum(std::forward<Args...>(args...));  // not std::forward<Args>(args)...
}
//! [function]

}  // namespace

int main()
{
    //! [main]
    std::cout << sum(1, 2) << ' ' << relay(1)
              << '\n';  // relay(1) compiles: one argument, one spelling

#ifdef TUT_BREAK
    std::cout << relay(1, 2) << '\n';
#endif
    //! [main]
    return 0;
}
