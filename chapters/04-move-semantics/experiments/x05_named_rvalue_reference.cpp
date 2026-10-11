// Break it on purpose, chapter 4: forget that a parameter with a name is an lvalue.
//
//   cmake --build --preset clang-debug -t x04_05 && build/clang-debug/bin/x04_05

#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace
{

//! [function]
void keep(std::vector<std::string>& store, std::string&& text)
{
    store.push_back(text);  // `text` is declared as an rvalue reference...
}
//! [function]

}  // namespace

int main()
{
    std::cout << std::boolalpha;
    //! [main]
    std::vector<std::string> store;
    std::string              message(40, 'm');
    const char* const        characters = message.data();

    keep(store, std::move(message));  // the caller gave the string up

    std::cout << "the stored string is the caller's block of characters: "
              << (store.front().data() == characters) << '\n';
    std::cout << "the caller's string still has " << message.size() << " characters\n";
    //! [main]
    return 0;
}
