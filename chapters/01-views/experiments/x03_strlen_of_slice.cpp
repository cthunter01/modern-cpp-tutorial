// Break it on purpose, chapter 1: hand a view's data() to code that expects a C string.
//
//   cmake --build --preset clang-debug -t x01_03 && build/clang-debug/bin/x01_03

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string_view>

int main()
{
    //! [main]
    const std::string_view line = "name=Ada Lovelace";
    const std::string_view key  = line.substr(0, line.find('='));

    std::cout << "key is '" << key << "' and key.size() is " << key.size() << '\n';
    std::cout << "strlen(key.data()) is " << std::strlen(key.data()) << '\n';
    std::printf("printf with %%s prints '%s'\n", key.data());
    //! [main]
    return 0;
}
