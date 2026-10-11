// The small C++17/20/23 conveniences of chapter 0, each in the shape the chapter quotes.

#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace
{

//! [nodiscard]
[[nodiscard]] bool save(const std::string& text)
{
    return !text.empty();
}
//! [nodiscard]

//! [designated-type]
struct Window
{
    std::string title;
    int         width      = 800;
    int         height     = 600;
    bool        fullscreen = false;
};
//! [designated-type]

//! [enum]
enum class Level : std::uint8_t
{
    Error,
    Warn,
    Info,
};
//! [enum]

void structured_bindings()
{
    //! [bindings]
    const std::map<std::string, int> stock{{"apple", 3}, {"pear", 0}, {"plum", 12}};

    for (const auto& [name, count] : stock)  // each element is a std::pair<const std::string, int>
    {
        std::cout << name << ": " << count << '\n';
    }
    //! [bindings]
}

void if_with_initialiser()
{
    const std::map<std::string, int> stock{{"apple", 3}, {"pear", 0}, {"plum", 12}};
    //! [if-init]
    if (const auto found = stock.find("pear"); found != stock.end())
    {
        std::cout << "pear is listed, " << found->second << " in stock\n";
    }
    // `found` no longer exists here.
    //! [if-init]
}

void class_template_argument_deduction()
{
    //! [ctad]
    std::mutex mutex;

    const std::pair        entry{"plum", 12};       // std::pair<const char*, int>
    const std::vector      primes{2, 3, 5, 7};      // std::vector<int>
    const std::array       corners{0.0, 0.5, 1.0};  // std::array<double, 3>
    const std::scoped_lock lock(mutex);             // std::scoped_lock<std::mutex>
    //! [ctad]
    std::cout << entry.first << ' ' << primes.size() << ' ' << corners.size() << '\n';
}

void designated_initialisers()
{
    //! [designated]
    const Window dialog{.title = "Save as", .width = 400, .height = 300};
    //! [designated]
    std::cout << dialog.title << ' ' << dialog.width << 'x' << dialog.height
              << (dialog.fullscreen ? " fullscreen" : " windowed") << '\n';
}

void to_underlying()
{
    //! [to-underlying]
    const Level level = Level::Info;
    std::cout << "level " << static_cast<int>(std::to_underlying(level)) << '\n';
    //! [to-underlying]
}

void exchange()
{
    //! [exchange]
    std::string pending = "disk almost full";

    const std::string taken =
        std::exchange(pending, {});  // take the value, leave an empty one behind
    //! [exchange]
    std::cout << "taken: '" << taken << "', left behind: '" << pending << "'\n";
}

}  // namespace

int main()
{
    structured_bindings();
    if_with_initialiser();
    class_template_argument_deduction();
    designated_initialisers();
    to_underlying();
    exchange();
    if (save("report"))
    {
        std::cout << "saved\n";
    }
    return 0;
}
