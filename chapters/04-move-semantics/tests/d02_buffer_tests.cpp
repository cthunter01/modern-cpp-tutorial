#include "d02_buffer.h"

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

// These tests look at objects after they were moved from, on purpose: what state a Buffer is left
// in is part of what the drill asks for.
//
// NOLINTBEGIN(bugprone-use-after-move, clang-analyzer-cplusplus.Move)

// Run this drill under the asan preset as well: it reports a block that is freed twice, and one
// that is never freed. An ordinary build sees neither.

namespace
{

ch04::Buffer numbered(std::size_t count)
{
    ch04::Buffer buffer(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        buffer.Data()[i] = static_cast<int>(i);  // NOLINT(*-pointer-arithmetic)
    }
    return buffer;
}

TEST(Buffer, ACopyHasItsOwnStorageWithTheSameValues)
{
    ch04::Buffer       original = numbered(4);
    const ch04::Buffer copy(original);

    ASSERT_EQ(copy.Size(), 4U);
    EXPECT_NE(copy.Data(), original.Data());
    EXPECT_EQ(copy.Data()[3], 3);  // NOLINT(*-pointer-arithmetic)

    original.Data()[3] = 99;       // NOLINT(*-pointer-arithmetic)
    EXPECT_EQ(copy.Data()[3], 3);  // NOLINT(*-pointer-arithmetic): the copy did not change
}

TEST(Buffer, CopyAssignmentReplacesTheContent)
{
    const ch04::Buffer source = numbered(3);
    ch04::Buffer       target(10);

    target = source;

    ASSERT_EQ(target.Size(), 3U);
    EXPECT_NE(target.Data(), source.Data());
    EXPECT_EQ(target.Data()[2], 2);  // NOLINT(*-pointer-arithmetic)
}

TEST(Buffer, AssigningABufferToItselfChangesNothing)
{
    ch04::Buffer        buffer = numbered(5);
    const ch04::Buffer& alias  = buffer;

    buffer = alias;

    ASSERT_EQ(buffer.Size(), 5U);
    EXPECT_EQ(buffer.Data()[4], 4);  // NOLINT(*-pointer-arithmetic)
}

TEST(Buffer, MovingTakesTheStorageAndLeavesTheSourceEmpty)
{
    ch04::Buffer     source = numbered(6);
    const int* const block  = source.Data();

    const ch04::Buffer target(std::move(source));

    EXPECT_EQ(target.Data(), block)
        << "a move takes the block over; it does not allocate a new one";
    EXPECT_EQ(target.Size(), 6U);
    // The documented state of a moved-from Buffer.
    EXPECT_EQ(source.Size(), 0U);
    EXPECT_EQ(source.Data(), nullptr);
}

TEST(Buffer, MoveAssignmentTakesTheStorageToo)
{
    ch04::Buffer     source = numbered(6);
    const int* const block  = source.Data();
    ch04::Buffer     target(100);

    target = std::move(source);

    EXPECT_EQ(target.Data(), block);
    EXPECT_EQ(target.Size(), 6U);
    EXPECT_EQ(source.Size(), 0U);
    EXPECT_EQ(source.Data(), nullptr);
}

TEST(Buffer, AMovedFromBufferCanBeAssignedAgain)
{
    ch04::Buffer       source = numbered(2);
    const ch04::Buffer target(std::move(source));

    source = numbered(3);

    EXPECT_EQ(source.Size(), 3U);
    EXPECT_EQ(target.Size(), 2U);
}

TEST(Buffer, AVectorMovesItsBuffersWhenItGrows)
{
    std::vector<ch04::Buffer> buffers;
    buffers.push_back(numbered(8));
    const int* const first = buffers.front().Data();

    for (int i = 0; i < 100; ++i)  // far beyond the first capacity: the vector reallocates
    {
        buffers.push_back(numbered(1));
    }

    // A vector moves its elements to the new storage only if their move constructor promises not to
    // throw. Otherwise it copies them, to be able to go back, and this block would be a new one.
    EXPECT_TRUE(std::is_nothrow_move_constructible_v<ch04::Buffer>);
    EXPECT_EQ(buffers.front().Data(), first);
}

}  // namespace

// NOLINTEND(bugprone-use-after-move, clang-analyzer-cplusplus.Move)
