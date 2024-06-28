#include <gtest/gtest.h>

#include <matrix4x4.h>

TEST(Matrix4x4, Construct)
{
    matrix4x4 m {};
    EXPECT_EQ(m[0][0], 0.0f);
}
