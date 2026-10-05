#include "als_motion_compensator.h"

#include "actor.h"
#include "als_animation_logic_system.h"
#include "common.h"
#include "custom_math.h"
#include "oldmath_po.h"
#include "trace.h"
#include "physical_interface.h"
#include "memory.h"
#include <array>
#include <functional>
#include "utility.h"
#include "vtbl.h"

namespace als {

VALIDATE_SIZE(motion_compensator, 0x14);

void motion_compensator::finalize(bool a1)
{
    void(__fastcall * func)(void *, void *edx, bool) = CAST(func, get_vfunc(m_vtbl, 0x8));
    func(this, nullptr, a1);
}

void motion_compensator::activate(animation_logic_system *a2)
{
    this->field_4 = a2;
    this->field_8 = (als::state_machine *)this->field_4->get_als_layer_internal(static_cast<als::layer_types>(0));
    this->the_actor = this->field_4->get_actor();
    this->field_10 = 10.0;
}

void motion_compensator::deactivate()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x1C));
    func(this);
}

void motion_compensator::pre_anim_action(Float a3)
{
    void(__fastcall * func)(void *, void *, Float) = CAST(func, get_vfunc(m_vtbl, 0x20));
    func(this, nullptr, a3);
}

void motion_compensator::post_anim_action(Float a2)
{
    void(__fastcall * func)(void *, void *, Float) = CAST(func, get_vfunc(m_vtbl, 0x24));
    func(this, nullptr, a2);
}

void motion_compensator::set_facing_to_dir_internal(actor *explicit_actor, vector3d current, vector3d desired,
                                                    vector3d up, Float turn_rate, Float threshold, Float dt)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x00499E80, this, explicit_actor, current, desired, up, turn_rate, threshold, dt);
    } else {
        if (desired.length2() < EPSILON)
            return;
        desired.normalize();
        if (current.length2() >= EPSILON)
            current.normalize();
        else
            current = desired;
        auto *owner = field_4->get_actor();
        const auto &transform = owner->get_abs_po();
        const float cosine = std::clamp(dot(current, desired), -1.0f, 1.0f);
        if (cosine <= threshold || dot(up, transform.get_y_facing()) < 1.0f) {
            float angle = bounded_acos(cosine);
            if (dot(desired, owner->get_abs_po().get_x_facing()) > 0.0f)
                angle = -angle;
            angle *= std::min(turn_rate * dt, 1.0f);
            po rotation{};
            rotation.set_rot(up, angle);
            const auto facing = rotation.non_affine_slow_xform(current);
            po result{};
            result.set_po(facing, up, owner->get_abs_position());
            entity_set_abs_po(owner, result);
        }
    }
}

void motion_compensator::set_anim_playback_speed(Float new_anim_speed)
{
    TRACE("motion_compensator::set_anim_playback_speed");

    assert(new_anim_speed < 10.0f && "Unreasonable anim speed shift!");

    auto *the_machine = this->field_4->get_als_layer_internal(static_cast<layer_types>(0));
    auto the_handle = the_machine->get_anim_handle();
    the_handle.set_anim_speed(new_anim_speed);
}

double motion_compensator::get_anim_movement_scale_param()
{
    TRACE("als::motion_compensator::get_anim_movement_scale_param");

    if (this->field_8->has_ext_param_been_set(0xFu)) {
        return this->field_8->get_param(this->field_4, 0xFu);
    } else {
        return 1.0;
    }
}

double motion_compensator::get_anim_playback_speed_param()
{
    return field_8->has_ext_param_been_set(0x10u) ? field_8->get_param(field_4, 0x10u) : 1.0;
}

namespace {
void __fastcall mocomp_empty(motion_compensator *, void *) {}
void __fastcall mocomp_unmash(motion_compensator *, void *, mash_info_struct *, void *) {}
motion_compensator *__fastcall mocomp_finalize(motion_compensator *self, void *, unsigned flags)
{
    self->~motion_compensator();
    if (flags & 1)
        mem_dealloc(self, sizeof(motion_compensator));
    return self;
}
int __fastcall mocomp_base_type(motion_compensator *, void *)
{
    return 490;
}
int __fastcall mocomp_null_type(motion_compensator *, void *)
{
    return 514;
}
bool __fastcall mocomp_parent_type(motion_compensator *, void *, int type)
{
    return type == 490 || type == 573;
}
bool __fastcall mocomp_is_type(motion_compensator *self, void *, int type)
{
    using type_fn = int(__fastcall *)(motion_compensator *, void *);
    using parent_fn = bool(__fastcall *)(motion_compensator *, void *, int);
    return reinterpret_cast<type_fn>(get_vfunc(self->m_vtbl, 0xC))(self, nullptr) == type ||
           reinterpret_cast<parent_fn>(get_vfunc(self->m_vtbl, 0x10))(self, nullptr, type);
}
void __fastcall mocomp_activate(motion_compensator *self, void *, animation_logic_system *system)
{
    self->activate(system);
}
void __fastcall mocomp_frame_empty(motion_compensator *, void *, Float) {}
void __fastcall mocomp_post(motion_compensator *self, void *, Float)
{
    using get_fn = double(__fastcall *)(motion_compensator *, void *);
    using set_fn = void(__fastcall *)(motion_compensator *, void *, Float);
    const Float speed = reinterpret_cast<get_fn>(get_vfunc(self->m_vtbl, 0x48))(self, nullptr);
    reinterpret_cast<set_fn>(get_vfunc(self->m_vtbl, 0x40))(self, nullptr, speed);
}
bool __fastcall mocomp_fulfilled(motion_compensator *, void *)
{
    return true;
}
void __fastcall mocomp_po(motion_compensator *self, void *, actor *owner, po *offset)
{
    using scale_fn = double(__fastcall *)(motion_compensator *, void *);
    const float scale = reinterpret_cast<scale_fn>(get_vfunc(self->m_vtbl, 0x44))(self, nullptr);
    offset->m[3].x *= scale;
    offset->m[3].y *= scale;
    offset->m[3].z *= scale;
    offset->set_from_ptr_to_po_world(ptr_to_po{&offset->m, &owner->get_abs_po().m});
    offset->sub_48D840();
    entity_set_abs_po(owner, *offset);
}
bool __fastcall mocomp_position(motion_compensator *self, void *, vector3d *position)
{
    bool grounded = false;
    auto *physical = self->the_actor->physical_ifc();
    if (physical != nullptr && std::not_equal_to<float>{}(physical->ground_elevation, -10000.0f)) {
        const float floor = physical->ground_elevation + physical->get_floor_offset();
        if (position->y - floor < LARGE_EPSILON) {
            position->y = floor;
            grounded = true;
        }
    }
    entity_set_abs_position(self->the_actor, *position);
    return grounded;
}
void __fastcall mocomp_facing(motion_compensator *self, void *, actor *owner, vector3d current, vector3d desired,
                              vector3d up, Float rate, Float threshold, Float dt)
{
    self->set_facing_to_dir_internal(owner, current, desired, up, rate, threshold, dt);
}
void dispatch_facing(motion_compensator *self, actor *owner, vector3d current, vector3d desired, vector3d up,
                     Float rate, Float threshold, Float dt)
{
    using face_fn =
        void(__fastcall *)(motion_compensator *, void *, actor *, vector3d, vector3d, vector3d, Float, Float, Float);
    reinterpret_cast<face_fn>(get_vfunc(self->m_vtbl, 0x3C))(
        self, nullptr, owner, current, desired, up, rate, threshold, dt);
}
void __fastcall mocomp_facing_2d(motion_compensator *self, void *, actor *owner, vector3d current, vector3d desired,
                                 Float rate, Float threshold, Float dt)
{
    current.y = 0.0f;
    desired.y = 0.0f;
    dispatch_facing(self, owner, current, desired, YVEC, rate, threshold, dt);
}
void __fastcall mocomp_facing_3d(motion_compensator *self, void *, actor *owner, vector3d current, vector3d desired,
                                 vector3d up, Float rate, Float threshold, Float dt)
{
    current = orthogonal_projection_onto_plane(current, up);
    desired = orthogonal_projection_onto_plane(desired, up);
    dispatch_facing(self, owner, current, desired, up, rate, threshold, dt);
}
void __fastcall mocomp_speed(motion_compensator *self, void *, Float speed)
{
    self->set_anim_playback_speed(speed);
}
double __fastcall mocomp_scale(motion_compensator *self, void *)
{
    return self->get_anim_movement_scale_param();
}
double __fastcall mocomp_speed_param(motion_compensator *self, void *)
{
    return self->get_anim_playback_speed_param();
}
int __fastcall mocomp_size(motion_compensator *, void *)
{
    return sizeof(motion_compensator);
}
}  // namespace

void *motion_compensator::native_vtable(uint32_t type)
{
    static std::array<void *, 20> base_table{
        reinterpret_cast<void *>(mocomp_empty),       reinterpret_cast<void *>(mocomp_unmash),
        reinterpret_cast<void *>(mocomp_finalize),    reinterpret_cast<void *>(mocomp_base_type),
        reinterpret_cast<void *>(mocomp_parent_type), reinterpret_cast<void *>(mocomp_is_type),
        reinterpret_cast<void *>(mocomp_activate),    reinterpret_cast<void *>(mocomp_empty),
        reinterpret_cast<void *>(mocomp_frame_empty), reinterpret_cast<void *>(mocomp_post),
        reinterpret_cast<void *>(mocomp_fulfilled),   reinterpret_cast<void *>(mocomp_po),
        reinterpret_cast<void *>(mocomp_position),    reinterpret_cast<void *>(mocomp_facing_2d),
        reinterpret_cast<void *>(mocomp_facing_3d),   reinterpret_cast<void *>(mocomp_facing),
        reinterpret_cast<void *>(mocomp_speed),       reinterpret_cast<void *>(mocomp_scale),
        reinterpret_cast<void *>(mocomp_speed_param), reinterpret_cast<void *>(mocomp_size)};
    static auto null_table = [] {
        auto table = base_table;
        table[3] = reinterpret_cast<void *>(mocomp_null_type);
        return table;
    }();
    return type == 514 ? null_table.data() : base_table.data();
}

}  // namespace als

void als_motion_compensator_patch()
{
    FUNC_ADDRESS(address, &als::motion_compensator::set_facing_to_dir_internal);
    //set_vfunc(0x0087860C, address);

    {
        FUNC_ADDRESS(address, &als::motion_compensator::set_anim_playback_speed);
        SET_JUMP(0x004A0FC0, address);
    }

    {
        FUNC_ADDRESS(address, &als::motion_compensator::get_anim_movement_scale_param);
        //SET_JUMP(0x004A0F60, address);
    }
}
