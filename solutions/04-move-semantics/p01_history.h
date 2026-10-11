#pragma once

#include <cstddef>
#include <deque>
#include <optional>
#include <string>
#include <utility>

namespace ch04
{

/// The last few states of a document, for undo.
class History
{
public:
    explicit History(std::size_t capacity) : limit(capacity) { }

    //! [store]
    /// Takes @p snapshot over. An rvalue reference, on purpose: a snapshot is megabytes, and a
    /// caller who wants to keep theirs has to write the copy where everybody can see it,
    /// Store(std::string(text)).
    void Store(std::string&& snapshot)
    {
        if (limit == 0)
        {
            return;
        }
        if (snapshots.size() == limit)
        {
            snapshots.pop_front();  // the oldest goes
        }
        snapshots.push_back(std::move(snapshot));
    }
    //! [store]

    //! [undo]
    /// The most recent snapshot, which is removed from the history, or nothing when it is empty.
    [[nodiscard]] std::optional<std::string> Undo()
    {
        if (snapshots.empty())
        {
            return std::nullopt;
        }
        std::string latest =
            std::move(snapshots.back());  // out of the container before it is popped
        snapshots.pop_back();
        return latest;  // moved into the optional: `latest` is a local being returned
    }
    //! [undo]

    [[nodiscard]] std::size_t Size() const noexcept { return snapshots.size(); }

private:
    std::size_t             limit;
    std::deque<std::string> snapshots;
};

}  // namespace ch04
