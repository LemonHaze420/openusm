#include <gtest/gtest.h>

#include <string_hash.h>

TEST(StringHash, Construct)
{
    string_hash hash {};
    EXPECT_EQ(0, hash.source_hash_code);
}
