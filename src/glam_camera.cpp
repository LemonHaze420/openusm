#include "glam_camera.h"

#include "common.h"
#include "conglom.h"
#include "oldmath_po.h"
#include "resource_manager.h"
#include "wds.h"
#include "memory.h"
#include <algorithm>
#include <array>

VALIDATE_SIZE(glam_camera, 0x1BC);
VALIDATE_OFFSET(glam_camera, elevation, 0x1A0);

#if STANDALONE_SYSTEM
namespace {
void __fastcall glam_destroy(glam_camera *self, void *, bool release)
{
    self->~glam_camera();
    if (release)
        mem_dealloc(self, sizeof(glam_camera));
}
int __fastcall glam_size(glam_camera *, void *)
{
    return sizeof(glam_camera);
}
void __fastcall glam_advance(glam_camera *self, void *, Float dt)
{
    self->frame_advance(dt);
}
}

void *glam_camera::native_vtable()
{
    static auto table = [] {
        std::array<void *, 192> result;
        std::copy_n(static_cast<void **>(game_camera::native_vtable()), result.size(), result.data());
        result[0] = reinterpret_cast<void *>(glam_destroy);
        result[1] = reinterpret_cast<void *>(glam_size);
        result[0x1A4 / 4] = reinterpret_cast<void *>(glam_advance);
        return result;
    }();
    return table.data();
}
#endif


glam_camera::glam_camera(const char *name)
    : game_camera(string_hash{name}, g_world_ptr->get_hero_ptr(0)), elevation(0.0f), azimuth(0.0f), distance(3.0f),
      fov(0.52359879f), camera_bone("bip01_head"), target_bone("bip01_head"), target(0)
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<int>(native_vtable());
#endif
    m_resource_context = resource_manager::get_best_context(static_cast<resource_partition_enum>(0));
}


void glam_camera::frame_advance(Float)
{
    if (anim_ctrl)
        return;
    auto *tracked = target.get_volatile_ptr();
    if (!tracked)
        return;
    if (target_bone != string_hash{})
        tracked = static_cast<conglomerate *>(tracked)->get_bone(target_bone, 1);
    const auto &pose = tracked->get_abs_po();
    const vector3d position = pose.get_position();
    const vector3d right{pose.m[0][0], pose.m[0][1], pose.m[0][2]};
    const vector3d up{pose.m[1][0], pose.m[1][1], pose.m[1][2]};
    const vector3d backwards{-pose.m[2][0], -pose.m[2][1], -pose.m[2][2]};
    matrix4x4 pitch;
    pitch.make_rotate(right, elevation);
    matrix4x4 yaw;
    yaw.make_rotate(up, azimuth);
    const auto direction = yaw * (pitch * backwards);
    set_abs_position(position + direction * distance);
    look_at(position);
    set_fov(fov);
}
