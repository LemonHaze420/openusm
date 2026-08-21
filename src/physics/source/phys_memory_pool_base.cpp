#include "phys_memory_pool_base.h"

#include "phys_mem.h"

template <>
void list_user_rigid_body::sub_7A2690()
{
    for (int i = 0; i < m_slot_array_size; ++i) {
        this->m_index_array[i] = i;
        this->m_alloc_list[i] = &this->m_slot_array[i];
    }

    this->m_alloc_count = 0;
}

template <>
bool list_user_rigid_body::is_member(user_rigid_body *a2)
{
    if ((unsigned int)((char *)a2 - (char *)this->m_slot_array) % sizeof(*a2)) {
        return false;
    }

    auto i = a2 - this->m_slot_array;
    if (i < 0 || i >= this->m_slot_array_size) {
        return false;
    }

    auto idx = this->m_index_array[i];
    return idx >= 0 && idx < this->m_alloc_count;
}

template <>
void list_user_rigid_body::destroy_member(user_rigid_body *data)
{
    assert(is_member(data));

    auto idx = data - this->m_slot_array;
    auto index = this->m_index_array[idx];
    if (this->m_alloc_count == 1) {
        this->sub_7A2690();
    } else {
        auto v3 = this->m_alloc_list[--this->m_alloc_count] - this->m_slot_array;
        auto *v2 = this->m_alloc_list[index];
        this->m_alloc_list[index] = this->m_alloc_list[this->m_alloc_count];
        this->m_alloc_list[this->m_alloc_count] = v2;
        this->m_index_array[idx] = this->m_alloc_count;
        this->m_index_array[v3] = index;
    }
}

template <>
unsigned list_user_rigid_body::get_alignment()
{
    return 4u;
}

template <>
void list_user_rigid_body::allocate(int num, phys_memory_heap &a3)
{
    assert(m_index_array == nullptr);

    assert(m_slot_array == nullptr);

    assert(m_alloc_list == nullptr);

    if (num > 0) {
        this->m_slot_array_size = num;
        auto align = get_alignment();
        auto size = 0x1BC * num;
        this->m_slot_array = static_cast<decltype(m_slot_array)>(a3.allocate(size, align));
        this->m_alloc_list = (decltype(m_alloc_list))&this->m_slot_array[num];
        this->m_index_array = (int *)&this->m_alloc_list[num];

        this->sub_7A2690();
    }
}
