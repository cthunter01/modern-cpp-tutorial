#include "d04_port_or.h"

#include <functional>

#include <gtest/gtest.h>

namespace
{

/// A fallback that counts how often it was asked.
class CountingFallback
{
public:
    [[nodiscard]] unsigned operator()()
    {
        ++calls;
        return 9000;
    }

    [[nodiscard]] int Calls() const noexcept { return calls; }

private:
    int calls = 0;
};

TEST(PortOr, UsesTheConfiguredPort)
{
    const ch02::Config config{{{"host", "example.org"}, {"port", "8080"}}};
    CountingFallback   fallback;

    EXPECT_EQ(ch02::port_or(config, std::ref(fallback)), 8080U);
}

TEST(PortOr, DoesNotAskTheFallbackWhenItHasAPort)
{
    const ch02::Config config{{{"port", "8080"}}};
    CountingFallback   fallback;

    static_cast<void>(ch02::port_or(config, std::ref(fallback)));

    EXPECT_EQ(fallback.Calls(), 0) << "the fallback is expensive and its answer was not needed";
}

TEST(PortOr, AsksTheFallbackWhenThereIsNoPort)
{
    const ch02::Config config{{{"host", "example.org"}}};
    CountingFallback   fallback;

    EXPECT_EQ(ch02::port_or(config, std::ref(fallback)), 9000U);
    EXPECT_EQ(fallback.Calls(), 1);
}

TEST(PortOr, AsksTheFallbackWhenThePortIsNotANumber)
{
    const ch02::Config config{{{"port", "http"}}};
    CountingFallback   fallback;

    EXPECT_EQ(ch02::port_or(config, std::ref(fallback)), 9000U);
    EXPECT_EQ(fallback.Calls(), 1);
}

TEST(PortOr, PortZeroIsAConfiguredPort)
{
    const ch02::Config config{{{"port", "0"}}};
    CountingFallback   fallback;

    EXPECT_EQ(ch02::port_or(config, std::ref(fallback)), 0U);
    EXPECT_EQ(fallback.Calls(), 0);
}

}  // namespace
