#include "vehicle_sounds.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(vehicle_sounds, 0x50);

vehicle_sounds::vehicle_sounds() {}

void vehicle_sounds::manage_engine_sounds(Float a2, bool a3)
{
    if constexpr (0) {
    } else {
        THISCALL(0x006BB690, this, a2, a3);
    }
}
