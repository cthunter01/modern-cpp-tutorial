// Break it on purpose, chapter 1: the same mistake as x03, on a buffer with nothing behind it.
//
//   cmake --build --preset asan -t x01_04 && build/asan/bin/x01_04

#include <cstring>
#include <iostream>
#include <string_view>
#include <vector>

int main()
{
    //! [main]
    const std::vector<char> received{'P', 'I', 'N', 'G'};  // four bytes off a socket: no terminator
    const std::string_view  message(received.data(), received.size());

    std::cout << "message.size() is " << message.size() << '\n';
    std::cout << "strlen(message.data()) is " << std::strlen(message.data()) << '\n';
    //! [main]
    return 0;
}
