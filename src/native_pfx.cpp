#include "native_pfx.h"
#include "aps_native_graphics.h"
#include "aps_native_curves.h"
#include "common.h"
#include "memory.h"
#include "fixedstring.h"
#include "ngl.h"
#include "oldmath_po.h"
#include "vtbl.h"
#include "sound_and_pfx_interface.h"
#include "mash_info_struct.h"
#include "os_developer_options.h"
#include "poi.h"
#include <new>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>
#include <functional>

namespace native_pfx {
namespace {
void align(uint8_t *&stream, uintptr_t alignment)
{
    stream = reinterpret_cast<uint8_t *>((reinterpret_cast<uintptr_t>(stream) + alignment - 1) & ~(alignment - 1));
}
void *record(uint8_t *&stream, size_t size, size_t alignment)
{
    if (alignment != 0)
        align(stream, alignment);
    else {
        auto *end = stream;
        while (*end == effect_mash::alignment_marker)
            ++end;
        stream += static_cast<size_t>(end - stream) & ~size_t{3};
    }
    auto *result = stream;
    stream += size;
    return result;
}
void pointers(PointerVector &vector, uint8_t *&stream)
{
    if (vector.data != nullptr)
        vector.data = static_cast<void **>(record(stream, vector.count * sizeof(void *), 4));
    else
        vector.count = vector.capacity = 0;
}
struct ActionVector {
    uint32_t end_offset;
    uint32_t count;
    void **data;
    uint32_t capacity;
};
void action_pointers(ActionVector &vector, uint8_t *&stream)
{
    if (vector.data != nullptr)
        vector.data = static_cast<void **>(record(stream, vector.count * sizeof(void *), 4));
    else
        vector.count = vector.capacity = 0;
}
void *unmash_action(uint8_t *&stream)
{
    auto *object = static_cast<uint32_t *>(record(stream, 4, 0));
    const unsigned type = *object;
    assert(type >= 8 && type < effect_mash::aps_sizes.size());
    record(stream, effect_mash::aps_sizes[type] - 4, 1);
    if (type >= 23) {
        auto &parameters = *reinterpret_cast<ActionVector *>(object + 1);
        auto &domains = *reinterpret_cast<ActionVector *>(object + 5);
        action_pointers(parameters, stream);
        parameters.end_offset = static_cast<uint32_t>(stream - reinterpret_cast<uint8_t *>(&parameters));
        action_pointers(domains, stream);
        for (unsigned i = 0; i < domains.count; ++i)
            if (domains.data[i] != nullptr)
                domains.data[i] = unmash_action(stream);
        domains.end_offset = static_cast<uint32_t>(stream - reinterpret_cast<uint8_t *>(&domains));
    }
    aps_native_curve_fixup(object, type);
    return object;
}
EffectTemplate *unmash_template(uint8_t *&stream)
{
    auto *result = static_cast<EffectTemplate *>(record(stream, sizeof(EffectTemplate), 16));
    pointers(result->particles, stream);
    for (unsigned i = 0; i < result->particles.count; ++i) {
        if (result->particles.data[i] == nullptr)
            continue;
        auto *particle = static_cast<ParticleTemplate *>(record(stream, sizeof(ParticleTemplate), 4));
        result->particles.data[i] = particle;
        if (particle->graphics != nullptr) {
            auto *graphics = static_cast<uint32_t *>(record(stream, 4, 0));
            const unsigned type = *graphics;
            assert(type < 8);
            record(stream, effect_mash::aps_sizes[type] - 4, 1);
            particle->graphics = reinterpret_cast<aeps::GroupGraphics *>(graphics);
            aps_native_graphics_fixup(graphics, type);
        }
        if (particle->actions != nullptr) {
            particle->actions = static_cast<PointerVector *>(record(stream, sizeof(PointerVector), 4));
            pointers(*particle->actions, stream);
            for (unsigned j = 0; j < particle->actions->count; ++j)
                if (particle->actions->data[j] != nullptr)
                    particle->actions->data[j] = unmash_action(stream);
            particle->actions->end_offset =
                static_cast<uint32_t>(stream - reinterpret_cast<uint8_t *>(particle->actions));
        }
    }
    result->particles.end_offset = static_cast<uint32_t>(stream - reinterpret_cast<uint8_t *>(&result->particles));
    pointers(result->auxiliaries, stream);
    for (unsigned i = 0; i < result->auxiliaries.count; ++i)
        if (result->auxiliaries.data[i] != nullptr)
            result->auxiliaries.data[i] = record(stream, sizeof(effect_mash::Auxiliary), 4);
    result->auxiliaries.end_offset = static_cast<uint32_t>(stream - reinterpret_cast<uint8_t *>(&result->auxiliaries));
    align(stream, 4);
    for (unsigned i = 0; i < result->particles.count; ++i) {
        auto *particle = static_cast<ParticleTemplate *>(result->particles.data[i]);
        if (particle != nullptr && particle->graphics != nullptr && *stream != 0) {
            const char *name = reinterpret_cast<const char *>(stream);
            auto *texture = nglGetTexture(tlFixedString{_stricmp(name, "NGLDEFAULT") == 0 ? "c_banana" : name});
            *reinterpret_cast<nglTexture **>(reinterpret_cast<uint8_t *>(particle->graphics) + 8) = texture;
            stream += std::strlen(name) + 1;
        }
    }
    align(stream, 16);
    return result;
}
Format &format(aeps::Group &group)
{
    return *reinterpret_cast<Format *>(&group.format);
}
constexpr unsigned attribute_sizes[21]{12, 4, 4, 12, 4, 16, 4, 4, 4, 4, 4, 4, 12, 4, 12, 12, 12, 12, 4, 4, 4};
void set_format(Format &value, unsigned flags)
{
    value.flags = flags;
    unsigned size = 0;
    for (unsigned i = 0; i < 21; ++i) {
        value.offsets[i] = flags & (1u << i) ? size : 0;
        if (flags & (1u << i))
            size += attribute_sizes[i];
    }
    value.stride = (size + 15) & ~15u;
}
unsigned action_format(const PointerVector *actions)
{
    unsigned flags = 0;
    if (actions != nullptr)
        for (unsigned i = 0; i < actions->count; ++i)
            flags |= static_cast<uint32_t *>(actions->data[i])[11];
    return flags;
}
void group_transform(aeps::Group &group, const matrix4x4 &transform)
{
    group.field_50 = std::equal_to<float>{}(group.field_50.x, 0.0f) && std::equal_to<float>{}(group.field_50.y, 0.0f) &&
                             std::equal_to<float>{}(group.field_50.z, 0.0f)
                         ? vector3d{transform[3].x, transform[3].y, transform[3].z}
                         : vector3d{group.transform[3].x, group.transform[3].y, group.transform[3].z};
    group.transform = transform;
}
void init_group(aeps::Group &group, Effect &effect, ParticleTemplate &particle, unsigned index)
{
    group = {};
    group.field_0 = &effect.transform;
    group.field_5 = true;
    group.transform = identity_matrix;
    group.bounds_min = {
        std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    group.bounds_max = {
        -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
    set_format(format(group), action_format(particle.actions));
    group.graphics = particle.graphics;
    group.field_9C = reinterpret_cast<void *>(particle.capacity);
    group.initialized = true;
    group.transformed = particle.local_space;
    group.render_info.format = &group.format;
    group.render_info.field_4 = index;
    group.render_info.scale = {1.0f, 1.0f, 1.0f};
    group.render_info.field_38 = -1;
}
void allocate_group(aeps::Group &group, ParticleTemplate &particle)
{
    if (group.particles != nullptr)
        return;
    set_format(format(group), action_format(particle.actions) | aps_native_graphics_format(particle.graphics) | 1536);
    group.particles = tlMemAlloc(format(group).stride * particle.capacity, 128, 0x03000000);
    assert(group.particles != nullptr);
}
}

Instance *load(generic_mash_data_ptrs *data, Entity *owner)
{
    auto *&normal = data->field_0;
    auto *&shared = data->field_4;
    const bool mismatch = (reinterpret_cast<uintptr_t>(normal) & 15u) == 8u;
    align(normal, 16);
    if (mismatch) {
        record(normal, 80, 1);
        shared += 8;
        align(shared, 8);
        shared += 8;
    }
    auto *instance = static_cast<Instance *>(record(normal, sizeof(Instance), 0));
    assert(instance->m_vtbl == effect_mash::particle_instance_type);
    pointers(instance->points, normal);
    for (unsigned i = 0; i < instance->points.count; ++i)
        instance->points.data[i] = record(normal, sizeof(Point), 4);
    instance->points.end_offset = static_cast<uint32_t>(normal - reinterpret_cast<uint8_t *>(&instance->points));
    align(normal, 16);
    fixup_instance(instance);
    instance->resource = unmash_template(shared);
    instance->owner = owner;
    if (owner != nullptr) {
        for (unsigned i = 0; i < instance->points.count; ++i) {
            const auto &point = *static_cast<const Point *>(instance->points.data[i]);
            poi_manager::add_point_of_interest(
                point.position, point.type, point.radius, point.weight, {owner->my_handle});
        }
        spawn(instance);
        owner->set_fade_distance(Float{96.0f});
    }
    return instance;
}

namespace {
void camera_basis(float *first, float *second, float *angle)
{
    std::memcpy(first, &var<vector3d>(0x0093A4E0), sizeof(vector3d));
    std::memcpy(second, &var<vector3d>(0x0093A4EC), sizeof(vector3d));
    *angle = var<float>(0x009711F4) - 1.5707963267948966f;
}
void kill_particle(void *object, void *particle)
{
    auto &group = *static_cast<aeps::Group *>(object);
    if (group.auxiliary_count == group.auxiliary_capacity) {
        const unsigned capacity = group.auxiliary_count <= 3 ? group.auxiliary_count + 1 : group.auxiliary_count + 4;
        auto **entries = static_cast<void **>(tlMemAlloc(capacity * sizeof(void *), 8, 0x03000000));
        assert(entries != nullptr);
        if (group.auxiliary_count != 0)
            std::memcpy(entries, group.auxiliary, group.auxiliary_count * sizeof(void *));
        tlMemFree(group.auxiliary);
        group.auxiliary = entries;
        group.auxiliary_capacity = capacity;
    }
    static_cast<void **>(group.auxiliary)[group.auxiliary_count++] = particle;
}
vector3d rotate_vector(const matrix4x4 &matrix, const vector3d &point)
{
    return {matrix[0].x * point.x + matrix[1].x * point.y + matrix[2].x * point.z,
            matrix[0].y * point.x + matrix[1].y * point.y + matrix[2].y * point.z,
            matrix[0].z * point.x + matrix[1].z * point.y + matrix[2].z * point.z};
}
void normalize(vector3d &point)
{
    const float length = std::sqrt(point.x * point.x + point.y * point.y + point.z * point.z);
    if (std::not_equal_to<float>{}(length, 0.0f))
        point = point * (1.0f / length);
}
void emit_particles(void *object, void *, void *, void *group_object, float dt, const float *modifiers)
{
    auto &action = *static_cast<aps_native::Action *>(object);
    auto &group = *static_cast<aeps::Group *>(group_object);
    const int count = aps_native_emitter_count(object, &group, dt);
    group.field_108 = group.field_104;
    group.field_104 =
        count > 0 && static_cast<unsigned>(group.particle_count) < reinterpret_cast<uintptr_t>(group.field_9C) ? count
                                                                                                               : 0;
    if (group.field_104 == 0)
        return;
    const auto &layout = format(group);
    assert(layout.stride <= 256);
    auto **domains = reinterpret_cast<void **>(action.domains.data);
    const vector3d current{group.transform[3].x, group.transform[3].y, group.transform[3].z};
    const vector3d step = count > 1 ? (current - group.field_50) * (1.0f / count) : vector3d{0.0f, 0.0f, 0.0f};
    vector3d origin = count > 1 ? group.field_50 + step : current;
    uint8_t particle[256]{};
    constexpr unsigned attributes[16]{0, 1, 2, 3, 4, 6, 7, 8, 10, 11, 12, 13, 14, 18, 19, 20};
    constexpr float defaults[16]{
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    for (int emission = 0; emission < count; ++emission) {
        for (unsigned domain = 0; domain < 16; ++domain) {
            const unsigned attribute = attributes[domain];
            if ((layout.flags & (1u << attribute)) == 0)
                continue;
            auto *value = reinterpret_cast<float *>(particle + layout.offsets[attribute]);
            const unsigned components = attribute_sizes[attribute] / sizeof(float);
            const bool sampled = domain < action.domains.count && domains[domain] != nullptr;
            if (sampled)
                aps_native_domain_sample(domains[domain], components, value);
            else
                std::fill(value, value + components, defaults[domain]);
            if ((domain == 1 || domain == 2 || domain == 6) && sampled)
                *value *= modifiers[domain == 6 ? 2 : 0];
        }
        auto &position = *reinterpret_cast<vector3d *>(particle);
        if (!group.transformed)
            position = rotate_vector(group.transform, position) + origin;
        origin = origin + step;
        *reinterpret_cast<float *>(particle + layout.offsets[9]) = 0.0f;
        for (const unsigned attribute : {12u, 14u})
            if ((layout.flags & (1u << attribute)) != 0 && !group.transformed) {
                auto &value = *reinterpret_cast<vector3d *>(particle + layout.offsets[attribute]);
                value = rotate_vector(group.transform, value);
            }
        if ((layout.flags & (1u << 15)) != 0) {
            auto &forward = *reinterpret_cast<vector3d *>(particle + layout.offsets[15]);
            auto &right = *reinterpret_cast<vector3d *>(particle + layout.offsets[16]);
            auto &up = *reinterpret_cast<vector3d *>(particle + layout.offsets[17]);
            forward = *reinterpret_cast<vector3d *>(particle + layout.offsets[12]);
            normalize(forward);
            right = std::equal_to<float>{}(forward.z, 0.0f) ? vector3d{forward.y, -forward.x, 0.0f}
                                                            : vector3d{forward.z, 0.0f, -forward.x};
            normalize(right);
            up = {forward.y * right.z - forward.z * right.y,
                  forward.z * right.x - forward.x * right.z,
                  forward.x * right.y - forward.y * right.x};
        }
        if (static_cast<unsigned>(group.particle_count) < reinterpret_cast<uintptr_t>(group.field_9C)) {
            std::memcpy(static_cast<uint8_t *>(group.particles) + layout.stride * group.particle_count,
                        particle,
                        layout.stride);
            ++group.particle_count;
        }
    }
}
void action_unmash(void *object, void *info, int)
{
    struct MashInfo {
        uint8_t *base;
        unsigned used;
        unsigned size;
    };
    auto &mash = *static_cast<MashInfo *>(info);
    auto *stream = mash.base + mash.used;
    auto &action = *static_cast<aps_native::Action *>(object);
    action_pointers(*reinterpret_cast<ActionVector *>(&action.parameters), stream);
    action_pointers(*reinterpret_cast<ActionVector *>(&action.domains), stream);
    auto **domains = reinterpret_cast<void **>(action.domains.data);
    for (unsigned i = 0; i < action.domains.count; ++i)
        if (domains[i] != nullptr)
            domains[i] = unmash_action(stream);
    mash.used = stream - mash.base;
}
void finish_group(aeps::Group &group)
{
    const auto &layout = format(group);
    group.bounds_min = {
        std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    group.bounds_max = {
        -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
    for (int i = 0; i < group.particle_count; ++i) {
        const auto *particle = static_cast<const uint8_t *>(group.particles) + i * layout.stride;
        const auto &position = *reinterpret_cast<const vector3d *>(particle);
        float radius = 1.0f;
        if ((layout.flags & 2) != 0)
            radius = *reinterpret_cast<const float *>(particle + layout.offsets[1]) * 1.4199999570846558f;
        else if ((layout.flags & 4) != 0)
            radius = *reinterpret_cast<const float *>(particle + layout.offsets[2]) +
                     *reinterpret_cast<const float *>(particle + layout.offsets[7]);
        for (unsigned axis = 0; axis < 3; ++axis) {
            group.bounds_min[axis] = std::min(group.bounds_min[axis], position[axis] - radius);
            group.bounds_max[axis] = std::max(group.bounds_max[axis], position[axis] + radius);
        }
    }
    if (group.transformed && group.particle_count != 0) {
        const auto minimum = group.bounds_min;
        const auto maximum = group.bounds_max;
        group.bounds_min = {
            std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        group.bounds_max = {
            -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
        for (unsigned corner = 0; corner < 8; ++corner) {
            const auto point = rotate_vector(group.transform,
                                             vector3d{corner & 1 ? maximum.x : minimum.x,
                                                      corner & 2 ? maximum.y : minimum.y,
                                                      corner & 4 ? maximum.z : minimum.z}) +
                               vector3d{group.transform[3].x, group.transform[3].y, group.transform[3].z};
            for (unsigned axis = 0; axis < 3; ++axis) {
                group.bounds_min[axis] = std::min(group.bounds_min[axis], point[axis]);
                group.bounds_max[axis] = std::max(group.bounds_max[axis], point[axis]);
            }
        }
    }
    auto **killed = static_cast<void **>(group.auxiliary);
    for (int i = group.auxiliary_count; i != 0; --i) {
        auto *last = static_cast<uint8_t *>(group.particles) + layout.stride * (group.particle_count - 1);
        if (last != killed[i - 1])
            std::memcpy(killed[i - 1], last, layout.stride);
        --group.particle_count;
    }
    group.auxiliary_count = 0;
}
int __cdecl effect_pool_callback(void *context, int reason)
{
    auto &effect = *static_cast<Effect *>(context);
    switch (reason) {
    case 2:
        effect.initialized = true;
        return 0;
    case 4:
        if (effect.render_seen || effect.playing)
            return 2;
        if (effect.retry)
            return 1;
        return effect.fade < 1.0f ? 1 : 2;
    case 5:
        return effect.retry || effect.fade < 1.0f ? 1 : 2;
    case 6:
        effect.retry = true;
        [[fallthrough]];
    case 1:
    case 3:
        effect.initialized = false;
        effect.reset = true;
        return 0;
    default:
        return 0;
    }
}
void initialize_groups(Effect &effect)
{
    effect.groups_first = aeps::allocate_groups(effect.resource->particles.count, effect_pool_callback, &effect);
    if (!effect.initialized)
        return;
    effect.groups_last = effect.groups_first + effect.resource->particles.count;
    for (unsigned i = 0; i < effect.resource->particles.count; ++i)
        init_group(
            effect.groups_first[i], effect, *static_cast<ParticleTemplate *>(effect.resource->particles.data[i]), i);
}
void __fastcall destroy_effect(Effect *effect, void *, int deallocate)
{
    if (effect->initialized && effect->groups_first != nullptr && effect->groups_first->field_0 == &effect->transform)
        aeps::release_groups(effect->groups_first, effect->resource->particles.count);
    effect->owner = nullptr;
    if (deallocate & 1)
        delete effect;
}
void __fastcall destroy_updater(Updater *updater, void *, int deallocate)
{
    updater->effect = nullptr;
    if (deallocate & 1)
        tlMemFree(updater);
}
bool finished(const Effect &effect)
{
    if (effect.groups_started < effect.resource->particles.count)
        return false;
    for (unsigned i = 0; i < effect.resource->particles.count; ++i)
        if (effect.previous_time < static_cast<ParticleTemplate *>(effect.resource->particles.data[i])->end)
            return false;
    return true;
}
void __fastcall set_entity_visible(Entity *owner, void *, bool value, bool family);
void __fastcall update_updater(Updater *updater, void *, float dt, int)
{
    auto *effect = updater->effect;
    if (effect == nullptr)
        return;
    if (effect->initialized && effect->groups_first->field_0 != &effect->transform)
        return;
    updater->elapsed += dt;
    if (finished(*effect)) {
        set_entity_visible(static_cast<Entity *>(updater->owner->owner), nullptr, false, false);
        return;
    }
    if ((effect->playing && effect->initialized) ||
        ((updater->owner->flags & 0x10) != 0 && std::not_equal_to<float>{}(updater->owner->field_7C, 0.0f))) {
        effect->playing = true;
        const auto &owner_transform = updater->owner->owner->get_abs_po().m;
        effect->transform = effect->resource->transform * owner_transform;
        if (effect->initialized)
            for (auto *group = effect->groups_first; group < effect->groups_last; ++group)
                group_transform(*group, effect->transform);
        update_effect(*effect, updater->elapsed);
    }
    effect->playing = false;
    updater->owner->started = 0;
}
void __fastcall idle_updater(Updater *updater, void *, float dt, int argument)
{
    if ((updater->owner->flags & 0x10) != 0 && std::not_equal_to<float>{}(updater->owner->field_7C, 0.0f)) {
        update_updater(updater, nullptr, dt, argument);
        return;
    }
    if (updater->effect != nullptr) {
        updater->elapsed += dt;
        if (finished(*updater->effect))
            set_entity_visible(static_cast<Entity *>(updater->owner->owner), nullptr, false, false);
    }
}
void __fastcall reset_updater(Updater *updater, void *)
{
    updater->elapsed = 0.0f;
    auto *effect = updater->effect;
    if (effect == nullptr)
        return;
    effect->reset = false;
    effect->start_time = effect->previous_time = 0.0f;
    if (effect->initialized)
        for (unsigned i = 0; i < effect->resource->particles.count; ++i) {
            auto &group = effect->groups_first[i];
            group.particle_count = 0;
            const float modifiers[3]{1.0f, 1.0f, 1.0f};
            auto *particle = static_cast<ParticleTemplate *>(effect->resource->particles.data[i]);
            aps_native_actions_run(particle->actions, &group, 0.0f, 0.0f, true, modifiers);
        }
}
void __fastcall preroll_updater(Updater *updater, void *, float time, bool reset)
{
    if (updater->effect == nullptr)
        return;
    if (reset)
        reset_updater(updater, nullptr);
    else
        time += updater->elapsed;
    while (updater->elapsed < time) {
        updater->elapsed += 0.03333333507180214f;
        updater->effect->playing = true;
        update_effect(*updater->effect, updater->elapsed);
        updater->effect->playing = false;
    }
}
void __fastcall destroy_action_updater(Updater *updater, void *, int deallocate)
{
    auto *instance = updater->owner;
    if (instance != nullptr) {
        if (instance->updater == updater) {
            instance->updater = nullptr;
            instance->effect = nullptr;
        } else {
            auto &list = instance->action_updaters;
            auto it = std::find(list.begin(), list.end(), reinterpret_cast<aeps::UpdateStruct *>(updater));
            if (it != list.end())
                list.erase(it);
        }
    }
    if (updater->effect != nullptr) {
        aeps::RemFx(reinterpret_cast<aeps::Effect *>(updater->effect));
        destroy_effect(updater->effect, nullptr, 1);
        updater->effect = nullptr;
    }
    if (deallocate & 1)
        tlMemFree(updater);
}


void __fastcall advance_action_updater(Updater *updater, void *, float time, int)
{
    auto *effect = updater->effect;
    if (effect != nullptr) {
        const float delta = time * updater->scale;
        updater->elapsed += delta;
        if (updater->delay > 0.0f) {
            updater->elapsed -= delta;
            updater->delay -= time;
            if (updater->delay > 0.0f)
                return;
            updater->elapsed -= updater->delay;
        }
        if (updater->lifetime > 0.0f)
            updater->lifetime -= time;
        if (updater->lifetime >= 0.0f && !finished(*effect)) {
            if (effect->render_seen && effect->initialized)
                update_effect(*effect, updater->elapsed);
            return;
        }
    }
    aeps::RemUpdater(reinterpret_cast<aeps::UpdateStruct *>(updater));
    destroy_action_updater(updater, nullptr, 1);
}


void __fastcall idle_action_updater(Updater *, void *, float, int) {}
void *action_updater_vtable[]{reinterpret_cast<void *>(destroy_action_updater),
                              reinterpret_cast<void *>(advance_action_updater),
                              reinterpret_cast<void *>(idle_action_updater),
                              reinterpret_cast<void *>(reset_updater),
                              reinterpret_cast<void *>(preroll_updater)};

struct AttachedInstance {
    intptr_t m_vtbl;
    Instance *instance;
    uint32_t handle;
    entity_base *owner;
    bool pending;
    uint8_t padding[3];
};
static_assert(sizeof(AttachedInstance) == 20);
void __fastcall destroy_attachment(AttachedInstance *entry, void *, int deallocate)
{
    if (deallocate & 1)
        tlMemFree(entry);
}

void *attachment_vtable[]{reinterpret_cast<void *>(destroy_attachment)};

void *effect_vtable[]{reinterpret_cast<void *>(destroy_effect)};
void *updater_vtable[]{reinterpret_cast<void *>(destroy_updater),
                       reinterpret_cast<void *>(update_updater),
                       reinterpret_cast<void *>(idle_updater),
                       reinterpret_cast<void *>(reset_updater),
                       reinterpret_cast<void *>(preroll_updater)};
void __fastcall destroy_instance(Instance *instance, void *, int)
{
    release(instance);
}
void __fastcall instance_unmash(Instance *instance, void *, mash_info_struct *info, void *)
{
    unmash_instance(instance, info);
}
unsigned __fastcall instance_type(Instance *, void *)
{
    return 7;
}
unsigned __fastcall instance_size(Instance *, void *)
{
    return sizeof(Instance);
}
void *instance_vtable[]{reinterpret_cast<void *>(destroy_instance),
                        reinterpret_cast<void *>(instance_unmash),
                        reinterpret_cast<void *>(destroy_instance),
                        reinterpret_cast<void *>(instance_type),
                        nullptr,
                        nullptr,
                        reinterpret_cast<void *>(instance_size)};
void __fastcall set_entity_visible(Entity *owner, void *, bool value, bool family)
{
    owner->entity::_set_visible(value, family);
    if (owner->particle == nullptr)
        return;
    auto &instance = *owner->particle;
    auto *effect = reinterpret_cast<Effect *>(instance.effect);
    if (effect != nullptr) {
        const bool active = (effect->render_seen && effect->initialized) || instance.playing;
        if (value) {
            instance.updater->elapsed = 0.0f;
            if (!active) {
                aeps::s_activeStructs().push_back(reinterpret_cast<aeps::UpdateStruct *>(instance.updater));
                aeps::s_entityFx().push_back(instance.effect);
            }
            visible(&instance, false);
        } else if (active) {
            aeps::RemEntityFx(instance.effect);
            aeps::RemUpdater(reinterpret_cast<aeps::UpdateStruct *>(instance.updater));
        }
    }
    visible(&instance, value);
}
void __fastcall destroy_entity(Entity *owner, void *, int deallocate)
{
    if (owner->particle != nullptr) {
        release(owner->particle);
        owner->particle = nullptr;
    }
    owner->entity::~entity();
    if (deallocate & 1)
        tlMemFree(owner);
}
void __fastcall release_entity(Entity *owner, void *)
{
    if (owner->particle != nullptr) {
        release(owner->particle);
        owner->particle = nullptr;
    }
    owner->entity::release_mem();
}
void __fastcall set_color(Entity *owner, void *, color32 value)
{
    if (owner->particle == nullptr)
        return;
    const uint32_t packed = static_cast<uint32_t>(color32::to_int(value));
    auto *effect = reinterpret_cast<Effect *>(owner->particle->effect);
    if (effect != nullptr) {
        effect->color = packed;
        if (effect->initialized)
            for (auto *group = effect->groups_first; group < effect->groups_last; ++group)
                group->render_info.field_38 = packed;
    }
    owner->particle->color = packed;
}
color32 *__fastcall get_color(const Entity *owner, void *, color32 *result)
{
    if (owner->particle == nullptr)
        *result = color32{0xFFFFFFFF};
    else if (owner->particle->effect != nullptr)
        *result = color32{reinterpret_cast<const Effect *>(owner->particle->effect)->color};
    else
        *result = color32{owner->particle->color};
    return result;
}
}

void *instance_mash_vtable()
{
    return instance_vtable;
}

Instance *construct_instance(void *storage)
{
    auto *instance = storage != nullptr ? static_cast<Instance *>(storage) : new Instance{};
    fixup_instance(instance);
    if (storage == nullptr) {
        instance->mashed = 0;
        instance->started = instance->playing = 1;
        instance->field_94 = 1;
    }
    return instance;
}


void unmash_instance(Instance *instance, mash_info_struct *info)
{
    auto &points = instance->points;
    if (points.data != nullptr) {
        points.data = reinterpret_cast<void **>(info->read_from_buffer(points.count * sizeof(void *), 4));
        for (unsigned i = 0; i < points.count; ++i)
            points.data[i] = info->read_from_buffer(sizeof(Point), 4);
    }
    points.end_offset = static_cast<uint32_t>(info->mash_image_ptr[0] + info->buffer_size_used[0] -
                                              reinterpret_cast<uint8_t *>(&points));
}

void fixup_instance(Instance *instance)
{
    instance->m_vtbl = reinterpret_cast<intptr_t>(instance_vtable);
    instance->mashed = 1;
    instance->owner = nullptr;
    instance->field_C = 0;
    instance->updater = nullptr;
    new (&instance->action_updaters) _std::vector<aeps::UpdateStruct *>;
    instance->effect = nullptr;
    instance->resource = nullptr;
    instance->alpha = instance->alpha_multiplier = 1.0f;
    instance->position = instance->field_4C = {0.0f, 0.0f, 0.0f};
    instance->scale = {1.0f, 1.0f, 1.0f};
    instance->field_64 = 0;
    instance->color = 0xFFFFFFFF;
    aps_native_curve_set_services({kill_particle, emit_particles, action_unmash, camera_basis});
}
bool spawn_action(Instance *instance, unsigned flags, float delay, float lifetime, float scale)
{
    if (os_developer_options::instance->get_flag(static_cast<os_developer_options::flags_t>(45)))
        return true;
    if ((flags & 0x40000000u) && !(flags & 2u))
        return false;
    if ((flags & 1u) && instance->effect != nullptr) {
        if (instance->updater != nullptr) {
            instance->updater->delay = delay;
            instance->updater->lifetime = lifetime;
        }
        return true;
    }
    if (instance->effect != nullptr && instance->owner != nullptr) {
        if (instance->updater != nullptr)
            reset_updater(instance->updater, nullptr);
        return true;
    }
    auto *effect = new Effect{};
    effect->m_vtbl = reinterpret_cast<intptr_t>(effect_vtable);
    effect->transform = identity_matrix;
    effect->scale = {1.0f, 1.0f, 1.0f};
    effect->resource = instance->resource;
    effect->playing = effect->render_seen = true;
    effect->owner = instance;
    instance->effect = reinterpret_cast<aeps::Effect *>(effect);
    initialize_groups(*effect);
    if (instance->updater != nullptr)
        instance->action_updaters.push_back(reinterpret_cast<aeps::UpdateStruct *>(instance->updater));
    auto *updater = new (tlMemAlloc(sizeof(Updater), 16, 0)) Updater{};
    updater->m_vtbl = reinterpret_cast<intptr_t>(instance->owner != nullptr ? updater_vtable : action_updater_vtable);
    updater->active = true;
    updater->owner = instance;
    updater->effect = effect;
    updater->delay = delay;
    updater->lifetime = lifetime;
    updater->scale = std::not_equal_to<float>{}(scale, 0.0f) ? scale : 1.0f;
    instance->updater = updater;
    aeps::s_activeStructs().push_back(reinterpret_cast<aeps::UpdateStruct *>(updater));
    instance->playing = 0;
    if (instance->owner != nullptr)
        visible(instance, true);
    else
        instance->playing = 1;
    if (instance->owner != nullptr)
        aeps::s_entityFx().push_back(instance->effect);
    else
        aeps::s_activeFx().push_back(instance->effect);
    auto transform = effect->transform;
    transform[3].x = instance->position.x;
    transform[3].y = instance->position.y;
    transform[3].z = instance->position.z;
    effect->transform = instance->resource->transform * transform;
    effect->scale = instance->scale;
    effect->color = (instance->color & 0xFFFFFF) |
                    (static_cast<unsigned>(instance->alpha * instance->alpha_multiplier * 255.0f) << 24);
    if (instance->flags & 2)
        effect->z_depth = instance->z_depth;
    if (instance->flags & 4)
        effect->fade = instance->fade;
    if (instance->flags & 8)
        effect->retry = std::not_equal_to<float>{}(instance->field_78, 0.0f);
    return true;
}

void spawn(Instance *instance)
{
    spawn_action(instance, 0, 0.0f, 0.0f, 1.0f);
}

void set_position(Instance *instance, const vector3d &position)
{
    instance->position = position;
    if (instance->effect != nullptr) {
        auto *effect = reinterpret_cast<Effect *>(instance->effect);
        effect->transform[3].x = position.x;
        effect->transform[3].y = position.y;
        effect->transform[3].z = position.z;
        if (effect->initialized)
            for (auto *group = effect->groups_first; group != effect->groups_last; ++group)
                group_transform(*group, effect->transform);
    }
}


void attach_instance(pfx_interface *owner, Instance *instance)
{
    auto *entry = new (tlMemAlloc(sizeof(AttachedInstance), 16, 0)) AttachedInstance{};
    entry->m_vtbl = reinterpret_cast<intptr_t>(attachment_vtable);
    entry->instance = instance;
    entry->handle = reinterpret_cast<uintptr_t>(entry);
    instance->field_C = entry->handle;
    auto &entries = *reinterpret_cast<_std::vector<AttachedInstance *> *>(&owner->field_30);
    entries.push_back(entry);
}


void set_attached_owner(Instance *instance, entity_base *owner)
{
    auto *entry = reinterpret_cast<AttachedInstance *>(instance->field_C);
    entry->owner = owner;
    if (owner != nullptr && entry->instance != nullptr)
        set_position(entry->instance, owner->get_abs_position());
}

void release_attached_instances(pfx_interface *owner)
{
    auto &entries = *reinterpret_cast<_std::vector<AttachedInstance *> *>(&owner->field_30);
    for (auto *entry : entries) {
        auto *instance = entry->instance;
        release(instance);
        instance->field_C = 0;
        if (instance->mashed)
            instance->action_updaters.~vector();
        else
            delete instance;
        destroy_attachment(entry, nullptr, 1);
    }
    entries.clear();
}

void visible(Instance *instance, bool value)
{
    if (instance->playing != static_cast<int>(value) && instance->owner != nullptr)
        aeps::DoCallback(instance->owner, 3, value ? 0 : 0x10000000);
    if (instance->effect != nullptr)
        instance->effect->render_seen = value;
    instance->playing = value;
}
void release(Instance *instance)
{
    if (instance->updater != nullptr) {
        auto *updater = reinterpret_cast<aeps::UpdateStruct *>(instance->updater);
        aeps::RemUpdater(updater);
        auto callback =
            reinterpret_cast<void(__fastcall *)(aeps::UpdateStruct *, void *, int)>(get_vfunc(updater->m_vtbl, 0));
        callback(updater, nullptr, 1);
        instance->updater = nullptr;
    }
    while (!instance->action_updaters.empty()) {
        auto *updater = instance->action_updaters.back();
        instance->action_updaters.pop_back();
        aeps::RemUpdater(updater);
        auto callback =
            reinterpret_cast<void(__fastcall *)(aeps::UpdateStruct *, void *, int)>(get_vfunc(updater->m_vtbl, 0));
        callback(updater, nullptr, 1);
    }
    if (instance->effect != nullptr) {
        if (instance->owner != nullptr)
            aeps::RemEntityFx(instance->effect);
        else
            aeps::RemFx(instance->effect);
        destroy_effect(reinterpret_cast<Effect *>(instance->effect), nullptr, 1);
        instance->effect = nullptr;
    }
}
void update_effect(Effect &effect, float elapsed)
{
    if (!effect.playing)
        return;
    if (!effect.initialized) {
        if (!effect.retry)
            return;
        initialize_groups(effect);
        if (!effect.initialized)
            return;
        effect.start_time = effect.previous_time = 0.0f;
        effect.reset = false;
    }
    const float time = elapsed - effect.start_time;
    const float dt = time - effect.previous_time;
    if (time < 0.0f || dt < 0.0f)
        return;
    for (unsigned i = 0; i < effect.resource->particles.count; ++i) {
        auto &group = effect.groups_first[i];
        auto &particle = *static_cast<ParticleTemplate *>(effect.resource->particles.data[i]);
        float group_dt = dt;
        if (group.active) {
            if (time >= particle.end) {
                group.active = false;
                group.release();
                continue;
            }
            if (group.particles == nullptr)
                return;
        } else {
            if (effect.reset || time < particle.begin || time >= particle.end)
                continue;
            group.active = true;
            group_dt = time - particle.begin;
            allocate_group(group, particle);
            group.render_info.scale = effect.scale;
            group_transform(group, effect.transform);
            group.render_info.field_34 = *reinterpret_cast<uint32_t *>(&effect.z_depth);
            group.render_info.field_38 = effect.color;
            ++effect.groups_started;
        }
        const float modifiers[3]{effect.transform[0].x, effect.transform[1].y, effect.transform[2].z};
        aps_native_actions_run(
            particle.actions, &group, time - particle.begin, std::min(group_dt, 0.3f), effect.reset, modifiers);
        finish_group(group);
    }
    effect.previous_time = time;
}
void install_entity_callbacks(void **vtable)
{
    vtable[0] = reinterpret_cast<void *>(destroy_entity);
    vtable[0x10 / 4] = reinterpret_cast<void *>(release_entity);
    vtable[0x44 / 4] = reinterpret_cast<void *>(set_entity_visible);
    vtable[0x1C0 / 4] = reinterpret_cast<void *>(set_color);
    vtable[0x1C4 / 4] = reinterpret_cast<void *>(get_color);
}
}

namespace aeps {
bool DoCallback(entity_base *owner, int callback, unsigned flags)
{
    auto *ifc = owner->my_sound_and_pfx_interface;
    if (ifc == nullptr || ifc->field_28 == nullptr)
        return false;
    auto *graph = ifc->field_28;
    const intptr_t vtable = *static_cast<intptr_t *>(graph);
    auto has = reinterpret_cast<bool(__fastcall *)(void *, void *, int)>(get_vfunc(vtable, 0x40));
    if (!has(graph, nullptr, callback))
        return false;
    ActionInfoStruct info;
    info.owner = owner;
    info.flags = flags;
    static_assert(sizeof(ActionInfoStruct) == 0x2C);
    auto perform =
        reinterpret_cast<void(__fastcall *)(void *, void *, int, sound_and_pfx_interface *, ActionInfoStruct *)>(
            get_vfunc(vtable, 0x34));
    perform(graph, nullptr, callback, ifc, &info);
    return true;
}
}
