// Break it on purpose, chapter 4: a move constructor that does not promise not to throw.
//
//   cmake --build --preset clang-debug -t x04_01 && build/clang-debug/bin/x04_01

#include <iostream>
#include <vector>

namespace
{

int g_copies = 0;

//! [types]
struct Careful
{
    Careful() = default;
    Careful(const Careful& /*unused*/) { ++g_copies; }
    Careful(Careful&& /*unused*/) { }  // a move constructor, but it might throw
};

struct Confident
{
    Confident() = default;
    Confident(const Confident& /*unused*/) { ++g_copies; }
    Confident(Confident&& /*unused*/) noexcept { }  // the same, with a promise
};
//! [types]

template <typename Element>
int copies_while_growing()
{
    g_copies = 0;
    std::vector<Element> elements;
    for (int i = 0; i < 16; ++i)
    {
        elements.emplace_back();  // constructs in place: only a reallocation can copy or move
    }
    return g_copies;
}

}  // namespace

int main()
{
    const int careful   = copies_while_growing<Careful>();
    const int confident = copies_while_growing<Confident>();
    std::cout << "move may throw:  the vector copied its elements while growing? "
              << (careful > 0 ? "yes" : "no") << " (" << careful << " copies)\n";
    std::cout << "move is noexcept: the vector copied its elements while growing? "
              << (confident > 0 ? "yes" : "no") << " (" << confident << " copies)\n";
    return 0;
}
