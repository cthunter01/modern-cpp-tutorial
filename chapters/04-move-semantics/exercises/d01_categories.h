#pragma once

#include <array>
#include <string_view>

namespace ch04
{

// For each expression, which overload of which() (see which.h) is called: the one for lvalues or
// the one for rvalues? Replace every "?" by "lvalue" or by "rvalue". Decide before you run the
// tests.
//
//   tut::Tracked       named{1};
//   const tut::Tracked fixed{2};
//   tut::Tracked       make();                       // returns by value
//   void sink(tut::Tracked&& parameter) { ... }      // expressions 6 and 7 are inside sink
//
inline constexpr std::array<std::string_view, 8> kPredictions{
    "?",  // 1  which(named)
    "?",  // 2  which(tut::Tracked{3})
    "?",  // 3  which(std::move(named))
    "?",  // 4  which(make())
    "?",  // 5  which(std::move(fixed))
    "?",  // 6  which(parameter)
    "?",  // 7  which(std::move(parameter))
    "?",  // 8  which(static_cast<tut::Tracked&&>(named))
};

}  // namespace ch04
