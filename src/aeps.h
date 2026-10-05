#pragma once

#include "variable.h"
#include "float.hpp"
#include "matrix4x4.h"
#include "vector3d.h"
#include <vector.hpp>
#include <cstdint>

struct entity_base;
struct nglLightContext;
namespace aeps {

struct ActionInfoStruct {
    entity_base *owner{};
    entity_base *other{};
    unsigned hash{};
    int index{-1};
    unsigned flags{};
    float delay{};
    float lifetime{};
    float scale{1.0f};
    const vector3d *position{};
    unsigned field_24{};
    unsigned mask{~0u};
};

struct UpdateStruct;
struct UpdateTarget {
    std::intptr_t m_vtbl;
    uint32_t field_4[10];
    UpdateStruct *updater;
};

struct UpdateStruct {
    std::intptr_t m_vtbl;
    float elapsed;
    float delay;
    float lifetime;
    int field_10;
    bool field_14;
    bool started;
    uint8_t padding[2];
    UpdateTarget *target;

    void advance(float time);
};

struct ParticleFormat {
    uint32_t flags;
    uint32_t stride;
    uint8_t offsets[21];
    uint8_t padding[3];
};

struct GroupRenderInfo {
    ParticleFormat *format;
    uint32_t field_4;
    void *particles;
    int particle_count;
    vector3d center;
    float radius;
    matrix4x4 *transform;
    nglLightContext *light_context;
    vector3d scale;
    uint32_t field_34;
    int field_38;
    uint32_t field_3C;
};

struct GroupGraphics {
    std::intptr_t m_vtbl;
};

struct Group {
    void *field_0;
    bool active;
    bool field_5;
    uint8_t field_6[10];
    matrix4x4 transform;
    vector3d field_50;
    uint32_t field_5C;
    vector3d bounds_min;
    vector3d bounds_max;
    ParticleFormat format;
    GroupGraphics *graphics;
    void *field_9C;
    bool initialized;
    bool transformed;
    uint8_t field_A2[14];
    GroupRenderInfo render_info;
    void *particles;
    int particle_count;
    void *auxiliary;
    int auxiliary_capacity;
    int auxiliary_count;
    uint32_t field_104;
    uint32_t field_108;
    uint32_t field_10C;

    void render(nglLightContext *light_context);
    void release();
};

struct Effect;
struct EffectOwner {
    uint32_t field_0[9];
    Effect *effect;
    uint32_t field_28;
    int started;
    uint32_t field_30[25];
    int fade_index;
};

struct Effect {
    std::intptr_t m_vtbl;
    uint32_t field_4[3];
    matrix4x4 transform;
    uint32_t field_50[9];
    bool playing;
    bool render_seen;
    uint8_t field_76[2];
    bool initialized;
    uint8_t field_79[7];
    Group *groups_first;
    Group *groups_last;
    uint32_t field_88[2];
    EffectOwner *owner;
};

extern Var<_std::vector<UpdateStruct *>> s_activeStructs;
extern Var<_std::vector<Effect *>> s_activeFx;
extern Var<_std::vector<Effect *>> s_entityFx;
extern Var<_std::vector<Group *>> s_renderList;
using group_callback = int(__cdecl *)(void *, int);
Group *allocate_groups(unsigned count, group_callback callback, void *context);
void release_groups(Group *first, unsigned count, int reason = 3);
bool DoCallback(entity_base *owner, int callback, unsigned flags);
void DoSpideySenseEffect(entity_base *owner, float lifetime, unsigned flags);

void FrameAdvance(Float time);
void RefreshDevOptions();
void FrameSetupRenderAndThenRender();
void RenderAll();
void RemUpdater(UpdateStruct *updater);
void RemFx(Effect *effect);
void RemEntityFx(Effect *effect);
void Reset();
void Destroy();
void Init();
} // namespace aeps

extern void aeps_patch();
