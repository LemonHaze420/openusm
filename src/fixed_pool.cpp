#include "fixed_pool.h"

#include "common.h"
#include "dynamic_rtree.h"
#include "fixed_allocator_base.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

#include <cassert>

VALIDATE_SIZE(fixed_pool, 0x24);

#if 0
static Var<fixed_allocator_base> g_fixed_allocator{0x0095BB58};
#endif

fixed_pool::fixed_pool(int a2, int a3, int a4, int a5, int a6, void *base)
{
    this->field_0 = 0;
    this->init(a2, a3, a4, a5, a6, base);
}

void *sub_501DD0()
{
    TRACE("sub_501DD0");

    static fixed_allocator_base g_fixed_allocator{4u};
    return &g_fixed_allocator;
}

void fixed_pool::init(int size, int a3, int a4, int a5, int a6, void *base)
{
    sp_log("%d", size);
    this->m_size = size;
    this->m_alignment = a4;
    this->m_number_of_entries_per_block = a3;
    this->field_18 = a5;

    this->m_base = (base != nullptr ? base : sub_501DD0());

    assert(m_alignment >= 4);
    assert(m_number_of_entries_per_block >= 1);

    auto v8 = this->m_size / this->m_alignment;
    if ((this->m_size & (this->m_alignment - 1)) != 0) {
        ++v8;
    }

    this->m_size = this->m_alignment * v8;
    assert(m_size > 0);

    this->field_14 = 0;
    this->field_4 = nullptr;
    this->m_initialized = true;
    for (auto i = 0; i < a6; ++i) {
        this->sub_4368C0();
    }
}

bool fixed_pool::is_empty()
{
    auto v5 = 0;
    for (auto *i = (uint32_t *)this->field_0; i != nullptr; i = (uint32_t *)*i) {
        ++v5;
    }

    auto v3 = 0;
    for (auto *j = static_cast<uint32_t *>(this->field_4); j != nullptr; j = (uint32_t *)*j) {
        ++v3;
    }

    return v3 == this->m_number_of_entries_per_block * v5;
}

void *fixed_pool::allocate_new_block()
{
    assert(m_initialized);

    if (!this->field_4) {
        if (!this->field_18 && this->field_14 > 0) {
            return nullptr;
        }

        this->sub_4368C0();
    }

    auto v2 = (void **)this->field_4;
    this->field_4 = *v2;
    return v2;
}

void fixed_pool::remove(void *a2)
{
    assert(m_initialized);

    struct {
        void *field_0;
    } *iterator = static_cast<decltype(iterator)>(a2);

    iterator->field_0 = this->field_4;
    this->field_4 = a2;
}

void fixed_pool::sub_4368C0()
{
    if constexpr (0) {
        auto *base = static_cast<fixed_allocator_base *>(m_base);
        assert(base != nullptr);

        auto *v2 = (int *)base->alloc(this->m_alignment + this->m_size * this->m_number_of_entries_per_block + 4);
        assert(v2 != nullptr && "Fixed Pool Allocator: Out Of Memory");

        *v2 = this->field_0;
        auto v3 = this->m_alignment;
        auto v4 = this->m_number_of_entries_per_block;
        auto v5 = this->field_4;
        auto v6 = this->m_size;
        this->field_0 = (int)v2;
        auto *v7 = (int *)(v3 + ((unsigned int)(v2 + 1) & ~(v3 - 1)));
        auto *v8 = v7;
        if (v4 - 1 > 0) {
            auto v9 = v4 - 1;
            do {
                --v9;
                *v8 = (int)((char *)v8 + v6);
                v8 = (int *)((char *)v8 + v6);
            } while (v9);
        }

        *v8 = (int)v5;
        this->field_4 = v7;
        ++this->field_14;
    } else {
        THISCALL(0x4368C0, this);
    }
}

template <>
void *allocate_new_block<rtree_hash_entry>(fixed_pool &pool)
{
    assert(pool.get_entry_size() == sizeof(rtree_hash_entry));
    return pool.allocate_new_block();
}

void fixed_pool_patch()
{
    SET_JUMP(0x00501DD0, sub_501DD0);
}
