#pragma once

#include <cstdint>

#include "utility.h"
#include "mash_virtual_base.h"

struct mash_info_struct {

    uint8_t *mash_image_ptr[1];
    int buffer_size_used[1];
    int m_size;
    int field_C;

    mash_info_struct(uint8_t *a1, int size);

    //0x004208F0
    uint8_t *read_from_buffer(int a2, int a3);

    //template<typename T>
    //T *read_from_buffer(T *a);

    //0x0041F7C0
    int align_buffer(int a2);

    //0x0041F770
    void advance_buffer(int how_many_bytes);

    //0x0041F780
    void deductive_align_buffer();

    template<typename T>
    void unmash_class(T *&a2, [[maybe_unused]] void *a3)
#if 0 
    ;
#else
    {
        if constexpr (std::is_base_of_v<mash_virtual_base, T>)
        {
            a2 = bit_cast<T *>(this->read_from_buffer(sizeof(T), 0));
            mash_virtual_base::fixup_vtable(a2);
            auto v7 = a2->get_mash_sizeof();
            this->advance_buffer(v7 - sizeof(T));
            a2->unmash(this, nullptr);
        }
        else
        {
            T *v6 = bit_cast<T *>(this->read_from_buffer(sizeof(T), std::alignment_of_v<T>));
            a2 = v6;

            a2->unmash(this, a3);
        }
    }
#endif

    template<typename T>
    void unmash_class_in_place(T &a1, void *a2)
    {
        if constexpr (std::is_base_of_v<mash_virtual_base, T>)
        {
            mash_virtual_base::fixup_vtable(&a1);
        }

        a1.unmash(this, a2);
    }

    template<typename T>
    static void construct_class(T *&);
};

extern void mash_info_struct_patch();
