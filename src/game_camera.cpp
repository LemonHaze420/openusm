#include "game_camera.h"

#include "collide.h"
#include "common.h"
#include "func_wrapper.h"
#include "local_collision.h"
#include "oldmath_po.h"
#include "osassert.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"
#include "memory.h"
#include <algorithm>
#include <array>
#include <cstring>

#include <cmath>

VALIDATE_SIZE(game_camera::_camera_shake_t, 0x1Cu);
VALIDATE_SIZE(game_camera, 0x1A0u);
VALIDATE_OFFSET(game_camera, field_118, 0x118);
VALIDATE_OFFSET(game_camera, field_12C, 0x12C);
VALIDATE_OFFSET(game_camera, field_130, 0x130);

static Var<vector3d> chest{0x00960E3C};

#if STANDALONE_SYSTEM
namespace {
void __fastcall game_destroy(game_camera *self, void *, bool release)
{
    self->~game_camera();
    if (release)
        mem_dealloc(self, sizeof(game_camera));
}
int __fastcall game_size(game_camera *, void *)
{
    return sizeof(game_camera);
}
int __fastcall game_flavor(game_camera *, void *)
{
    return 20;
}
bool __fastcall game_identity(game_camera *, void *)
{
    return true;
}
void __fastcall game_advance(game_camera *self, void *, Float dt)
{
    self->frame_advance(dt);
}
void __fastcall game_sync(game_camera *self, void *, camera *source)
{
    self->_sync(*source);
}
vector3d *__fastcall game_shake(game_camera *self, void *, vector3d *out, vector3d pos, Float dt)
{
    *out = self->frame_advance_shake(pos, dt);
    return out;
}
void __fastcall game_target(game_camera *self, void *, entity *target)
{
    self->set_target_entity(target);
}
void __fastcall game_target_handle(game_camera *self, void *, vhandle_type<entity> target)
{
    self->field_118 = target;
}
bool __fastcall game_shaking(game_camera *self, void *, short handle)
{
    return self->is_shake_active(handle);
}
short __fastcall game_add_shake(game_camera *self, void *, float a, float f, float d, float fi, float fo)
{
    return self->add_shake(a, f, d, fi, fo);
}
void __fastcall game_remove_shake(game_camera *self, void *, short handle)
{
    self->remove_shake(handle);
}
void __fastcall game_clear_shakes(game_camera *self, void *)
{
    self->clear_shakes();
}
}  // namespace
#endif

void *game_camera::native_vtable()
{
#if STANDALONE_SYSTEM
    static auto table = [] {
        std::array<void *, 192> result;
        std::copy_n(static_cast<void **>(camera::native_vtable()), result.size(), result.data());
        result[0] = reinterpret_cast<void *>(game_destroy);
        result[1] = reinterpret_cast<void *>(game_size);
        result[0x54 / 4] = reinterpret_cast<void *>(game_flavor);
        result[0x74 / 4] = reinterpret_cast<void *>(game_identity);
        result[0x1A4 / 4] = reinterpret_cast<void *>(game_advance);
        result[0x294 / 4] = reinterpret_cast<void *>(game_sync);
        result[0x2B0 / 4] = reinterpret_cast<void *>(game_shake);
        result[0x2B4 / 4] = reinterpret_cast<void *>(game_target);
        result[0x2B8 / 4] = reinterpret_cast<void *>(game_target_handle);
        result[0x2BC / 4] = reinterpret_cast<void *>(game_shaking);
        result[0x2C0 / 4] = reinterpret_cast<void *>(game_add_shake);
        result[0x2C4 / 4] = reinterpret_cast<void *>(game_remove_shake);
        result[0x2C8 / 4] = reinterpret_cast<void *>(game_clear_shakes);
        result[0x2CC / 4] = reinterpret_cast<void *>(game_clear_shakes);
        return result;
    }();
    return table.data();
#else
    return reinterpret_cast<void *>(0x00881B50);
#endif
}

void game_camera::_camera_shake_t::clear()
{
    this->field_1A = 0;
    this->field_18 = -1;
    this->field_0 = 0;
    this->field_4 = 0;
    this->field_8 = 0;
    this->field_C = 0;
}

game_camera::_camera_shake_t::_camera_shake_t()
{
    this->field_1A = 0;
    this->clear();
}

game_camera::game_camera(const string_hash &a2, entity *a3) : camera(nullptr, a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        m_vtbl = reinterpret_cast<int>(native_vtable());
        std::fill_n(field_D0, 16, 0.0f);
        field_D0[0] = field_D0[5] = field_D0[10] = field_D0[15] = 1.0f;
        reinterpret_cast<unsigned char *>(empty)[8] = 0;
        field_12C = false;
        set_target_entity(a3);
        clear_shakes();
    }
}

void game_camera::clear_shakes()
{
    for (unsigned i = 0; i < CAMERA_SHAKES_TOTAL; ++i) {
        field_130[i].clear();
        field_130[i].empty[0] = 0.0f;
        field_130[i].empty[1] = 0.0f;
        field_130[i].field_18 = static_cast<short>(8 + i);
    }
}

vector3d game_camera::frame_advance_shake(vector3d position, Float dt)
{
    for (auto &shake : field_130) {
        if (!shake.field_1A)
            continue;
        float amplitude = std::sin(shake.field_C / (1.0f / shake.field_4) * 6.283185307179586f) * shake.field_0;
        if (shake.empty[0] > EPSILON && shake.field_C < shake.empty[0]) {
            amplitude *= std::clamp(shake.field_C / shake.empty[0], 0.0f, 1.0f);
        } else if (shake.empty[1] > EPSILON && shake.field_8 + shake.empty[0] < shake.field_C) {
            amplitude *=
                std::clamp(1.0f - (shake.field_C - shake.field_8 - shake.empty[0]) / shake.empty[1], 0.0f, 1.0f);
        }
        position.y += amplitude;
        shake.field_C += dt;
        if (shake.field_C >= shake.field_8 + shake.empty[0] + shake.empty[1]) {
            shake.field_18 += 8;
            shake.field_1A = false;
        }
    }
    return position;
}

short game_camera::add_shake(float amplitude, float frequency, float duration, float fade_in, float fade_out)
{
    _camera_shake_t *selected = nullptr;
    for (auto &shake : field_130) {
        if (!shake.field_1A) {
            selected = &shake;
            break;
        }
    }
    if (!selected) {
        for (auto &shake : field_130)
            if (shake.field_C / (shake.field_8 + shake.empty[0] + shake.empty[1]) > -1.0e20f)
                selected = &shake;
        selected->field_18 += 8;
    }
    selected->field_1A = true;
    selected->field_0 = amplitude;
    selected->field_4 = frequency;
    selected->field_8 = duration;
    selected->field_C = 0.0f;
    selected->empty[0] = fade_in;
    selected->empty[1] = fade_out;
    return selected->field_18;
}

void game_camera::remove_shake(short handle)
{
    auto &shake = field_130[handle & 7];
    if (shake.field_18 == handle && shake.field_1A) {
        shake.field_18 += 8;
        shake.field_1A = false;
    }
}

bool game_camera::is_shake_active(short handle) const
{
    const auto &shake = field_130[handle & 7];
    return shake.field_18 == handle && shake.field_1A;
}

//FIXME
entity *game_camera::get_target_entity() const
{
    TRACE("game_camera::get_target_entity");

    if constexpr (STANDALONE_SYSTEM) {
        return this->field_118.get_volatile_ptr();
    } else {
        return (entity *)THISCALL(0x0057A220, this);
    }
}

void game_camera::set_target_entity(entity *e)
{
    TRACE("game_camera::set_target_entity");

    if (e != nullptr) {
        this->field_118 = {e->get_my_handle()};
    } else {
        this->field_118 = {0};
    }
}

void game_camera::frame_advance(Float t)
{
    assert(t >= 0 && t < 1e9f);

    if (this->is_externally_controlled()) {
        this->field_12C = false;
    } else {
        auto *target_entity = this->get_target_entity();
        assert(target_entity != nullptr);

        this->field_11C = target_entity->get_abs_position();
        vector3d v26 = target_entity->get_abs_po().get_z_facing();

        auto v31 = this->get_abs_po();
        this->field_128 = this->field_11C[1];
        vector3d v27 = this->field_11C;

        if (!this->field_11C.is_valid()) {
            warning("camera target went out of the world!!");
            this->field_11C = target_entity->get_abs_position();
        }

        bool v23 = target_entity->is_in_limbo();

        v27 += v26;

        vector3d v24;
        if (v23) {
            v24 = v31.get_position();
        } else {
            v24 = this->field_11C;
            v24[1] = this->field_128 + 0.69999999;

            auto &abs_po = target_entity->get_abs_po();
            auto v16 = abs_po.get_z_facing();
            v24 = v24 - (v16 * (3.0 - 1.0));
        }

        vector3d a2 = this->field_11C;
        a2[1] += s_camera_target_radius_factor;

        vector3d a5{};
        vector3d a6{};
        if (!v23 && find_intersection(this->field_11C,
                                      a2,
                                      *local_collision::entfilter_line_segment_camera_collision,
                                      *local_collision::obbfilter_lineseg_test,
                                      &a5,
                                      &a6,
                                      nullptr,
                                      nullptr,
                                      nullptr,
                                      false)) {
            a2 = a5;
            a2.y = a5.y - 0.30000001f;
            if (v27[1] > a2[1]) {
                v27[1] = a2[1];
            }

            if (v24[1] > a2[1]) {
                v24[1] = a2[1];
            }
        }

        chest() = this->field_11C;
        chest().y += 0.69999999f;
        if (chest().y > a2.y) {
            chest().y = a2.y;
        }

        if (!v23 && find_intersection(this->field_11C,
                                      v24,
                                      *local_collision::entfilter_line_segment_camera_collision,
                                      *local_collision::obbfilter_lineseg_test,
                                      &a5,
                                      &a6,
                                      nullptr,
                                      nullptr,
                                      nullptr,
                                      false)) {
            v24 = a6 * 0.30000001 + a5;
            if (a6.y > -0.5f) {
                auto v21 = (v24 - v27).length2();
                if (v21 > EPSILON) {
                    v21 = std::sqrt(v21);
                    v24.y += s_camera_target_radius_factor - v21;
                }
            }
        }

        if (v24.y > a2.y) {
            v24.y = a2.y;
        }

        if (!v23) {
            vector3d v26{0, 0, 0};
            auto *reg = this->get_primary_region();
            v24 = collide_with_world(this, v26, 0.30000001f, v24, reg);
        }

        assert(t >= 0 && t < 1e2f);

        this->blend(v27, v24, t);
    }
}

void game_camera::_sync(camera &a2)
{
    if (!this->is_externally_controlled()) {
#if STANDALONE_SYSTEM
        camera::_sync(a2);
#else
        camera::sync(a2);
#endif
        this->field_12C = false;
    }
}

void game_camera::blend(vector3d arg0, vector3d eax0, Float arg18)
{
#if STANDALONE_SYSTEM
    const auto desired = eax0;
    if (field_12C) {
        const auto current = get_abs_position();
        const auto relative = current - field_11C;
        const auto wanted = desired - field_11C;
        const float radius = std::sqrt(relative.x * relative.x + relative.z * relative.z);
        const float angle = std::atan2(-relative.x, relative.z);
        const float radius_delta = std::sqrt(wanted.x * wanted.x + wanted.z * wanted.z) - radius;
        float angle_delta = std::atan2(-wanted.x, wanted.z) - angle;
        if (angle_delta < -3.141592653589793f)
            angle_delta += 6.283185307179586f;
        else if (angle_delta > 3.141592653589793f)
            angle_delta -= 6.283185307179586f;
        const float dead_zone = arg18 * 0.001f;
        const float new_radius = radius + (std::fabs(radius_delta) <= dead_zone ? 0.0f : radius_delta * arg18 * 6.0f);
        const float new_angle =
            angle + (std::fabs(angle_delta * radius) <= dead_zone ? 0.0f : angle_delta * arg18 * 3.0f);
        eax0 = {field_11C.x - std::sin(new_angle) * new_radius,
                current.y + 2.0f * (desired.y - current.y) * arg18,
                field_11C.z + std::cos(new_angle) * new_radius};
        const auto movement = eax0 - current;
        const float distance = movement.length();
        if (distance > arg18 * 10.0f)
            eax0 = current + movement * (arg18 * 10.0f / distance);
        vector3d hit, normal;
        if (find_intersection(chest(),
                              eax0,
                              *local_collision::entfilter_line_segment_camera_collision,
                              *local_collision::obbfilter_lineseg_test,
                              &hit,
                              &normal,
                              nullptr,
                              nullptr,
                              nullptr,
                              false) &&
            (chest() - hit).length2() > 1.5625f)
            eax0 = desired;
    }
    eax0 = collide_with_world(this, eax0 - get_abs_position(), 0.3f, eax0, get_primary_region());
    if (!eax0.is_valid())
        eax0 = get_target_entity()->get_abs_position();
    set_abs_position(eax0);
    auto forward = arg0 - get_abs_position();
    const float distance = forward.length();
    forward = distance <= 0.001f ? ZVEC : forward / distance;
    auto right = vector3d{forward.z, 0.0f, -forward.x};
    const float width = right.length();
    right = width <= 0.00001f ? XVEC : right / width;
    const auto up = vector3d::cross(forward, right);
    po facing;
    facing.set_po(right, up, forward, ZEROVEC);
    if (m_parent) {
        po relative;
        po::full_inv_multiply(relative, m_parent->get_abs_po(), facing);
        facing = relative;
    }
    facing.sub_48D840();
    facing.set_position(get_rel_position());
    set_abs_po(facing);
    field_12C = true;
    std::memcpy(field_D0, &get_abs_po(), sizeof(field_D0));
#else
    THISCALL(0x0057A330, this, arg0, eax0, arg18);
#endif
}

void game_camera_patch()
{
    {
        FUNC_ADDRESS(address, &game_camera::frame_advance);
        set_vfunc(0x00881CF4, address);
    }

    {
        FUNC_ADDRESS(address, &game_camera::set_target_entity);
        SET_JUMP(0x0057CC50, address);
    }
}
