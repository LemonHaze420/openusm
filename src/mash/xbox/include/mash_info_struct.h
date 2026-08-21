#pragma once

#include <cstdint>

#include "utility.h"
#include "mash_virtual_base.h"

namespace mash {
enum mode_t {
    MASH_MODE = 1,
    UNMASH_MODE = 2,
    ALIGN_MODE = 3,
};

enum buffer_type {
    NORMAL_BUFFER = 0,
    SHARED_BUFFER = 1,
};
}  // namespace mash

struct mash_header {
    int field_0;
    int field_4;
    int field_8;
    bool field_C;
};

struct mash_info_struct {
    mash::mode_t mode;
    enum {
        INITIALIZED = 1,
    } state;
    mash_header *header;
    uint8_t *mash_image_ptr[2];
    int buffer_size_used[2];
    int m_index;
    int m_size;
    int field_C;

    mash_info_struct(mash::mode_t a2, uint8_t *a3, int size, bool a5);

    int get_header_size();

    void advance_buffer(mash::buffer_type, int);

    uint8_t *read_from_buffer(mash::buffer_type, int a2, int a3);

    void align_buffer(mash::buffer_type, int a2);

    void deductive_align_buffer(mash::buffer_type);

    template <typename T>
    void unmash_class(T *&a2, void *a3, mash::buffer_type buffer)
#if 0 
    ;
#else
    {
        if constexpr (std::is_base_of_v<mash_virtual_base, T>) {
            a2 = bit_cast<T *>(this->read_from_buffer(buffer, sizeof(T), 0));
            mash_virtual_base::fixup_vtable(a2);
            auto v7 = a2->get_mash_sizeof();
            this->advance_buffer(buffer, v7 - sizeof(T));
            a2->unmash(this, nullptr);
        } else {
            T *v6 = bit_cast<T *>(this->read_from_buffer(buffer, sizeof(T), std::alignment_of_v<T>));
            a2 = v6;

            a2->unmash(this, a3);
        }
    }
#endif

    template <typename T>
    void unmash_class_in_place(T &a1, void *a2)
    {
        if constexpr (std::is_base_of_v<mash_virtual_base, T>) {
            mash_virtual_base::fixup_vtable(&a1);
        }

        a1.unmash(this, a2);
    }

    template <typename T>
    static void construct_class(T *&a1);
};

extern void mash_info_struct_patch();
