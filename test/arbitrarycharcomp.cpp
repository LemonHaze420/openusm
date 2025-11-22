#include <gtest/gtest.h>

#include <custom_math.h>
#include <vector3d.h>
#include <vector4d.h>

TEST(ArbitraryPOCharComp, Test1)
{
    const float a2 = 0.250006f;
    vector4d a3 {-0.706791, 0.706905, 0.019531, 0.018715};
    vector4d a4 {-0.706791, 0.706867, 0.019531, 0.020097};
    const auto result = sub_5FD0C0(a2, a3, a4);

    EXPECT_TRUE(approx_equals(result, vector4d(-0.508389, 0.508572, 0.014048, 0.009813), LARGE_EPSILON));
}

TEST(ArbitraryPOCharComp, Test2)
{
    const float a2 = 0.750019f;
    vector4d a3 {-0.706543, 0.706543, 0.027832, 0.028626};
    vector4d a4 {-0.706791, 0.706905, 0.019531, 0.018715};
    const auto result = sub_5FD0C0(a2, a3, a4);

    EXPECT_TRUE(approx_equals(result, vector4d(0.509007, -0.509392, 0.007846, 0.012680), LARGE_EPSILON));
}

TEST(ArbitraryPOCharComp, Test3)
{
    const float a2 = 0.500013f;
    vector4d a3 {0.667383, -0.667222, -0.037991, 0.018715};
    vector4d a4 {-0.706791, 0.706905, 0.019531, 0.018715};
    const auto result = sub_5FD0C0(a2, a3, a4);

    EXPECT_TRUE(approx_equals(result, vector4d(-0.104594, 0.105326, -0.048983, 0.099320), LARGE_EPSILON));
}

TEST(ArbitraryPOCharComp, Test4)
{
    const float a2 = 0.875022f;
    vector4d a3 {-0.706791, 0.706905, 0.019531, 0.018715};
    vector4d a4 {-0.706543, 0.706543, 0.027832, 0.028626};
    const auto result = sub_5FD0C0(a2, a3, a4);

    EXPECT_TRUE(approx_equals(result, vector4d(0.667383, -0.667222, -0.037991, -0.041009), LARGE_EPSILON));
}
