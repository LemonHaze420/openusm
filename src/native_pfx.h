#pragma once

#include "aeps.h"
#include "entity.h"
#include "parse_generic_mash.h"
#include "effect_mash_layout.h"
#include <cstddef>

struct mash_info_struct;

struct pfx_interface;
namespace native_pfx {
struct PointerVector {
    uint32_t end_offset;
    uint32_t count;
    void **data;
    uint32_t capacity;
    uint32_t flags;
};
struct Format {
    uint32_t flags;
    uint32_t stride;
    uint8_t offsets[21];
    uint8_t padding[3];
};
struct ParticleTemplate {
    unsigned capacity;
    float begin;
    float end;
    uint32_t field_C;
    bool local_space;
    uint8_t padding[3];
    aeps::GroupGraphics *graphics;
    PointerVector *actions;
};
struct EffectTemplate {
    PointerVector particles;
    PointerVector auxiliaries;
    uint32_t field_28[2];
    matrix4x4 transform;
    uint32_t field_70[8];
};
struct Point {
    int type;
    vector3d position;
    float radius;
    float weight;
};
struct Updater;
struct Instance {
    intptr_t m_vtbl;
    uint32_t mashed;
    entity *owner;
    uint32_t field_C;
    Updater *updater;
    _std::vector<aeps::UpdateStruct *> action_updaters;
    aeps::Effect *effect;
    EffectTemplate *resource;
    int started;
    int playing;
    uint32_t color;
    float alpha;
    float alpha_multiplier;
    vector3d position;
    vector3d field_4C;
    vector3d scale;
    uint32_t field_64;
    uint32_t flags;
    float field_6C;
    float z_depth;
    float fade;
    float field_78;
    float field_7C;
    PointerVector points;
    uint32_t field_94;
};
struct Entity : entity {
    Instance *particle;
};
struct Effect {
    intptr_t m_vtbl;
    uint32_t field_4[3];
    matrix4x4 transform;
    vector3d scale;
    float z_depth;
    uint32_t color;
    float start_time;
    float previous_time;
    EffectTemplate *resource;
    unsigned groups_started;
    bool playing;
    bool render_seen;
    bool reset;
    bool retry;
    bool initialized;
    uint8_t padding[3];
    float fade;
    aeps::Group *groups_first;
    aeps::Group *groups_last;
    uint32_t field_88[2];
    Instance *owner;
    uint32_t field_94[3];
};
struct Updater {
    intptr_t m_vtbl;
    float elapsed;
    float delay;
    float lifetime;
    uint32_t field_10;
    bool active;
    bool started;
    uint8_t padding[2];
    Instance *owner;
    Effect *effect;
    float scale;
};

Instance *load(generic_mash_data_ptrs *data, Entity *owner);
Instance *construct_instance(void *storage);
void unmash_instance(Instance *instance, mash_info_struct *info);
void *instance_mash_vtable();
void fixup_instance(Instance *instance);
void spawn(Instance *instance);
bool spawn_action(Instance *instance, unsigned flags, float delay, float lifetime, float scale);
void set_position(Instance *instance, const vector3d &position);
void attach_instance(pfx_interface *owner, Instance *instance);
void release_attached_instances(pfx_interface *owner);
void set_attached_owner(Instance *instance, entity_base *owner);
void visible(Instance *instance, bool value);
void release(Instance *instance);
void update_effect(Effect &effect, float elapsed);
void install_entity_callbacks(void **vtable);

static_assert(sizeof(PointerVector) == 20);
static_assert(sizeof(Format) == 32);
static_assert(sizeof(ParticleTemplate) == 28);
static_assert(sizeof(EffectTemplate) == 144);
static_assert(sizeof(Instance) == 152);
static_assert(sizeof(Effect) == 160);
static_assert(sizeof(Updater) == 36);
static_assert(offsetof(Instance, resource) == 0x28);
static_assert(offsetof(Instance, points) == 0x80);
static_assert(offsetof(Effect, groups_first) == 0x80);
static_assert(offsetof(Updater, effect) == 0x1C);
}  // namespace native_pfx
