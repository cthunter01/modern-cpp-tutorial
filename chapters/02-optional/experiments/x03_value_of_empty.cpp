// Break it on purpose, chapter 2: .value() on an empty optional.
//
//   cmake --build --preset clang-debug -t x02_03 && build/clang-debug/bin/x02_03

#include <iostream>
#include <optional>

int main()
{
    //! [main]
    const std::optional<unsigned> port;  // empty

    std::cout << "listening on port " << port.value() << std::endl;  // checked: throws
    //! [main]
    return 0;
}
