#include "tidy_canary/canary.h"

namespace tut::canary
{

int use_it()
{
    return BadlyNamedFunction();
}

}  // namespace tut::canary
