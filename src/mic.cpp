#include "mic.h"

#include "common.h"
#include "func_wrapper.h"
#include "memory.h"
#include "camera.h"
#include "game.h"
#include "os_developer_options.h"
#include "sound_manager.h"
#include "wds.h"
#include "physical_interface.h"
#include "entity_mash.h"
#include "oldmath_po.h"
#include <algorithm>
#include <cmath>

VALIDATE_SIZE(mic, 0x80);

#if STANDALONE_SYSTEM
namespace {
void __fastcall native_mic_destroy(mic *self, void *, bool free_memory)
{
    self->~mic();
    if (free_memory)
        mem_dealloc(self, sizeof(mic));
}

int __fastcall native_mic_size(mic *, void *)
{
    return sizeof(mic);
}

int __fastcall native_mic_flavor(mic *, void *)
{
    return 8;
}

bool __fastcall native_mic_identity(mic *, void *)
{
    return true;
}

void __fastcall native_mic_advance(mic *self, void *, Float dt)
{
    self->frame_advance(dt);
}

void *native_mic_vtable()
{
    construct_v_table_lookup();
    static void *table[192];
    static const bool initialized = [] {
        const auto *base = reinterpret_cast<void *const *>(ent_v_table_lookup[2]);
        std::copy_n(base, 192, table);

        table[0] = reinterpret_cast<void *>(native_mic_destroy);
        table[0x4 / 4] = reinterpret_cast<void *>(native_mic_size);
        table[0x54 / 4] = reinterpret_cast<void *>(native_mic_flavor);
        table[0xD0 / 4] = reinterpret_cast<void *>(native_mic_identity);
        table[0x1A4 / 4] = reinterpret_cast<void *>(native_mic_advance);
        return true;
    }();
    (void)initialized;
    return table;
}
}  // namespace
#endif

mic::mic(entity *a2, const string_hash &a3) : entity(a3, 0)
{
#if STANDALONE_SYSTEM
    this->m_vtbl = reinterpret_cast<int>(native_mic_vtable());
#else
    this->m_vtbl = 0x00888258;
#endif

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

void mic::operator delete(void *ptr, size_t size)
{
    mem_dealloc(ptr, size);
}

#if STANDALONE_SYSTEM
namespace {

float listener_asin(float value)
{
    if (value < 0.5f) {
        const float square = value * value;
        return value + value * square * 0.16666670143604279f + value * square * square * 0.07500000298023224f +
               value * square * square * square * 0.0539812408387661f;
    }
    const float falloff = std::sqrt(std::fabs((1.0f - value) * 0.5f));
    const float square = falloff * falloff;
    return 1.570796012878418f - 2.0f * falloff - falloff * square * 0.3333333134651184f -
           falloff * square * square * 0.15000000596046448f +
           falloff * square * square * square * -0.10796249657869339f;
}

vector3d advance_listener_velocity(const vector3d &start, const vector3d &target, float dt)
{
    if (dt >= 0.3f || is_colinear(target, start, 0.0001f))
        return target;
    vector3d axis = vector3d::cross(target, start);
    axis.normalize();
    if (axis.length2() < LARGE_EPSILON)
        axis = YVEC;
    vector3d from = start;
    vector3d to = target;
    from.normalize();
    to.normalize();
    const float cosine = dot(from, to);
    float angle = listener_asin(std::fabs(cosine));
    if (cosine < 0.0f)
        angle = -angle;
    const float phase = dt / 0.3f;
    po rotation = po_identity_matrix;
    rotation.set_rot(axis, (1.5707963705062866f - angle) * phase);
    vector3d result = rotation.slow_xform(start);
    result.set_length(start.length() + (target.length() - start.length()) * phase);
    return result;
}
}  // namespace
#endif

void mic::frame_advance(Float time_inc)
{
#if STANDALONE_SYSTEM
    if (os_developer_options::instance->get_int(static_cast<os_developer_options::ints_t>(2)) != 0)
        return;
    const po &transform = this->get_abs_po();
    const vector3d position = transform.get_position();
    auto *view = g_game_ptr->get_current_view_camera(0);
    const vector3d listener_position =
        view == reinterpret_cast<camera *>(g_world_ptr->get_chase_cam_ptr(0)) ? position : view->get_abs_position();
    sound_manager::set_listener_position(listener_position);
    sound_manager::set_listener_orientation(transform.get_z_facing(), transform.get_y_facing());
    auto *parent = m_parent != nullptr && m_parent->is_an_actor() ? static_cast<actor *>(m_parent) : nullptr;
    vector3d velocity;
    if (parent != nullptr && parent->has_physical_ifc() && !parent->physical_ifc()->is_effectively_standing()) {
        velocity = parent->physical_ifc()->get_velocity();
    } else {
        velocity = time_inc > 0.0f && parent->get_abs_position().is_valid()
                       ? (parent->get_abs_position() - field_68) / time_inc
                       : ZEROVEC;
    }
    field_68 = position.is_valid() ? position : ZEROVEC;
    field_74 = advance_listener_velocity(field_74, velocity, time_inc);
    sound_manager::set_listener_velocity(field_74);
    if (!field_74.is_valid())
        field_74 = ZEROVEC;
#else
    THISCALL(0x0051D9A0, this, time_inc);
#endif
}
