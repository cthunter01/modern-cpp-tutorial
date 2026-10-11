#include "tut/todo.h"

#include <cstddef>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

#include <gtest/gtest.h>

// The tutorial's own machinery, checked: what every exercise stub is made of has to behave as the
// chapters say it does.

namespace
{

// The shapes a stub takes. None of them may produce a warning: these compile with the full warning
// set.
[[nodiscard]] int function_stub(int first, const std::string& second)
{
    TUT_TODO(first, second);
}

template <typename T, typename... Args>
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward): a stub forwards nothing yet
[[nodiscard]] std::unique_ptr<T> template_stub(Args&&... args)
{
    TUT_TODO(args...);
}

// A constexpr function whose body can never be a constant expression is still valid C++23 (P2448),
// as long as nobody asks for it at compile time.
[[nodiscard]] constexpr std::string_view constexpr_stub(std::string_view text)
{
    TUT_TODO(text);
}

class ClassStub
{
public:
    explicit ClassStub(std::size_t capacity) { TUT_TODO(capacity); }
    [[nodiscard]] static int Take() { TUT_TODO(); }
};

TEST(Todo, ThrowsNotImplemented)
{
    EXPECT_THROW(static_cast<void>(function_stub(1, "two")), tut::NotImplemented);
    EXPECT_THROW(static_cast<void>(template_stub<int>(1, 2.0)), tut::NotImplemented);
    EXPECT_THROW(static_cast<void>(constexpr_stub("text")), tut::NotImplemented);
    EXPECT_THROW(ClassStub{3}, tut::NotImplemented);
    EXPECT_THROW(static_cast<void>(ClassStub::Take()), tut::NotImplemented);
}

TEST(Todo, TheMessageNamesTheStubAndWhereItIs)
{
    try
    {
        static_cast<void>(function_stub(1, "two"));
        FAIL() << "function_stub returned";
    }
    catch (const tut::NotImplemented& e)
    {
        const std::string_view what = e.what();
        EXPECT_TRUE(what.starts_with("NOT IMPLEMENTED: ")) << what;
        EXPECT_TRUE(what.contains("function_stub")) << what;
        EXPECT_TRUE(what.contains("(todo_tests.cpp:")) << what;
    }
}

TEST(Todo, IsNotOneOfTheExceptionsATestMightExpect)
{
    static_assert(!std::is_base_of_v<std::runtime_error, tut::NotImplemented>);
    static_assert(!std::is_base_of_v<std::logic_error, tut::NotImplemented>);
    static_assert(std::is_base_of_v<std::exception, tut::NotImplemented>);
}

}  // namespace
