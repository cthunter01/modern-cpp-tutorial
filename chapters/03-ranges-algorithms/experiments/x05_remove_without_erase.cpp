// Break it on purpose, chapter 3: remove elements, and forget that the container has not been told.
//
//   cmake --build --preset clang-debug -t x03_05 && build/clang-debug/bin/x03_05

#include <algorithm>
#include <iostream>
#include <vector>

namespace
{

void show(const char* when, const std::vector<int>& values)
{
    std::cout << when << values.size() << " elements:";
    for (const int value : values)
    {
        std::cout << ' ' << value;
    }
    std::cout << '\n';
}

}  // namespace

int main()
{
    //! [main]
    std::vector<int> values{1, 0, 2, 0, 3};

    const auto leftover =
        std::ranges::remove(values, 0);  // moves 1 2 3 to the front, returns the rest
    show("after remove: ", values);
    std::cout << "the leftover part has " << leftover.size() << " elements\n";

    values.erase(leftover.begin(), leftover.end());  // only this changes the size
    show("after erase:  ", values);
    //! [main]
    return 0;
}
