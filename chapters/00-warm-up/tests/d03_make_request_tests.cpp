#include "d03_make_request.h"

#include <gtest/gtest.h>

namespace
{

TEST(MakeRequest, OrdinaryPathsKeepTheDefaults)
{
    const ch00::Request request = ch00::make_request("/index.html");
    EXPECT_EQ(request.method, "GET");
    EXPECT_EQ(request.path, "/index.html");
    EXPECT_EQ(request.timeoutSeconds, 30);
    EXPECT_TRUE(request.keepAlive);
}

TEST(MakeRequest, UploadsArePostedAndGetMoreTime)
{
    const ch00::Request request = ch00::make_request("/upload/photo.jpg");
    EXPECT_EQ(request.method, "POST");
    EXPECT_EQ(request.path, "/upload/photo.jpg");
    EXPECT_EQ(request.timeoutSeconds, 120);
    EXPECT_FALSE(request.keepAlive);
}

}  // namespace
