#pragma once

#include <array>
#include <string_view>

namespace ch04
{

// For each expression, which overload of which() is called: "lvalue" or "rvalue".
//
//   tut::Tracked       named{1};
//   const tut::Tracked fixed{2};
//   tut::Tracked       make();                       // returns by value
//   void sink(tut::Tracked&& parameter) { ... }      // expressions 6 and 7 are inside sink
//
inline constexpr std::array<std::string_view, 8> kPredictions{
    "lvalue",  // 1  which(named)                              a name: it can be used again
    "rvalue",  // 2  which(tut::Tracked{3})                    a temporary
    "rvalue",  // 3  which(std::move(named))                   move casts the name to an rvalue
    "rvalue",  // 4  which(make())                             the result of a call that returns by
               // value
    "lvalue",  // 5  which(std::move(fixed))                   const Tracked&& cannot bind to
               // Tracked&&
    "lvalue",  // 6  which(parameter)                          a name again, whatever its type says
    "rvalue",  // 7  which(std::move(parameter))               and so it has to be moved again
    "rvalue",  // 8  which(static_cast<tut::Tracked&&>(named)) this is all std::move is
};

}  // namespace ch04
