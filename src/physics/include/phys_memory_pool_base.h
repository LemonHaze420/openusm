#pragma once

#include "rigid_body.h"
#include "rbc_def_contact.h"

#include <cassert>

struct phys_memory_heap;

template <typename T>
struct phys_memory_pool {
    T *m_slot_array;
    T **m_alloc_list;
    int *m_index_array;
    int m_slot_array_size;
    int m_alloc_count;

    ~phys_memory_pool()
    {
        if (this->m_slot_array != nullptr) {
            assert(m_alloc_list == (T **)(m_slot_array + m_slot_array_size));

            assert(m_index_array == (int *)(m_alloc_list + m_slot_array_size));
        }

        this->m_index_array = nullptr;
        this->m_slot_array = nullptr;
        this->m_slot_array_size = 0;
        this->m_alloc_list = nullptr;
        this->m_alloc_count = 0;
    }

    void allocate(int a2, phys_memory_heap &a3);

    bool is_member(T *data);

    void destroy_member(T *data);

    void sub_7A2690();

    static unsigned get_alignment();
};

using list_user_rigid_body = phys_memory_pool<user_rigid_body>;
using list_rigid_body = phys_memory_pool<rigid_body>;
using list_rigid_body_constraint_contact = phys_memory_pool<rigid_body_constraint_contact>;
