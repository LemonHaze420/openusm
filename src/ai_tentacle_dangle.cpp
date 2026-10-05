#include "ai_tentacle_dangle.h"

#include "common.h"
#include "func_wrapper.h"
#include "variable.h"
#include "ai_tentacle_info.h"
#include "base_ai_core.h"
#include "dangler.h"
#include "entity_base.h"
#include "polytube.h"
#include "oldmath_usefulmath.h"
#include "vtbl.h"

VALIDATE_SIZE(ai_tentacle_dangle, 0x24);

int &ai_tentacle_engine::id_counter = var<int>(0x0095807C);

namespace {
void destroy_engine(ai_tentacle_engine *engine)
{
    using callback = void *(__fastcall *)(ai_tentacle_engine *, void *, unsigned);
    reinterpret_cast<callback>(get_vfunc(engine->m_vtbl, 0))(engine, nullptr, 1);
}
void *__fastcall native_destroy(ai_tentacle_dangle *self, void *, unsigned flags)
{
    self->~ai_tentacle_dangle();
    if (flags & 1)
        ::operator delete(self);
    return self;
}
int __fastcall native_type(ai_tentacle_dangle *, void *)
{
    return 5;
}
void __fastcall native_modifier(ai_tentacle_dangle *self, void *, ai_tentacle_engine *modifier)
{
    self->set_modifier(modifier);
}
bool __fastcall native_advance(ai_tentacle_dangle *self, void *, Float dt, bool modifier)
{
    return self->frame_advance(dt, modifier);
}
void __fastcall native_length(ai_tentacle_dangle *self, void *, float length)
{
    self->set_length(length);
}
float __fastcall native_get_length(ai_tentacle_dangle *self, void *)
{
    return self->get_length();
}

vector3d interpolate(const vector3d &start, const vector3d &end, float amount)
{
    return start + (end - start) * amount;
}

void update_end_facing(ai_tentacle_info *info)
{
    const vector3d start = info->positions.size() ? info->positions[info->positions.size() - 1]
                           : info->base_node      ? info->base_node->get_abs_position()
                                                  : info->field_60;
    vector3d facing = info->end_pos - start;
    if (facing.length() >= EPSILON) {
        facing.normalize();
    } else {
        facing = info->get_end_po().get_z_facing();
        if (facing.length() < EPSILON)
            facing = ZVEC;
    }
    po pose = info->get_abs_end_po();
    vector3d up = pose.get_y_facing();
    if (is_colinear(up, facing, 0.01f))
        up = pose.get_x_facing();
    pose.set_po(up, facing, info->end_pos);
    pose.set_po(pose.get_y_facing(), pose.get_z_facing(), info->end_pos);
    info->field_78 = quaternion{pose.m};
}
}  // namespace

void *ai_tentacle_dangle::native_vtable()
{
    static void *table[] = {reinterpret_cast<void *>(&native_destroy),
                            reinterpret_cast<void *>(&native_type),
                            reinterpret_cast<void *>(&native_modifier),
                            reinterpret_cast<void *>(&native_advance),
                            reinterpret_cast<void *>(&native_length),
                            reinterpret_cast<void *>(&native_get_length)};
    return table;
}

ai_tentacle_dangle::ai_tentacle_dangle(ai_tentacle_info *a2) : ai_tentacle_engine(a2)
{
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    this->tentacle_dangler = nullptr;
    this->field_20 = true;
    this->field_21 = true;
}

ai_tentacle_dangle::~ai_tentacle_dangle()
{
    delete tentacle_dangler;
    tentacle_dangler = nullptr;
    if (field_18) {
        destroy_engine(reinterpret_cast<ai_tentacle_engine *>(field_18));
        field_18 = 0;
    }
    if (simple_list_vars._sl_list_owner)
        simple_list_vars._sl_list_owner->erase(this);
}

void ai_tentacle_dangle::set_modifier(ai_tentacle_engine *modifier)
{
    if (field_18)
        destroy_engine(reinterpret_cast<ai_tentacle_engine *>(field_18));
    field_18 = reinterpret_cast<std::intptr_t>(modifier);
}

void ai_tentacle_dangle::setup(float length, const vector3d &velocity, bool update_end, bool collision)
{
    tentacle_dangler = new dangler;
    entity_base *owner = field_14->my_ai ? static_cast<entity_base *>(field_14->my_ai->field_64)
                                         : static_cast<entity_base *>(field_14->tentacle);
    if (field_14->base_node)
        owner = field_14->base_node->m_parent ? field_14->base_node->m_parent : field_14->base_node;
    const vector3d &start = field_14->base_node ? field_14->base_node->get_abs_position() : field_14->field_60;
    tentacle_dangler->init_dangle(start,
                                  field_14->end_pos,
                                  field_14->positions.data(),
                                  field_14->positions.size(),
                                  length,
                                  velocity,
                                  static_cast<char>(owner->my_handle.field_0));
    tentacle_dangler->collision_enabled = collision;
    tentacle_dangler->field_21 = true;
    field_20 = update_end;
    field_21 = true;
}

bool ai_tentacle_dangle::frame_advance(Float dt, bool)
{
    if (!tentacle_dangler)
        return true;
    const auto place_particle = [this](unsigned index, const vector3d &position) {
        auto &particle = (*tentacle_dangler->particles)[index];
        particle.previous_position = particle.position;
        particle.position = position;
        particle.preserve_previous_position = true;
    };
    place_particle(0, field_14->base_node ? field_14->base_node->get_abs_position() : field_14->field_60);
    if (field_21) {
        for (unsigned i = 0; i < field_14->positions.size(); ++i) {
            vector3d position = field_14->positions[i];
            if (field_14->nodes.size()) {
                if (field_14->tween_timer < field_14->tween_duration)
                    position = interpolate(field_14->tween_positions[i], position, field_14->tween_amount);
                position = interpolate(field_14->nodes[i]->get_abs_position(), position, field_14->field_14);
            }
            place_particle(i + 1, position);
        }
        vector3d end = field_14->end_pos;
        if (field_14->end_node) {
            if (field_14->tween_timer < field_14->tween_duration)
                end = interpolate(field_14->field_2C, end, field_14->tween_amount);
            end = interpolate(field_14->end_node->get_abs_position(), end, field_14->field_14);
        }
        place_particle(field_14->positions.size() + 1, end);
    }
    tentacle_dangler->frame_advance(dt);
    for (unsigned i = 0; i < field_14->positions.size(); ++i)
        field_14->positions[i] = (*tentacle_dangler->particles)[i + 1].position;
    if (field_20) {
        field_14->end_pos = tentacle_dangler->particles->back().position;
        update_end_facing(field_14);
    }
    if (field_18) {
        auto *modifier = reinterpret_cast<ai_tentacle_engine *>(field_18);
        using callback = bool(__fastcall *)(ai_tentacle_engine *, void *, Float, bool);
        if (reinterpret_cast<callback>(get_vfunc(modifier->m_vtbl, 12))(modifier, nullptr, dt, true)) {
            destroy_engine(modifier);
            field_18 = 0;
        }
    }
    return false;
}

void ai_tentacle_dangle::set_length(float length)
{
    const int links = tentacle_dangler->field_4 - 1;
    tentacle_dangler->length = length <= EPSILON ? links * 0.0f : links > 0 ? length : links * length;
}

float ai_tentacle_dangle::get_length() const
{
    const float length = tentacle_dangler->length / tentacle_dangler->field_4 - 1.0f;
    return tentacle_dangler->field_4 > 1 ? length * (tentacle_dangler->field_4 - 1) : length;
}
