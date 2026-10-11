// Break it on purpose, chapter 0: throw away a result that is marked [[nodiscard]].
//
//   cmake --build --preset clang-debug -t x00_01
//
// The build is expected to fail: the tutorial compiles with warnings as errors, and this is one.

#include <iostream>

[[nodiscard]] static int parse_port(const char* text)
{
    return text[0] == '8' ? 8080 : 80;
}

int main()
{
    const int port = parse_port("8080");
    std::cout << port << '\n';
#ifdef TUT_BREAK
    parse_port("80");  // computed, then dropped
#endif
    return 0;
}
