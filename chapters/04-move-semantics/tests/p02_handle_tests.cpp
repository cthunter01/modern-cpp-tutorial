#include "p02_handle.h"

#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "handles.h"

// The tests fix a class ch04::Handle with a static function Open(), which acquires a handle through
// ch04::os::acquire(), and Id(), which returns the number the os API gave it. Everything else is
// checked through the os API itself: how many handles are open, and how many releases were wrong.

namespace
{

namespace os = ch04::os;

// Two owners of one handle are impossible, not merely discouraged.
static_assert(!std::is_copy_constructible_v<ch04::Handle>);
static_assert(!std::is_copy_assignable_v<ch04::Handle>);
static_assert(std::is_nothrow_move_constructible_v<ch04::Handle>);
static_assert(std::is_nothrow_move_assignable_v<ch04::Handle>);

class HandleTest : public testing::Test
{
protected:
    void SetUp() override { os::reset(); }
    void TearDown() override
    {
        EXPECT_EQ(os::open_handles(), 0) << "a handle was never released";
        EXPECT_EQ(os::bad_releases(), 0)
            << "a handle was released twice, or one that was never acquired";
    }
};

ch04::Handle open_one()
{
    return ch04::Handle::Open();
}

TEST_F(HandleTest, ReleasesItsHandleWhenItGoesOutOfScope)
{
    {
        const ch04::Handle handle = ch04::Handle::Open();
        EXPECT_EQ(os::open_handles(), 1);
        EXPECT_GE(handle.Id(), 100);
    }
    EXPECT_EQ(os::open_handles(), 0);
}

TEST_F(HandleTest, CanBeReturnedFromAFunction)
{
    const ch04::Handle handle = open_one();
    EXPECT_EQ(os::open_handles(), 1);
}

TEST_F(HandleTest, AMoveHandsTheHandleOnAndItIsReleasedOnce)
{
    {
        ch04::Handle original = ch04::Handle::Open();
        const int    id       = original.Id();

        const ch04::Handle moved(std::move(original));

        EXPECT_EQ(moved.Id(), id);
        EXPECT_EQ(os::open_handles(), 1);
    }
    EXPECT_EQ(os::open_handles(), 0);
    EXPECT_EQ(os::bad_releases(), 0) << "both objects released the handle";
}

TEST_F(HandleTest, MoveAssignmentReleasesTheHandleThatIsReplaced)
{
    ch04::Handle target = ch04::Handle::Open();
    ch04::Handle source = ch04::Handle::Open();
    const int    kept   = source.Id();
    EXPECT_EQ(os::open_handles(), 2);

    target = std::move(source);

    EXPECT_EQ(os::open_handles(), 1) << "the handle `target` owned before has to be released";
    EXPECT_EQ(target.Id(), kept);
}

TEST_F(HandleTest, HandlesCanLiveInAVector)
{
    {
        std::vector<ch04::Handle> handles;
        for (int i = 0; i < 50; ++i)  // the vector reallocates several times on the way
        {
            // NOLINTNEXTLINE(performance-inefficient-vector-operation): growing is what is tested
            handles.push_back(ch04::Handle::Open());
        }
        EXPECT_EQ(os::open_handles(), 50);

        handles.erase(handles.begin(), handles.begin() + 10);
        EXPECT_EQ(os::open_handles(), 40);
    }
    EXPECT_EQ(os::open_handles(), 0);
}

}  // namespace
