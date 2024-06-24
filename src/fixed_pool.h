#pragma once

#include <cstdint>

struct fixed_pool {
    int field_0;
    void *field_4;
    int m_size;
    uint32_t m_alignment;
    int m_number_of_entries_per_block;
    int field_14;
    int field_18;
    void *m_base;
    bool m_initialized;

    fixed_pool(int a2, int a3, int a4, int a5, int a6, void *a7);

    void init(int a2, int a3, int a4, int a5, int a6, void *a7);

    void sub_4368C0();

    bool is_empty();

    void remove(void *a2);
    
    int get_entry_size() const
    {
        return this->m_size;
    }

    void *allocate_new_block();
};

template<typename T>
void *allocate_new_block(fixed_pool &pool);

extern void fixed_pool_patch();
