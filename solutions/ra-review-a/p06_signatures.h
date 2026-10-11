#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// The answer key of the clinic: declarations only. The reasons are in README.md.

namespace review_a::clinic
{

struct Image;
struct Connection;
struct Shape;

//! [answers]
// 1. Keeps the title of a document.
class Document
{
public:
    void SetTitle(std::string newTitle);  // kept: by value, then moved into the member

private:
    std::string title;
};

// 2. Tells how many words of a text are longer than a limit.
[[nodiscard]] std::size_t count_long_words(std::string_view text, std::size_t limit) noexcept;

// 3. Looks up the population of a city; some cities are not in the table.
[[nodiscard]] std::optional<std::int64_t> population_of(std::string_view city);

// 4. Finds the customer record with a given number in a table the caller owns; it may not exist.
struct Customer;
[[nodiscard]] const Customer* find_customer(std::span<const Customer> table, int number) noexcept;

// 5. Takes over a decoded image, tens of megabytes, to show it. Callers must not copy one by
// accident.
void show(Image&& image);

// 6. Sorts readings in place.
void sort_readings(std::span<double> readings) noexcept;

// 7. Returns the words of a line; the caller keeps the line alive while using them.
[[nodiscard]] std::vector<std::string_view> words_of(std::string_view line);

// 8. Returns the words of a line that the caller will keep after the line is gone.
[[nodiscard]] std::vector<std::string> owned_words_of(std::string_view line);

// 9. Creates an object of any Shape type from whatever its constructor needs, and owns it.
template <typename S, typename... Args>
[[nodiscard]] std::unique_ptr<S> make_shape(Args&&... args);

// 10. Calls a callback once for every open connection.
void for_each_connection(const std::function<void(const Connection&)>& callback);
//! [answers]

}  // namespace review_a::clinic
