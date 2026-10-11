// Break it on purpose, chapter 2: value_or() with a default that is expensive to compute.
//
//   cmake --build --preset clang-debug -t x02_05 && build/clang-debug/bin/x02_05

#include <iostream>
#include <optional>

namespace
{

int g_loads = 0;  // how often the default was fetched

unsigned load_default_port()
{
    ++g_loads;  // imagine a request to a configuration server here
    return 9000;
}

}  // namespace

int main()
{
    //! [main]
    const std::optional<unsigned> configured = 8080;  // there is a value: no default is needed

    const unsigned eager = configured.value_or(load_default_port());
    std::cout << "value_or: port " << eager << ", the default was fetched " << g_loads
              << " time(s)\n";

    g_loads             = 0;
    const unsigned lazy = configured ? *configured : load_default_port();
    std::cout << "?:        port " << lazy << ", the default was fetched " << g_loads
              << " time(s)\n";

    g_loads = 0;
    const unsigned chained =
        configured.or_else([] { return std::optional<unsigned>(load_default_port()); }).value();
    std::cout << "or_else:  port " << chained << ", the default was fetched " << g_loads
              << " time(s)\n";
    //! [main]
    return 0;
}
