#include "p06_signatures.h"

#include <string>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

// The clinic has no test for your answers. This file only proves that the answer key is valid C++
// and accepts and refuses what its notes say.

namespace
{

namespace clinic = review_a::clinic;

TEST(Signatures, TheAnswerKeyBehavesAsItsNotesSay)
{
    // 2: any text, without building a std::string.
    static_assert(std::is_invocable_v<decltype(&clinic::count_long_words), const char*, int>);
    // 6: needs elements it may change.
    static_assert(std::is_invocable_v<decltype(&clinic::sort_readings), std::vector<double>&>);
    static_assert(
        !std::is_invocable_v<decltype(&clinic::sort_readings), const std::vector<double>&>);
    // 1: a caller may keep their title or give it up.
    static_assert(std::is_invocable_v<decltype(&clinic::Document::SetTitle), clinic::Document&,
                                      const std::string&>);
    static_assert(
        std::is_invocable_v<decltype(&clinic::Document::SetTitle), clinic::Document&, std::string>);
    SUCCEED();
}

}  // namespace
