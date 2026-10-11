#include "p03_signatures.h"

#include <span>
#include <string>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

// The clinic has no test for your answers: they are declarations, and the question is why. This
// file only proves that the answer key is valid C++ and behaves at the call site as the notes
// claim.

namespace
{

namespace clinic = ch01::clinic;

TEST(Signatures, TheAnswerKeyAcceptsWhatItShouldAndNothingElse)
{
    // 2: any text, without building a std::string.
    static_assert(std::is_invocable_v<decltype(&clinic::same_name), const char*, std::string>);
    // 6: read-only elements are enough.
    static_assert(std::is_invocable_v<decltype(&clinic::total), const std::vector<double>&>);
    // 7: needs elements it may change.
    static_assert(std::is_invocable_v<decltype(&clinic::clamp_samples), std::vector<float>&>);
    static_assert(
        !std::is_invocable_v<decltype(&clinic::clamp_samples), const std::vector<float>&>);
    // 8: needs the container itself.
    static_assert(!std::is_invocable_v<decltype(&clinic::add_defaults), std::span<std::string>>);
    SUCCEED();
}

}  // namespace
