// Break it on purpose, chapter 5: std::move where std::forward belongs.
//
//   cmake --build --preset clang-debug -t x05_01 && build/clang-debug/bin/x05_01

#include <functional>
#include <iostream>
#include <utility>

#include "tut/tracked.h"

namespace
{

//! [function]
template <typename F, typename... Args>
decltype(auto) call(F&& function, Args&&... args)
{
    return std::invoke(std::forward<F>(function),
                       std::move(args)...);  // every argument, unconditionally
}
//! [function]

}  // namespace

int main()
{
    std::cout << std::boolalpha;
    //! [main]
    tut::Tracked kept{7};  // the caller passes it by name and intends to go on using it

    const int value = call([](tut::Tracked item) { return item.Value(); }, kept);

    std::cout << "the function saw " << value << '\n';
    std::cout << "the caller's object was moved from: " << kept.IsMovedFrom() << '\n';
    //! [main]
    return 0;
}
