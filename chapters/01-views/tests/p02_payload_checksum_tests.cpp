#include "p02_payload_checksum.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

// The tests fix three calls. They do not say what the parameter type is: they say what has to be
// accepted. If you find yourself writing three functions, look for the one type that takes all
// three.

namespace
{

// header (4 bytes), payload, trailer (2 bytes)
constexpr std::array<std::uint8_t, 10> kFrame{0xAA, 0xBB, 0xCC, 0xDD, 1, 2, 3, 4, 0xEE, 0xFF};

TEST(PayloadChecksum, OfAFrameInAVector)
{
    const std::vector<std::uint8_t> frame(kFrame.begin(), kFrame.end());
    EXPECT_EQ(ch01::payload_checksum(frame), 10);
}

TEST(PayloadChecksum, OfAFrameInAFixedBuffer)
{
    std::array<std::uint8_t, 64> buffer{};
    buffer[4]  = 200;  // first payload byte
    buffer[61] = 100;  // last payload byte
    buffer[62] = 255;  // trailer: must not count
    buffer[3]  = 255;  // header: must not count

    EXPECT_EQ(ch01::payload_checksum(buffer), (200 + 100) % 256);
}

TEST(PayloadChecksum, OfAFrameGivenAsPointerAndLength)
{
    const std::uint8_t* const data   = kFrame.data();
    const std::size_t         length = kFrame.size();

    EXPECT_EQ(ch01::payload_checksum({data, length}), 10);
}

TEST(PayloadChecksum, TheSumWrapsAt256)
{
    const std::vector<std::uint8_t> frame{0, 0, 0, 0, 250, 250, 0, 0};
    EXPECT_EQ(ch01::payload_checksum(frame), (250 + 250) % 256);
}

TEST(PayloadChecksum, AFrameWithoutPayloadGivesZero)
{
    const std::vector<std::uint8_t> onlyHeaderAndTrailer{9, 9, 9, 9, 9, 9};
    const std::vector<std::uint8_t> tooShort{9, 9, 9};
    const std::vector<std::uint8_t> nothing;

    EXPECT_EQ(ch01::payload_checksum(onlyHeaderAndTrailer), 0);
    EXPECT_EQ(ch01::payload_checksum(tooShort), 0);
    EXPECT_EQ(ch01::payload_checksum(nothing), 0);
}

}  // namespace
