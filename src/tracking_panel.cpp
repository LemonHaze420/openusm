#include "tracking_panel.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"
#include "comic_panels.h"
#include "entity_base.h"
#include "wds.h"

VALIDATE_SIZE(tracking_panel, 0x7C);

tracking_panel::tracking_panel(from_mash_in_place_constructor *a2) : field_0(a2), field_10(a2), field_18(a2) {}

void tracking_panel::unmash(mash_info_struct *a1, [[maybe_unused]] void *a3)
{
    a1->unmash_class_in_place(this->field_0, this);
    a1->unmash_class_in_place(this->field_10, this);
    a1->unmash_class_in_place(this->field_18, this);
}

namespace {
int __fastcall tracking_flags(tracking_panel_anim *, void *)
{
    return -1;
}
vector3d *__fastcall exit_location(tracking_panel_anim *self, void *, vector3d *out)
{
    *out = static_cast<exiting_tracking_panel_anim *>(self)->location;
    return out;
}
vector2d *__fastcall exit_size(tracking_panel_anim *self, void *, vector2d *out)
{
    *out = static_cast<exiting_tracking_panel_anim *>(self)->size;
    return out;
}
aarect<float, vector2d> *__fastcall exit_gutter(tracking_panel_anim *, void *, aarect<float, vector2d> *out)
{
    *out = {vector2d{0.0f, 0.0f}, vector2d{0.0f, 0.0f}};
    return out;
}
void __fastcall exit_advance(tracking_panel_anim *self, void *, Float dt)
{
    static_cast<exiting_tracking_panel_anim *>(self)->advance_exit(dt);
}
bool __fastcall exit_playing(tracking_panel_anim *self, void *)
{
    auto *anim = static_cast<exiting_tracking_panel_anim *>(self);
    return anim->enter_remaining > 0.0f || anim->hold_remaining > 0.0f || anim->exit_remaining > 0.0f;
}
tracking_panel_anim::callbacks exit_callbacks{
    tracking_flags, exit_location, exit_size, exit_gutter, exit_advance, exit_playing};
vector3d *__fastcall continuous_location(tracking_panel_anim *self, void *, vector3d *out)
{
    *out = static_cast<continuous_tracking_panel_anim *>(self)->location();
    return out;
}
vector2d *__fastcall continuous_size(tracking_panel_anim *self, void *, vector2d *out)
{
    auto *anim = static_cast<continuous_tracking_panel_anim *>(self);
    const float weight = anim->blend_weight();
    *out = anim->final_size + (anim->initial_size - anim->final_size) * weight;
    return out;
}
aarect<float, vector2d> *__fastcall continuous_gutter(tracking_panel_anim *self, void *, aarect<float, vector2d> *out)
{
    auto *anim = static_cast<continuous_tracking_panel_anim *>(self);
    const float weight = anim->blend_weight();
    *out = {anim->gutter_rect.field_0[0] * weight, anim->gutter_rect.field_0[1] * weight};
    return out;
}
void __fastcall continuous_advance(tracking_panel_anim *self, void *, Float dt)
{
    static_cast<continuous_tracking_panel_anim *>(self)->advance_continuous(dt);
}
bool __fastcall continuous_playing(tracking_panel_anim *self, void *)
{
    auto *anim = static_cast<continuous_tracking_panel_anim *>(self);
    return anim->enter_remaining > 0.0f || anim->hold_remaining > 0.0f || anim->exit_remaining > 0.0f;
}
tracking_panel_anim::callbacks continuous_callbacks{
    tracking_flags, continuous_location, continuous_size, continuous_gutter, continuous_advance, continuous_playing};
}

vector3d tracking_panel_anim::get_loc()
{
    vector3d out;
    m_vtbl->location(this, nullptr, &out);
    return out;
}
vector2d tracking_panel_anim::get_size()
{
    vector2d out;
    m_vtbl->size(this, nullptr, &out);
    return out;
}
aarect<float, vector2d> tracking_panel_anim::get_gutter_rect()
{
    aarect<float, vector2d> out;
    m_vtbl->gutter(this, nullptr, &out);
    return out;
}
void tracking_panel_anim::advance(Float dt)
{
    m_vtbl->advance(this, nullptr, dt);
}
bool tracking_panel_anim::is_playing()
{
    return m_vtbl->playing(this, nullptr);
}
void tracking_panel_anim::destroy()
{
    if (m_vtbl == &continuous_callbacks)
        delete static_cast<continuous_tracking_panel_anim *>(this);
    else
        delete static_cast<exiting_tracking_panel_anim *>(this);
}

exiting_tracking_panel_anim::exiting_tracking_panel_anim(tracking_panel *source)
    : enter_remaining(source->enter_duration), hold_remaining(source->hold_duration),
      exit_remaining(source->exit_duration), location(source->start_location), size{0.0f, 0.0f},
      velocity((source->end_location - source->start_location) * (1.0f / source->enter_duration)),
      size_velocity(source->panel_size * (1.0f / source->enter_duration)), definition(source)
{
    m_vtbl = &exit_callbacks;
    if (auto *entity = g_world_ptr->ent_mgr.get_entity(string_hash{source->field_0.c_str()})) {
        const auto projected = comic_panels::game_play_panel()->project_position(entity->get_abs_position());
        location = {projected[0], projected[1], location.z};
        velocity = (source->end_location - location) * (1.0f / enter_remaining);
    }
}

void exiting_tracking_panel_anim::advance_exit(Float dt)
{
    float remaining = dt;
    if (enter_remaining > 0.0f) {
        if (enter_remaining <= remaining) {
            remaining -= enter_remaining;
            location = definition->end_location;
            size = definition->panel_size;
            enter_remaining = 0.0f;
        } else {
            enter_remaining -= remaining;
            location += velocity * remaining;
            size += size_velocity * remaining;
            remaining = 0.0f;
        }
    }
    if (hold_remaining > 0.0f && remaining > 0.0f) {
        if (hold_remaining <= remaining) {
            remaining -= hold_remaining;
            if (auto *entity = g_world_ptr->ent_mgr.get_entity(string_hash{definition->field_0.c_str()})) {
                const auto projected = comic_panels::game_play_panel()->project_position(entity->get_abs_position());
                velocity =
                    vector3d{projected[0] - location.x, projected[1] - location.y, 0.0f} * (1.0f / exit_remaining);
                size_velocity = definition->panel_size * (-1.0f / exit_remaining);
                hold_remaining = 0.0f;
            }
        } else {
            hold_remaining -= remaining;
            remaining = 0.0f;
        }
    }
    if (exit_remaining > 0.0f && remaining > 0.0f) {
        exit_remaining -= remaining;
        location += velocity * remaining;
        size += size_velocity * remaining;
    }
}

continuous_tracking_panel_anim::continuous_tracking_panel_anim(tracking_panel *source)
    : enter_remaining(source->enter_duration), hold_remaining(source->hold_duration),
      exit_remaining(source->exit_duration), definition(source), initial_location(source->end_location),
      initial_size(source->panel_size), final_size(source->final_size),
      gutter_rect{vector2d{-source->gutter_width, -source->gutter_width},
                  vector2d{source->gutter_width, source->gutter_width}}
{
    m_vtbl = &continuous_callbacks;
}
vector3d continuous_tracking_panel_anim::tracked_location() const
{
    if (auto *entity = g_world_ptr->ent_mgr.get_entity(string_hash{definition->field_0.c_str()})) {
        const auto projected = comic_panels::game_play_panel()->project_position(entity->get_abs_position());
        return {projected[0], projected[1], initial_location.z + initial_location.z};
    }
    return {0.0f, 0.0f, initial_location.z};
}
vector3d continuous_tracking_panel_anim::location() const
{
    if (enter_remaining > 0.0f) {
        const float tracked_weight = enter_remaining / definition->enter_duration;
        return tracked_location() * tracked_weight + initial_location * (1.0f - tracked_weight);
    }
    if (hold_remaining > 0.0f || exit_remaining < 0.0f)
        return initial_location;
    const float initial_weight = exit_remaining / definition->exit_duration;
    return initial_location * initial_weight + tracked_location() * (1.0f - initial_weight);
}
float continuous_tracking_panel_anim::blend_weight() const
{
    if (enter_remaining > 0.0f)
        return (definition->enter_duration - enter_remaining) / definition->enter_duration;
    if (hold_remaining > 0.0f || exit_remaining < 0.0f)
        return 1.0f;
    return exit_remaining / definition->exit_duration;
}
void continuous_tracking_panel_anim::advance_continuous(Float dt)
{
    float remaining = dt;
    if (enter_remaining > 0.0f) {
        const float bias = definition->transition_bias + enter_remaining;
        const float increment = remaining / (bias * bias);
        if (increment >= enter_remaining) {
            remaining = increment - enter_remaining;
            enter_remaining = 0.0f;
        } else {
            enter_remaining -= increment;
            remaining = 0.0f;
        }
    }
    if (hold_remaining > 0.0f && remaining > 0.0f) {
        if (remaining >= hold_remaining) {
            remaining -= hold_remaining;
            hold_remaining = 0.0f;
        } else {
            hold_remaining -= remaining;
            remaining = 0.0f;
        }
    }
    if (exit_remaining > 0.0f && remaining > 0.0f) {
        const float bias = definition->exit_duration - exit_remaining + definition->transition_bias;
        exit_remaining -= remaining / (bias * bias);
        if (exit_remaining < 0.0f)
            exit_remaining = 0.0f;
    }
}

VALIDATE_SIZE(exiting_tracking_panel_anim, 0x3C);
VALIDATE_SIZE(continuous_tracking_panel_anim, 0x40);
