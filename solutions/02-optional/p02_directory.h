#pragma once

#include <map>
#include <optional>
#include <string>
#include <utility>

namespace ch02
{

//! [profile]
struct Profile
{
    std::string                name;
    std::optional<std::string> nickname;   // most people have none
    std::optional<int>         managerId;  // the person at the top has none; 0 is somebody's id
};
//! [profile]

class Directory
{
public:
    void Add(int id, Profile profile) { profiles.insert_or_assign(id, std::move(profile)); }

    /// The nickname of @p id if there is one, else the name, else "<unknown #id>".
    [[nodiscard]] std::string DisplayName(int id) const
    {
        const Profile* const profile = Find(id);
        return profile != nullptr ? DisplayNameOf(*profile)
                                  : "<unknown #" + std::to_string(id) + ">";
    }

    //! [manager]
    /// The display name of the manager of @p id, or "nobody" when @p id is unknown, has no manager,
    /// or names a manager who is not in the directory.
    [[nodiscard]] std::string ManagerName(int id) const
    {
        const Profile* const person = Find(id);
        if (person == nullptr || !person->managerId)
        {
            return "nobody";
        }
        const Profile* const manager = Find(*person->managerId);
        return manager != nullptr ? DisplayNameOf(*manager) : "nobody";
    }
    //! [manager]

private:
    //! [find]
    /// The profile of @p id, or nullptr. "A reference to something that may not exist" is a
    /// pointer: std::optional cannot hold a reference before C++26.
    [[nodiscard]] const Profile* Find(int id) const
    {
        const auto found = profiles.find(id);
        return found != profiles.end() ? &found->second : nullptr;
    }
    //! [find]

    [[nodiscard]] static std::string DisplayNameOf(const Profile& profile)
    {
        return profile.nickname.value_or(profile.name);
    }

    std::map<int, Profile> profiles;
};

}  // namespace ch02
