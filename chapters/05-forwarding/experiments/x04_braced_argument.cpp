// Break it on purpose, chapter 5: forward a braced list.
//
//   cmake --build --preset clang-debug -t x05_04

#include <initializer_list>
#include <iostream>
#include <memory>
#include <vector>

int main()
{
    //! [main]
    const std::vector<int> direct({1, 2, 3});  // fine: the constructor says what the braces are

    const auto named =
        std::make_unique<std::vector<int>>(std::initializer_list<int>{1, 2, 3});  // fine

#ifdef TUT_BREAK
    const auto forwarded =
        std::make_unique<std::vector<int>>({1, 2, 3});  // what type is {1, 2, 3}?
    std::cout << forwarded->size() << '\n';
#endif
    //! [main]
    std::cout << direct.size() + named->size() << '\n';
    return 0;
}
