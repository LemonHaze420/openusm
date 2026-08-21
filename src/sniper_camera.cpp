#include "sniper_camera.h"

#include "common.h"

VALIDATE_SIZE(sniper_camera, 0x1B8u);

sniper_camera::sniper_camera(const string_hash &a2, entity *a3) : game_camera(a2, a3)
{
    this->field_1A0 = 0;
    this->field_1A4 = 0;
    this->field_1B0 = 0;
    this->field_1B4 = 0;

    this->field_1AC = 1.0;
}

void *sniper_camera::operator new(size_t size)
{
    return _aligned_malloc(size, 4);
}
