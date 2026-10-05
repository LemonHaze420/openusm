#include <gtest/gtest.h>

#include <entity_class_entry.h>

#include <gab_manager.h>
#include <mash_info_struct.h>
#include <mvector.h>

struct from_mash_in_place_constructor;

struct A {
    A(from_mash_in_place_constructor *) {}
};

TEST(MVector, Construct)
{
    mVector<A> v{};
    EXPECT_EQ(v.size(), 0);
}


TEST(MVector, ConstructFromMash)
{
    mVector<A> v{};
    EXPECT_EQ(v.size(), 0);

    constexpr auto size = 10u;

    A *buffer[size]{new A{nullptr}};
    v.m_data = buffer;
    v.m_size = size;

    auto *v1 = new (&v) mVector<A>{static_cast<from_mash_in_place_constructor *>(nullptr)};
    EXPECT_EQ(v1->size(), 10);
}

#if STANDALONE_SYSTEM
TEST(MVector, GabDatabasePreservesMashOwnedStorage)
{
    struct Image {
        gab_database database{nullptr};
        gab_archetype *archetypes[1]{};
        gab_archetype archetype{nullptr};
        gab_expression *expressions[1]{};
        gab_expression expression{nullptr};
        gab_source sources[2]{};
    };
    alignas(Image) unsigned char storage[sizeof(Image)]{};
    auto &image = *::new (static_cast<void *>(storage)) Image;
    auto *end = reinterpret_cast<char *>(&image) + sizeof(image);
    auto &archetypes = image.database.field_0;
    archetypes.field_0 = end - reinterpret_cast<char *>(&archetypes);
    archetypes.m_size = archetypes.m_max_size = 1;
    archetypes.m_data = image.archetypes;
    archetypes.field_10 = true;
    image.archetypes[0] = &image.archetype;
    auto &expressions = image.archetype.field_4;
    expressions.field_0 = end - reinterpret_cast<char *>(&expressions);
    expressions.m_size = expressions.m_max_size = 1;
    expressions.m_data = image.expressions;
    expressions.field_10 = true;
    image.expressions[0] = &image.expression;
    auto &sources = image.expression.field_8;
    sources.field_0 = end - reinterpret_cast<char *>(&sources);
    sources.m_size = sources.m_max_size = 2;
    sources.m_data = image.sources;
    image.sources[1].field_4 = 0x12345678;

    image.database.destruct_mashed_class();

    EXPECT_EQ(archetypes.m_data, nullptr);
    EXPECT_EQ(archetypes.m_size, 0);
    EXPECT_EQ(expressions.m_data, nullptr);
    EXPECT_EQ(expressions.m_size, 0);
    EXPECT_EQ(sources.m_data, nullptr);
    EXPECT_EQ(sources.m_size, 0);
    EXPECT_EQ(image.archetypes[0], nullptr);
    EXPECT_EQ(image.expressions[0], nullptr);
    EXPECT_EQ(image.sources[1].field_4, 0x12345678u);
    image.~Image();
}

TEST(MVector, GabSourcesReleaseHeapArrayAndResetOwnership)
{
    mVectorBasic<gab_source> sources;
    sources.m_data = new gab_source[2]{};
    sources.m_size = sources.m_max_size = 2;

    sources.destruct_mashed_class();

    EXPECT_EQ(sources.m_data, nullptr);
    EXPECT_EQ(sources.m_size, 0);
    EXPECT_EQ(sources.m_max_size, 0);
    EXPECT_EQ(sources.field_0, 0);
    sources.destruct_mashed_class();
    EXPECT_EQ(sources.m_data, nullptr);
}
#endif
