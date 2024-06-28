#include <gtest/gtest.h>

#include <msimpletemplates.h>

struct A {
    simple_list<A *>::vars_t simple_list_vars {};
};

TEST(SimpleList, Construct)
{
    simple_list<A *> list {};
    EXPECT_EQ(list.size(), 0);
}

TEST(SimpleList, PushBack)
{
    simple_list<A *> list {};

    A a {};
    list.push_back(&a);

    A b {};
    list.push_back(&b);

    EXPECT_EQ(list.size(), 2);

    auto it = list.begin();
    EXPECT_EQ((*it), &a);

    ++it;
    EXPECT_EQ((*it), &b);
}

TEST(SimpleList, PushFront)
{
    simple_list<A *> list {};

    A a {};
    list.push_front(&a);

    A b {};
    list.push_front(&b);

    EXPECT_EQ(list.size(), 2);

    auto it = list.begin();
    EXPECT_EQ((*it), &b);

    ++it;
    EXPECT_EQ((*it), &a);
}

TEST(SimpleList, Erase)
{
    simple_list<A *> list {};

    auto begin = list.begin();

    A a {};
    list.push_back(&a);

    A b {};
    list.push_back(&b);

    EXPECT_EQ(list.erase(&b), begin);
    EXPECT_EQ(list.erase(&a), begin);
}


TEST(SimpleList, CheckedErase)
{
    simple_list<A *> list {};

    A a {};

    EXPECT_FALSE(list.checked_erase(&a));

    list.push_back(&a);

    EXPECT_TRUE(list.checked_erase(&a));
}

TEST(SimpleList, PopFront)
{
    simple_list<A *> list {};

    A a {};
    list.push_back(&a);

    A b {};
    list.push_back(&b);

    list.pop_front();

    EXPECT_TRUE(list.contains(&b));
    EXPECT_FALSE(list.contains(&a));
}
