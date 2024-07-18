#include "mash_info_struct.h"

#include "variables.h"
#include "common.h"
#include "func_wrapper.h"
#include "trace.h"

#include <utility.h>

#include <cassert>

VALIDATE_SIZE(mash_info_struct, 0x10);

mash_info_struct::mash_info_struct(uint8_t *a1, int size) {
    this->mash_image_ptr[0] = a1;
    this->buffer_size_used[0] = 0;
    this->m_size = size;
    this->field_C = 0;
}

void mash_info_struct::advance_buffer(int how_many_bytes) {
#if 0
    //this->mode == 1 && this->state != 1
    assert(mode != mash::MASH_MODE || state == INITIALIZED);
#endif

    static constexpr auto buffer = 0;

    assert(mash_image_ptr[buffer] != nullptr);
    assert(how_many_bytes >= 0);

    this->buffer_size_used[buffer] += how_many_bytes;
}

uint8_t *mash_info_struct::read_from_buffer(int size, int align)
{
    //TRACE("mash_info_struct::read_from_buffer");

    if (align != 0)
    {
        this->align_buffer(align);
    }
    else
    {
        this->deductive_align_buffer();
    }

    static constexpr auto buffer = 0;

    int v4 = this->buffer_size_used[buffer];
    auto result = &this->mash_image_ptr[buffer][v4];
    this->buffer_size_used[buffer] += size;

    //sp_log("0x%08X", result);
    return result;
}

int mash_info_struct::align_buffer(int a2)
{
    static constexpr auto buffer = 0;

    auto *buffer_ptr = this->mash_image_ptr[buffer];
    assert(buffer_ptr != nullptr);

    auto *v2 = &buffer_ptr[this->buffer_size_used[buffer]];
    auto *v3 = bit_cast<uint8_t *>(~(a2 - 1) & ((uint32_t) &v2[a2 - 1]));
    if (v3 != v2)
    {
        if (g_is_the_packer) {
            std::memset(v2, 0xA1u, v3 - v2);
        }

        this->buffer_size_used[buffer] += v3 - v2;
    }

    return v3 - v2;
}

void mash_info_struct::deductive_align_buffer() {
    static constexpr auto buffer = 0;
    assert(mash_image_ptr[buffer] != nullptr);

    auto *i = &this->mash_image_ptr[buffer][this->buffer_size_used[buffer]];

    for (; *i == 161; ++i) {
        ;
    }

    auto new_used = i - this->mash_image_ptr[0];
    if (new_used % 4) {
        new_used = 4 * (new_used / 4);
        assert(new_used >= buffer_size_used[buffer]);
    }

    this->buffer_size_used[buffer] = new_used;
}

void mash_info_struct_patch() {
    {
        FUNC_ADDRESS(address, &mash_info_struct::read_from_buffer);
        SET_JUMP(0x004208F0, address);
    }

    {
        FUNC_ADDRESS(address, &mash_info_struct::align_buffer);
        //SET_JUMP(0x0041F7C0, address);
    }

    return;

    {
        FUNC_ADDRESS(address, &mash_info_struct::deductive_align_buffer);
        REDIRECT(0x004208FB, address);
        REDIRECT(0x00420958, address);
    }
}
