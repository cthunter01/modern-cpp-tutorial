// Break it on purpose, chapter 5: forward a literal 0 to a pointer parameter.
//
//   cmake --build --preset clang-debug -t x05_05

#include <iostream>
#include <memory>

namespace
{

//! [type]
struct Node
{
    explicit Node(Node* parentNode) : parent(parentNode) { }

    Node* parent;
};
//! [type]

}  // namespace

int main()
{
    //! [main]
    const Node root(
        0);  // compiles: the literal 0 is a null pointer constant (write nullptr anyway)

#ifdef TUT_BREAK
    const auto child = std::make_unique<Node>(0);  // the 0 is deduced as an int, and travels as one
    std::cout << (child->parent == nullptr) << '\n';
#endif
    //! [main]
    std::cout << (root.parent == nullptr) << '\n';
    return 0;
}
