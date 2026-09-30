#include <gtest/gtest.h>

#include <custom_math.h>
#include <nal_math.h>

#include <vector3d.h>

TEST(NalMath, Test1)
{
    const float a2 = 0.125003f;
    vector4d a3{0.706579, -0.706934, 0.022704, 0.021800};
    vector4d a4{0.706543, -0.706543, 0.022705, -0.022850};
    const vector4d expectedResult{0.706673, -0.706983, 0.022707, 0.016219};

    const auto result = math::Slerp(a2, a3, a4);
    EXPECT_TRUE(approx_equals(result, expectedResult, LARGE_EPSILON));
}

TEST(NalMath, Test2)
{
    const float a2 = 0.125003f;
    vector4d a3{0.00854492, -0.00146484, 0.00854492, 0.999926};
    vector4d a4{0.00854492, -0.00146484, 0.00854492, 0.999926};
    const vector4d expectedResult{0.00854492, -0.00146484, 0.00854492, 0.999926};

    const auto result = math::Slerp(a2, a3, a4);
    EXPECT_TRUE(approx_equals(result, expectedResult, LARGE_EPSILON));
}
