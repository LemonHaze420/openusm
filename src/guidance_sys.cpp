#include "guidance_sys.h"

#include "common.h"
#include "local_collision.h"
#include "oldmath_po.h"
#include "physical_interface.h"
#include "damage_interface.h"
#include "thrown_item.h"
#include "vtbl.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>

namespace {
constexpr float rand_scale = 1.0f / 32767.0f;
float signed_random() { return std::rand() * rand_scale * 2.0f - 1.0f; }
bool is_hero(entity_base *value)
{
    return reinterpret_cast<bool (__fastcall *)(entity_base *, void *)>(
        get_vfunc(value->m_vtbl, 0x4C))(value, nullptr);
}
vector3d cross(const vector3d &a, const vector3d &b)
{
    return {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
}
void *__fastcall guidance_delete(rocket_guidance_sys *self, void *, unsigned flags)
{
    self->owner = nullptr;
    if (flags & 1)
        delete self;
    return self;
}
int __fastcall guidance_type(rocket_guidance_sys *, void *) { return 1; }
void __fastcall guidance_advance(rocket_guidance_sys *self, void *, Float elapsed) { self->frame_advance(elapsed); }
void __fastcall guidance_launch(rocket_guidance_sys *self, void *, const vector3d &direction, float speed)
{
    self->launch(direction, speed);
}
}

void *rocket_guidance_sys::native_vtable()
{
    static void *table[]{reinterpret_cast<void *>(&guidance_delete), reinterpret_cast<void *>(&guidance_type),
                         reinterpret_cast<void *>(&guidance_advance), reinterpret_cast<void *>(&guidance_launch)};
    return table;
}

rocket_guidance_sys::rocket_guidance_sys(physical_interface *physical)
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
    owner = physical;
    field_18 = 0;
    field_1C = true;
    field_20 = {};
    field_24 = {};
    field_40 = field_44 = field_48 = field_4C = field_50 = field_5C = 0.0f;
    field_64 = field_68 = field_6C = field_70 = field_74 = 0.0f;
    field_A4 = nullptr;
    field_88 = 0.0f;
    field_58 = 0.35f;
    field_60 = 2.0f;
    field_7C = 0.3f;
    field_54 = 1.0f;
    field_80 = 1.0f;
    field_8C = {};
    field_84 = 0.0f;
    field_78 = -1.0f;
    field_98 = {};
}

void rocket_guidance_sys::set_target(vhandle_type<entity> target)
{
    field_20 = target;
    if (auto *value = target.get_volatile_ptr())
        field_24 = value->get_abs_position();
}

void rocket_guidance_sys::launch(const vector3d &direction, float speed)
{
    field_3C = 0.0f;
    field_40 = speed;
    field_4C = signed_random() * field_70 + field_64;
    field_48 = 0.0f;
    field_50 = signed_random() * field_74 + field_68;
    if (field_18 & 4) {
        field_24 = owner->field_4->get_abs_position();
        if (field_8C.length2() <= 0.0f)
            field_8C = {1.0f, 0.0f, 0.0f};
        if (dot(field_8C, direction) > 0.99f)
            field_8C = {0.0f, 1.0f, 0.0f};
        if (dot(field_8C, direction) > 0.99f)
            field_8C = {0.0f, 0.0f, 1.0f};
        po orientation;
        orientation.set_po(field_8C, direction, vector3d{});
        field_8C = orientation.get_z_facing();
    }
    field_84 = 0.0f;
    field_98 = {};
    owner->set_gravity(false);
    field_C = direction;
    field_8 = speed;
    owner->set_velocity({}, false);
    owner->field_2C = {};
    owner->set_acceleration_factor({});
    owner->field_44 = {};
    owner->field_50 = {};
    owner->field_4->exit_limbo();
    owner->set_velocity(direction * speed, false);
}

void rocket_guidance_sys::wobble()
{

    const float z = signed_random(), y = signed_random(), x = signed_random();
    vector3d axis{x, y, z};
    axis.normalize();
    const int sign = static_cast<int>(std::rand() * (2.0f / 32768.0f)) != 0 ? 1 : -1;
    field_98 = axis * (field_58 * 3.141592653589793f * sign);
    field_48 = signed_random() * field_6C + field_60;
}

void rocket_guidance_sys::reacquire_target(float radius, float cosine, float delay, entity_base *exclude)
{
    entity *best = field_20.get_volatile_ptr();
    auto *actor = owner->field_4;
    const auto origin = actor->get_abs_position();
    if (damage_interface::find_damageable(origin, radius, 3, true) > 0) {
        auto facing = actor->get_abs_po().get_z_facing();
        facing.normalize();
        const bool collisions = (actor->field_4 & 0x4000) != 0;
        actor->set_collisions_active(false, false);
        float best_score = -3.402823466e38f;
        for (auto *damage : *damage_interface::found_damageable) {
            auto *candidate = damage->field_4;
            if (!candidate || candidate == exclude || (candidate->field_4 & 0x20000) ||
                (!(candidate->field_4 & 0x200) && !is_hero(candidate)) ||
                (field_A4 && !field_A4->can_damage(candidate, false)))
                continue;
            auto direction = candidate->get_abs_position() - origin;
            const float distance = direction.length();
            if (distance <= 0.0001f)
                continue;
            const float alignment = dot(direction / distance, facing);
            if (alignment < delay)
                continue;
            const float score = alignment > 0.0f ? alignment / distance : alignment * distance;
            if (score <= best_score)
                continue;
            vector3d hit, normal;
            entity *occluder = nullptr;
            if (find_intersection(origin, candidate->get_abs_position(), *local_collision::entfilter_reject_all,
                    *local_collision::obbfilter_lineseg_test, &hit, &normal, nullptr, nullptr, nullptr, false))
                continue;

            if (find_intersection(origin, candidate->get_abs_position(), *local_collision::entfilter_blocks_beams,
                    *local_collision::obbfilter_reject_all, &hit, &normal, nullptr, &occluder, nullptr, false) &&
                occluder != candidate)
                continue;
            best_score = score;
            best = candidate;
        }
        actor->set_collisions_active(collisions, false);
    }
    if (best) {
        field_20.field_0 = best->my_handle.field_0;
        field_4C = delay;
    } else {
        field_4C = signed_random() * 0.25f + 0.65f;
    }
    (void)cosine;
}

void rocket_guidance_sys::frame_advance(Float elapsed)
{
    const float dt = elapsed.value;
    auto *target = field_20.get_volatile_ptr();
    if (field_78 >= 0.0f)
        field_78 = std::max(0.0f, field_78 - dt);
    field_48 = std::max(0.0f, field_48 - dt);
    field_4C = std::max(0.0f, field_4C - dt);
    field_50 = std::max(0.0f, field_50 - dt);
    auto *actor = owner->field_4;
    const auto position = actor->get_abs_position();
    if (field_18 & 4) {
        field_24 += field_C * (field_40 * dt);
        if (field_4C <= 0.0f) {
            field_84 += dt * field_40;
            if (std::fabs(field_88) > 0.001f) {
                po rotation;
                rotation.set_rot(field_C, Float{dt * field_88});
                field_8C = rotation.non_affine_slow_xform(field_8C);
            }
        }
        const float cycle = field_84 / field_80;
        const auto destination = field_24 + field_8C *
            (std::sin((cycle - static_cast<int>(cycle)) * 6.283185307179586f) * field_7C);
        auto direction = destination - position;
        direction.normalize();
        po pose = po_identity_matrix;
        pose.set_facing(direction);
        pose.set_position(destination);
        *actor->my_rel_po = pose;
        actor->dirty_family(false);
        if (actor->field_4 & (0x8000 | 4))
            actor->dirty_model_po_family();
        actor->po_changed();
        owner->set_velocity({}, false);
        if (!(field_78 <= 0.0f && field_78 >= 0.0f) && field_50 <= 0.0f)
            field_40 += dt * field_5C;
    } else if ((field_18 & 1) && (!target || (target->field_4 & 0x20000) ||
               (!(target->field_4 & 0x200) && !is_hero(target)))) {
        target = nullptr;
        if (field_4C <= 0.0f) {
            reacquire_target(35.0f, -1.0f, 0.0f, nullptr);
            target = field_20.get_volatile_ptr();
        }
    }
    auto direction = owner->get_velocity();
    const float speed = direction.length();
    if (speed <= 0.0001f)
        direction = actor->get_abs_po().get_z_facing();
    direction.normalize();
    if ((field_18 & 3) && field_4C <= 0.0f) {
        if (!target && !(field_18 & 2)) {
            if (field_48 <= 0.0f)
                wobble();
        } else {
            if ((field_18 & 1) || ((field_18 & 2) && field_3C <= var<float>(0x00960B5C)))
                field_24 = thrown_item::calc_target_pos(actor->get_abs_position(), field_20,
                    field_40, field_30, field_44);
            auto desired = field_24 - position;
            const float distance = desired.length();
            if (distance > 0.0f) {
                desired /= distance;
                const float alignment = dot(desired, direction);
                if (field_54 < 0.0f) {
                    direction = desired;
                    field_98 = {};
                } else if (field_54 >= 1.0f) {
                    if (alignment < 0.999f && alignment > -0.999f) {
                        auto axis = cross(desired, direction);
                        axis.normalize();
                        po rotation;
                        rotation.set_rot(axis, Float{std::clamp(std::acos(std::clamp(alignment, -1.0f, 1.0f)),
                            -field_58 * 3.141592653589793f * dt, field_58 * 3.141592653589793f * dt)});
                        direction = rotation.non_affine_slow_xform(direction);
                        field_98 = {};
                    }
                } else if (field_54 > 0.0f && alignment <= field_54) {
                    auto axis = cross(desired, direction);
                    axis.normalize();
                    field_98 = axis * (field_58 * 3.141592653589793f);
                } else if (field_48 <= 0.0f) {
                    wobble();
                }
            }
        }
    }
    if (!(field_78 <= 0.0f && field_78 >= 0.0f)) {
        if (!(field_98.length2() <= 0.0f)) {
            po rotation;
            rotation.set_rot(field_98 * dt);
            direction = rotation.non_affine_slow_xform(direction);
        }
        owner->set_velocity(direction * (field_50 > 0.0f ? speed : speed + dt * field_5C), false);
    }
    auto up = actor->get_abs_po().get_y_facing();
    up.normalize();
    if (std::fabs(dot(up, direction)) > 0.99f)
        up = actor->get_abs_po().get_x_facing();
    po pose;
    pose.set_po(up, direction, actor->get_abs_position());
    pose.set_po(pose.get_y_facing(), pose.get_z_facing(), pose.get_position());
    actor->set_abs_po(pose);
    field_3C += dt;
}

void physical_interface::create_guidance_sys(int type)
{
    if (type == 1)
        field_E8 = new rocket_guidance_sys(this);
}

void physical_interface::set_gravity_delay_timer(float timer)
{
    field_A4 = timer;
}

void physical_interface::set_acceleration_factor(const vector3d &acceleration)
{
    field_38 = acceleration;
}

VALIDATE_SIZE(guidance_system, 0x20);
VALIDATE_SIZE(rocket_guidance_sys, 0xA8u);
