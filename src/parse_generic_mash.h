#pragma once

#include "config.h"
#include "log.h"

struct resource_directory;
struct mission_table_container;

inline constexpr auto _MASH_FLAG_IN_USE = 0x80000000;

struct generic_mash_header {
    uint32_t safety_key;
    int field_4;
    int field_8;
    uint16_t class_id;
    uint16_t field_E;

    auto get_class_id() const
    {
        return this->class_id;
    }

    bool is_flagged(uint32_t f) const
    {
        return (f & this->field_4) != 0;
    }

    auto *get_mash_data()
    {
        return (bit_cast<uint8_t *>(this) + this->field_8);
    }

    auto generate_safety_key() const
    {
        return ((this->field_8 + 0x7BADBA5D - (this->field_4 & 0xFFFFFFF) + this->class_id + this->field_E) &
                0xFFFFFFF) |
               0x70000000;
    }
};

struct generic_mash_data_ptrs {
    uint8_t *field_0;
    uint8_t *field_4;

    generic_mash_data_ptrs() = default;

    generic_mash_data_ptrs(uint8_t *a1, uint8_t *a2) : field_0(a1), field_4(a2) {}

    template <typename T>
    T *get(uint32_t num = 1)
    {
        auto *res = bit_cast<T *>(this->field_0);
        this->field_0 += sizeof(T) * num;
        return res;
    }

    template <typename T>
    T *get_from_shared(uint32_t num = 1)
    {
        auto *res = bit_cast<T *>(this->field_4);
        this->field_4 += sizeof(T) * num;
        return res;
    }

    void rebase(uint32_t i)
    {
        uint32_t v9 = i - ((uint32_t)this->field_0 % i);
        if (v9 < i) {
            this->field_0 += v9;
        }
    }

    void rebase_shared(uint32_t i)
    {
        uint32_t v9 = i - ((uint32_t)this->field_4 % i);
        if (v9 < i) {
            this->field_4 += v9;
        }
    }
};

inline void rebase(uint8_t *&ptr, uint32_t i)
{
    uint32_t v9 = i - ((uint32_t)ptr % i);
    if (v9 < i) {
        ptr += v9;
    }
}


//0x004C1FA0
extern void *parse_generic_mash_init(generic_mash_header *&header, void *cur_ptr, bool *allocated_mem,
                                     generic_mash_data_ptrs *a4, uint32_t struct_size, uint32_t *virtual_table_lookup,
                                     uint32_t *size_table_lookup, uint32_t num_table_entries, uint32_t base_class_size,
                                     void *a10);

template <typename T>
bool parse_generic_object_mash(T *&arg0, void *a1, void *a5, uint32_t *a6, uint32_t *a7, uint32_t a8, uint32_t a9,
                               void *a10)
{
    bool allocated_mem = false;

    generic_mash_data_ptrs a4;
    auto *header = static_cast<generic_mash_header *>(a1);
    auto *v4 =
        static_cast<T *>(parse_generic_mash_init(header, a1, &allocated_mem, &a4, sizeof(T), a6, a7, a8, a9, a10));

    arg0 = v4;
    v4->un_mash_start(header, v4, &a4, a5);

    return allocated_mem;
}

//0x005E4020
template <>
bool parse_generic_object_mash(mission_table_container *&a1, void *a2, void *a3, unsigned int *a4, unsigned int *a5,
                               uint32_t a6, uint32_t a7, void *a8);

//0x00563F40
template <>
bool parse_generic_object_mash(resource_directory *&arg0, void *a1, void *a5, uint32_t *a6, uint32_t *a7, uint32_t a8,
                               uint32_t a9, void *a10);
