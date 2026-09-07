#include "scene_brew.h"

#include "common.h"
#include "func_wrapper.h"

#include <cstdint>
#include <cstring>

VALIDATE_SIZE(scene_brew, 0xD4);

scene_brew::scene_brew(const resource_key &a2, limited_timer *a3)
{
    std::memset(static_cast<void *>(this), 0, sizeof(*this));
    field_8 = a2;
    field_0.field_4 = a3;
    field_10.field_4 = a3;
    field_50.field_4 = reinterpret_cast<uintptr_t>(a3);
    field_50.field_1C.field_4 = reinterpret_cast<uintptr_t>(a3);
    field_50.field_1C.field_10 = reinterpret_cast<uintptr_t>(a3);
    field_B4.field_4 = a3;
    field_BC.field_4 = a3;
    field_C4.field_4 = a3;
    field_CC.field_4 = a3;
}
