#include <cstddef>

#include "d08_label.h"

// Copying a Label that is not const. This is an ordinary copy, and it has to compile: the copy
// constructor exists. If it does not compile, read the first lines of the error: which constructor
// did the compiler choose for `Label copy(original);`?

namespace
{

ch05::Label duplicate()
{
    ch05::Label original("text");
    ch05::Label copy(original);  // NOLINT(performance-unnecessary-copy-initialization): the test
    return copy;
}

}  // namespace

std::size_t use()
{
    return duplicate().Text().size();
}
