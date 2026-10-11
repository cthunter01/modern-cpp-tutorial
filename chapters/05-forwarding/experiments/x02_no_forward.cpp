// Break it on purpose, chapter 5: pass the arguments on without std::forward.
//
//   cmake --build --preset clang-debug -t x05_02

#include <iostream>
#include <memory>
#include <utility>

namespace
{

//! [function]
template <typename F, typename... Args>
decltype(auto) call(F&& function, Args&&... args)
{
    return std::forward<F>(function)(args...);  // each argument as an lvalue
}
//! [function]

int twice(int value)
{
    return 2 * value;
}

int read(std::unique_ptr<int> owned)
{
    return *owned;
}

}  // namespace

int main()
{
    //! [main]
    std::cout << call(twice, 21) << '\n';  // fine: an int can be copied

#ifdef TUT_BREAK
    std::cout << call(read, std::make_unique<int>(9)) << '\n';  // a unique_ptr cannot
#endif
    //! [main]
    static_cast<void>(read);
    return 0;
}
