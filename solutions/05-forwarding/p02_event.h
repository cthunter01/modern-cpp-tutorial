#pragma once

#include <functional>
#include <utility>
#include <vector>

namespace ch05
{

//! [event]
/// Something that happens, and carries values of the types Ts to everyone who asked to be told.
template <typename... Ts>
class Event
{
public:
    using Listener = std::function<void(const Ts&...)>;

    void Subscribe(Listener listener) { listeners.push_back(std::move(listener)); }

    /// Tells every listener, in the order they subscribed. Each of them sees the same, intact
    /// values: they are passed by const reference, never forwarded, because the second listener
    /// needs them too.
    void Emit(const Ts&... values) const
    {
        for (const Listener& listener : listeners)
        {
            listener(values...);
        }
    }

private:
    std::vector<Listener> listeners;
};
//! [event]

}  // namespace ch05
