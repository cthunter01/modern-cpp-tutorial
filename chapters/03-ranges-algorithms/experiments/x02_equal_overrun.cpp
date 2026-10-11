// Break it on purpose, chapter 3: the iterator form of std::equal, with the longer text first.
//
//   cmake --build --preset asan -t x03_02 && build/asan/bin/x03_02

#include <algorithm>
#include <iostream>
#include <string_view>
#include <vector>

int main()
{
    //! [main]
    const std::vector<char> raw{'w', 'a', 'r'};  // three characters, and the buffer ends there
    const std::string_view  typed(raw.data(), raw.size());
    const std::string_view  name = "warning";

    // Seven characters of `name` are compared with seven characters starting at typed.begin().
    const bool same =
        std::equal(name.begin(), name.end(), typed.begin());  // NOLINT(modernize-use-ranges)
    //! [main]
    std::cout << std::boolalpha << same << std::endl;
    return 0;
}
