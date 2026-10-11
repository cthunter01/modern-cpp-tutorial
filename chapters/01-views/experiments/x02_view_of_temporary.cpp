// Break it on purpose, chapter 1: a view of a string that lives for one statement.
//
//   cmake --build --preset clang-debug -t x01_02 && build/clang-debug/bin/x01_02
//   cmake --build --preset asan        -t x01_02 && build/asan/bin/x01_02

#include <iostream>
#include <string>
#include <string_view>

namespace
{

//! [functions]
std::string make_greeting(const std::string& name)
{
    return "Hello, " + name + ", and welcome back";  // a new string, returned by value
}

std::string_view first_word(std::string_view text)
{
    return text.substr(0, text.find(' '));  // correct: a part of its argument
}
//! [functions]

}  // namespace

int main()
{
    //! [main]
    // make_greeting() returns a std::string that nobody keeps. It lives until the end of this
    // statement.
    const std::string_view word = first_word(make_greeting("Ada"));

    std::cout << "the first word has " << word.size() << " characters\n";
    std::cout << "and it is: " << word << '\n';  // the characters were freed one line ago
    //! [main]
    return 0;
}
