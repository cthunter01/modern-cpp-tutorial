#pragma once

// Not part of the tutorial. This header breaks the naming rules of .clang-tidy on purpose, and the
// tidy preset has a test that passes only if clang-tidy rejects it. If the HeaderFilterRegex in
// .clang-tidy ever stopped matching this repository's headers, clang-tidy would check nothing and
// report success; this file is how that would be noticed. Your editor may underline the function
// below: that is correct.

namespace tut::canary
{

inline int BadlyNamedFunction()
{
    return 1;
}

}  // namespace tut::canary
