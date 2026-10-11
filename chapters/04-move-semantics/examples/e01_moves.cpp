// The listings of chapter 4: what a move is, what std::move is, and who decides which one happens.

#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{

void copy_and_move()
{
    //! [copy-and-move]
    std::string original(40, 'x');  // 40 characters, in a block on the heap
    const char* block = original.data();

    std::string copy = original;  // a second block; 40 characters copied
    std::string moved =
        std::move(original);  // no new block: `moved` took over the one `original` had

    std::cout << (copy.data() == block) << ' ' << (moved.data() == block) << '\n';  // false true
    //! [copy-and-move]
    std::cout << copy.size() << ' ' << moved.size() << '\n';
}

//! [overloads]
// What "moving" is underneath: overload resolution. A class offers two ways of being constructed
// from another object of its kind, and the compiler picks by whether the argument can still be
// needed.
struct Text  // NOLINT(cppcoreguidelines-special-member-functions): two constructors are the example
{
    explicit Text(std::string_view content) : characters(content) { }

    Text(const Text& other) : characters(other.characters)
    {
        std::cout << "copied\n";
    }  // from an lvalue
    Text(Text&& other) noexcept : characters(std::move(other.characters))
    {
        std::cout << "moved\n";
    }  // from an rvalue

    std::string characters;
};
//! [overloads]

Text make_text()
{
    return Text("made");
}

void who_decides()
{
    //! [who-decides]
    Text named("named");

    const Text a(named);             // copied: `named` has a name and may be used again
    const Text b(std::move(named));  // moved:  the caller says it will not be
    const Text c(make_text());       // neither: the result of the call is built directly as `c`
    //! [who-decides]
    std::cout << a.characters << ' ' << b.characters << ' ' << c.characters << '\n';
}

void move_is_a_cast()
{
    //! [cast]
    std::string name = "Ada Lovelace";

    std::string&& sameObject = std::move(name);  // a cast, and a reference bound to the result
    std::cout << name << " / " << sameObject
              << '\n';  // the name, twice: no constructor ran, nothing moved

    // What std::move(name) is short for. This line does move, because here a std::string is
    // constructed from the result, and the constructor that takes std::string&& is the one that
    // fits.
    const std::string taken = static_cast<std::string&&>(name);
    //! [cast]
    std::cout << taken << '\n';
}

//! [sink-by-value]
class Player
{
public:
    // A parameter the function keeps: take it by value, then move it into place.
    explicit Player(std::string playerName) : name(std::move(playerName)) { }

    void Rename(std::string newName) { name = std::move(newName); }

    [[nodiscard]] const std::string& Name() const noexcept { return name; }

private:
    std::string name;
};
//! [sink-by-value]

void sinks()
{
    //! [sink-callers]
    std::string chosen = "a name that is long enough to need the heap";

    const Player first(chosen);  // the caller keeps `chosen`: one copy, into the parameter
    const Player second(std::move(chosen));  // the caller gives it up: no copy at all
    const Player third("Grace");  // a literal: the string is built once, in the parameter
    //! [sink-callers]
    std::cout << first.Name().size() << ' ' << second.Name().size() << ' ' << third.Name() << '\n';
}

//! [rule-of-zero]
// No destructor, no copy or move operations declared: the compiler generates all of them, and each
// one copies or moves member by member. A struct of movable members is movable.
struct Order
{
    std::string              customer;
    std::vector<std::string> items;
    int                      priority = 0;
};
//! [rule-of-zero]

//! [return-local]
std::vector<std::string> split_words(std::string_view text)
{
    std::vector<std::string> words;
    while (!text.empty())
    {
        const std::size_t space = text.find(' ');
        words.emplace_back(text.substr(0, space));
        text.remove_prefix(space == std::string_view::npos ? text.size() : space + 1);
    }
    return words;  // no std::move: a local returned by name is moved at worst, and usually not even
                   // that
}
//! [return-local]

void transfer()
{
    //! [transfer]
    std::vector<std::string> queue{"first", "second", "third"};

    // Three ways to take everything out of `queue`.
    const std::vector<std::string> a = std::move(queue);  // queue: moved-from (for a vector: empty)
    queue                            = {"fourth"};        // assigning to it is always fine

    const std::vector<std::string> b = std::exchange(queue, {});  // queue: empty, by definition

    std::vector<std::string> c;
    queue = {"fifth", "sixth"};
    c.swap(queue);  // queue: has what `c` had (nothing), and keeps
                    // no storage of its own; `c` has the elements
    //! [transfer]
    std::cout << a.size() << ' ' << b.size() << ' ' << c.size() << ' ' << queue.size() << '\n';
}

}  // namespace

int main()
{
    std::cout << std::boolalpha;
    copy_and_move();
    who_decides();
    move_is_a_cast();
    sinks();
    Order       order{.customer = "Ada", .items = {"gears", "cards"}, .priority = 1};
    const Order moved = std::move(order);
    std::cout << moved.customer << ' ' << moved.items.size() << '\n';
    std::cout << split_words("to be or not").size() << '\n';
    transfer();
    return 0;
}
