#include "marky_camera.h"

#include "common.h"
#include "oldmath_po.h"
#include "string_hash.h"

VALIDATE_SIZE(marky_camera, 0x1E0u);

marky_camera::marky_camera(const string_hash &a2) : game_camera(a2, nullptr)
{
    static Var<vector3d> stru_960E24{0x00960E24};
    static Var<vector3d> stru_9225D4{0x009225D4};

    this->field_1BD = 0;
    this->field_1A0 = stru_960E24();
    this->field_1AC = stru_9225D4();
    this->field_1BC = 0;
    this->field_1BD = 0;
    this->field_1C0 = {};

    this->field_1B8 = 0;
    this->field_1DC = 0;
    this->field_1D8 = -1001.0;
}

void *marky_camera::operator new(size_t size)
{
    return _aligned_malloc(size, 4);
}

void marky_camera::set_affixed_x_facing(bool a2)
{
    this->field_1BD = a2;
    if (a2) {
        auto &v3 = this->get_rel_po();
        this->field_1C0 = v3.get_x_facing();
    }
}

void marky_camera::sync(camera &a2)
{
    if (!this->is_externally_controlled()) {
        game_camera::sync(a2);
    }
}

void marky_camera::camera_set_collide_with_world(bool a2)
{
    this->field_1BC = a2;
}
