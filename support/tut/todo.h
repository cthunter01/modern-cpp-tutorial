#pragma once

#include <cstddef>
#include <exception>
#include <source_location>
#include <string>
#include <string_view>

namespace tut
{

/// What an exercise that has not been written yet throws.
///
/// Derived from std::exception directly, not from std::runtime_error or std::logic_error: a test
/// that expects one of those must not be satisfied by a function that does not exist yet.
class NotImplemented : public std::exception
{
public:
    explicit NotImplemented(const std::source_location& location) : message(Describe(location)) { }

    [[nodiscard]] const char* what() const noexcept override { return message.c_str(); }

private:
    [[nodiscard]] static std::string Describe(const std::source_location& location)
    {
        const std::string_view file  = location.file_name();
        const std::size_t      slash = file.find_last_of("/\\");
        const std::string_view basename =
            slash == std::string_view::npos ? file : file.substr(slash + 1);
        std::string text = "NOT IMPLEMENTED: ";
        text += location.function_name();
        text += " (";
        text += basename;
        text += ':';
        text += std::to_string(location.line());
        text += ')';
        return text;
    }

    std::string message;
};

/// Does nothing with its arguments, which is enough for the compiler to count them as used.
template <typename... Ts>
constexpr void unused(const Ts&... /*unused*/) noexcept
{
}

/// Never returns: throws NotImplemented, naming the function it was called from.
[[noreturn]] inline void not_implemented(
    const std::source_location location = std::source_location::current())
{
    throw NotImplemented(location);
}

}  // namespace tut

/// The body of an exercise stub: `TUT_TODO(first, second);` with every parameter of the function.
///
/// Naming the parameters keeps the stub free of unused-parameter warnings, and because the call
/// never returns, a function with a return type needs no made-up return value. Replace the whole
/// line with your code. (A macro, so that the source location is the stub's and not this header's.)
#define TUT_TODO(...) (::tut::unused(__VA_ARGS__), ::tut::not_implemented())
