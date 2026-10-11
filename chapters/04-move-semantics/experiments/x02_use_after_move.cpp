// Break it on purpose, chapter 4: look at objects after they have been moved from.
//
//   cmake --build --preset clang-debug -t x04_02 && build/clang-debug/bin/x04_02

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>

int main()
{
    std::cout << std::boolalpha;

    //! [main]
    std::string text(40, 'x');
    std::string textTaker = std::move(text);
    std::cout << "string:      size " << text.size() << '\n';

    std::optional<std::string> maybe(std::string(40, 'y'));
    std::optional<std::string> maybeTaker = std::move(maybe);
    std::cout << "optional:    has a value: " << maybe.has_value() << '\n';

    std::unique_ptr<int> pointer = std::make_unique<int>(7);
    std::unique_ptr<int> owner   = std::move(pointer);
    std::cout << "unique_ptr:  is null: " << (pointer == nullptr) << '\n';

    int number      = 7;
    int numberTaker = std::move(number);
    std::cout << "int:         " << number << '\n';
    //! [main]

    std::cout << textTaker.size() + maybeTaker->size() +
                     static_cast<std::size_t>(*owner + numberTaker)
              << '\n';
    return 0;
}
