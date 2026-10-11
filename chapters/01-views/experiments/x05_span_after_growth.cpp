// Break it on purpose, chapter 1: a span of a vector, kept while the vector grows.
//
//   cmake --build --preset clang-debug -t x01_05 && build/clang-debug/bin/x01_05
//   cmake --build --preset asan        -t x01_05 && build/asan/bin/x01_05

#include <iostream>
#include <span>
#include <vector>

int main()
{
    //! [main]
    std::vector<int>           readings{10, 20, 30};
    const std::span<const int> window(
        readings);  // a pointer to readings' storage, and the number 3

    readings.push_back(
        40);  // the vector moves to bigger storage; `window` still points at the old one

    std::cout << "window has " << window.size() << " elements\n";
    std::cout << "the first one is " << window.front() << '\n';
    //! [main]
    return 0;
}
