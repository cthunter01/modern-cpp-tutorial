// Break it on purpose, chapter 0: a reference into a vector, kept across a push_back.
//
//   cmake --build --preset clang-debug -t x00_02 && build/clang-debug/bin/x00_02
//   cmake --build --preset asan        -t x00_02 && build/asan/bin/x00_02
//
// The first build runs to the end and prints something. The second one stops with a report.

#include <iostream>
#include <vector>

int main()
{
    //! [main]
    std::vector<int> values{1, 2, 3};
    const int&       first = values.front();  // refers to the vector's current storage

    values.push_back(4);  // no room left: the vector moves to new storage and frees the old one

    std::cout << "first is " << first << '\n';  // reads the storage that was freed
    std::cout << "finished without a complaint\n";
    //! [main]
    return 0;
}
