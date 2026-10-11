#pragma once

#include <concepts>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "widgets.h"

namespace review_a
{

/// Makes widgets by name.
class WidgetRegistry
{
public:
    //! [register]
    /// Records that @p name stands for "a T constructed from these arguments". The arguments are
    /// stored here, as copies or moved in, and every Create(name) constructs a new T from the
    /// stored ones.
    template <typename T, typename... Args>
        requires std::derived_from<T, Widget> &&
                 std::constructible_from<T, const std::decay_t<Args>&...>
    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward): they are forwarded, into the capture below
    void Register(std::string name, Args&&... args)
    {
        // The arguments have to outlive this call, so they go into a tuple that the lambda owns.
        // std::forward: what the caller gave up is moved into the tuple, the rest is copied. Each
        // Create() then passes the stored values as const lvalues: they are needed again next time.
        recipes.insert_or_assign(
            std::move(name),
            [stored = std::make_tuple(std::forward<Args>(args)...)] -> std::unique_ptr<Widget> {
                return std::apply([](const auto&... each) { return std::make_unique<T>(each...); },
                                  stored);
            });
    }
    //! [register]

    /// A new widget of the kind registered under @p name, or nullptr when no such name was
    /// registered.
    [[nodiscard]] std::unique_ptr<Widget> Create(std::string_view name) const
    {
        const auto found = recipes.find(name);
        return found == recipes.end() ? nullptr : found->second();
    }

private:
    std::map<std::string, std::function<std::unique_ptr<Widget>()>, std::less<>> recipes;
};

}  // namespace review_a
