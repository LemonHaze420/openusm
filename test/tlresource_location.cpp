#include <gtest/gtest.h>

#include <tlresource_location.h>

TEST(TlResourceLocation, Construct)
{
    tlresource_location loc {};
    EXPECT_EQ(loc.field_8, nullptr);
}
