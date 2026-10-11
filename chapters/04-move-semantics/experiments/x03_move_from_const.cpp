// Break it on purpose, chapter 4: std::move on something that is const.
//
//   cmake --build --preset clang-debug -t x04_03 && build/clang-debug/bin/x04_03

#include <iostream>
#include <utility>
#include <vector>

#include "tut/tracked.h"

int main()
{
    //! [main]
    const tut::Tracked        item{5};  // const: a habit, and usually a good one
    std::vector<tut::Tracked> kept;
    kept.reserve(1);
    tut::Tracked::ResetCounts();

    kept.push_back(std::move(item));  // "I am done with it, take it"

    std::cout << "copies: " << tut::Tracked::Counts().copies
              << ", moves: " << tut::Tracked::Counts().moves << '\n';
    //! [main]
    return 0;
}
