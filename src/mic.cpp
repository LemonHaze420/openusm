#include "mic.h"

#include "common.h"
#include "func_wrapper.h"
#include "memory.h"

VALIDATE_SIZE(mic, 0x80);

mic::mic(entity *a2, const string_hash &a3) : entity(a3, 0)
{
    this->m_vtbl = 0x00888258;

    if (a2 != nullptr) {
        this->set_parent(a2);
    }

    this->field_68 = ZEROVEC;
    this->field_74 = ZEROVEC;
}

void *mic::operator new(size_t size)
{
    return mem_alloc(size);
}

void mic::frame_advance(Float a2)
{
    if constexpr (0) {
    } else {
        THISCALL(0x0051D9A0, this, a2);
    }
}
