#include "aeps.h"

#include "camera.h"
#include "common.h"
#include "cut_scene_player.h"
#include "femanager.h"
#include "femenu.h"
#include "game.h"
#include "geometry_manager.h"
#include "memory.h"
#include "ngl.h"
#include "os_developer_options.h"
#include "pausemenusystem.h"
#include "vtbl.h"
#include "entity_base.h"
#include "event.h"
#include "sound_and_pfx_interface.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <new>

VALIDATE_SIZE(aeps::UpdateStruct, 0x1C);
VALIDATE_SIZE(aeps::Group, 0x110);
VALIDATE_OFFSET(aeps::Group, graphics, 0x98);
VALIDATE_OFFSET(aeps::Group, render_info, 0xB0);
VALIDATE_OFFSET(aeps::Effect, playing, 0x74);
VALIDATE_OFFSET(aeps::Effect, groups_first, 0x80);
VALIDATE_OFFSET(aeps::Effect, owner, 0x90);
VALIDATE_OFFSET(aeps::EffectOwner, fade_index, 0x94);

Var<_std::vector<aeps::UpdateStruct *>> aeps::s_activeStructs{0x0095B838};
Var<_std::vector<aeps::Effect *>> aeps::s_activeFx{0x0095ABF0};
Var<_std::vector<aeps::Effect *>> aeps::s_entityFx{0x0095B848};
Var<_std::vector<aeps::Group *>> aeps::s_renderList{0x0095AD10};

void aeps::DoSpideySenseEffect(entity_base *owner, float lifetime, unsigned flags)
{
    if (!owner->has_sound_and_pfx_ifc() || !owner->my_sound_and_pfx_interface->field_28)
        return;
    auto *ifc = owner->my_sound_and_pfx_interface;
    ifc->play_sound_grp(string_hash("spidey_sense"), 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
    ActionInfoStruct info;
    info.owner = owner;
    info.hash = event::ACTIVATE_SPIDEY_SENSE.source_hash_code;
    info.index = 0;
    info.flags = flags | 1;
    info.lifetime = lifetime;
    auto *graph = ifc->field_28;
    if (graph) {
        auto perform =
            reinterpret_cast<void(__fastcall *)(void *, void *, int, sound_and_pfx_interface *, ActionInfoStruct *)>(
                get_vfunc(*static_cast<std::intptr_t *>(graph), 0x34));
        perform(graph, nullptr, 5, ifc, &info);
    }
}

namespace {
Var<int> max_entities{0x0095A740};
Var<int> max_spawners{0x0095A73C};
Var<int> max_emitters{0x0095A738};
Var<int> max_particles{0x0095A744};
Var<unsigned> dev_counter{0x00921B18};

struct group_pool {
    unsigned capacity;
    aeps::Group *first;
    aeps::Group *available;
    aeps::Group *cursor;
    uint8_t *in_use;
    aeps::group_callback *callbacks;
    void **contexts;
};
Var<group_pool> pool{0x00970F04};
struct group_manager {
    aeps::Group *first;
    aeps::Group *last;
    aeps::Group *end;
};
Var<group_manager *> manager{0x00970F60};

bool option(int index)
{
    return os_developer_options::instance->get_flag(static_cast<os_developer_options::flags_t>(index)) != 0;
}

template <typename T>
void remove_pointer(_std::vector<T *> &list, T *entry)
{
    if (entry == nullptr)
        return;
    for (auto it = list.begin(); it != list.end() && *it != nullptr; ++it) {
        if (*it == entry) {
            list.erase(it);
            return;
        }
    }
}

template <typename T>
void release_vector(_std::vector<T> &list)
{
    _std::vector<T> empty;
    list.swap(empty);
}

template <typename T>
void destroy(T *object)
{
    if (object != nullptr) {
        auto callback = reinterpret_cast<void(__fastcall *)(T *, void *, int)>(get_vfunc(object->m_vtbl, 0));
        callback(object, nullptr, 1);
    }
}


void setup_frame(const matrix4x4 &view, const vector3d &position)
{
    auto &right = var<vector3d>(0x0093A4EC);
    auto &up = var<vector3d>(0x0093A4E0);
    auto &forward = var<vector3d>(0x0093A4D4);
    right = vector3d{view[0].x, view[1].x, view[2].x};
    up = vector3d{view[0].y, view[1].y, view[2].y};
    forward = vector3d{view[0].z, view[1].z, view[2].z};
    var<vector3d>(0x009711F8) = position;
    if (right.y < 0.0f || right.y > 0.0f || up.y < 0.0f || up.y > 0.0f || std::isnan(right.y) || std::isnan(up.y))
        var<float>(0x009711F4) = std::atan2(right.y, up.y);
    std::srand(query_perf_counter().LowPart);
    auto &frame = var<int>(0x00971AA4);
    if (++frame >= 2)
        frame = 0;
    var<void *>(0x00971AB4) = var<void *[2]>(0x00971A90)[frame];
    var<int>(0x00971AB0) = 0;
    var<int>(0x00971AC0) = 0;
}

void append_groups(aeps::Effect &effect)
{
    for (auto *group = effect.groups_first; group < effect.groups_last; ++group) {
        if (group->active)
            aeps::s_renderList().push_back(group);
    }
}
}  // namespace


void aeps::UpdateStruct::advance(float time)
{
    if (target == nullptr)
        return;
    elapsed += time;
    if (delay > 0.0f) {
        elapsed -= time;
        delay -= time;
        if (delay > 0.0f)
            return;
        elapsed -= delay;
    }
    if (lifetime > 0.0f) {
        lifetime -= time;
        if (lifetime <= 0.0f) {
            auto finish = reinterpret_cast<void(__fastcall *)(UpdateTarget *, void *)>(get_vfunc(target->m_vtbl, 0x24));
            finish(target, nullptr);
            target->updater = nullptr;
            RemUpdater(this);
            destroy(this);
            return;
        }
    }
    const int slot = started ? 0x28 : 0x20;
    started = true;
    auto callback = reinterpret_cast<void(__fastcall *)(UpdateTarget *, void *)>(get_vfunc(target->m_vtbl, slot));
    callback(target, nullptr);
}

void aeps::FrameAdvance(Float time)
{
    if (option(46) || option(45))
        return;
    if (dev_counter() % 60 == 0) {
        ++dev_counter();
        RefreshDevOptions();
    }
    if (!g_game_ptr->flag.game_paused || g_femanager.m_pause_menu_system->m_index == 10) {
        auto &list = s_activeStructs();
        for (unsigned i = list.size(); i != 0; --i) {
            auto *updater = list[i - 1];
            auto advance =
                reinterpret_cast<void(__fastcall *)(UpdateStruct *, void *, float, int)>(get_vfunc(updater->m_vtbl, 4));
            advance(updater, nullptr, time.value, 0);
        }
    }
}


void aeps::Group::render(nglLightContext *light_context)
{
    if (particle_count == 0 || !initialized || !active || particles == nullptr)
        return;
    render_info.format = &format;
    render_info.particles = particles;
    render_info.particle_count = particle_count;
    render_info.center = vector3d{transform[3].x, transform[3].y, transform[3].z};
    const vector3d center = (bounds_min + bounds_max) * 0.5f;
    const vector3d delta = render_info.center - center;
    const vector3d extent = bounds_max - bounds_min;
    render_info.radius = std::sqrt(extent.x * extent.x + extent.y * extent.y + extent.z * extent.z) * 0.5f +
                         std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    render_info.transform = transformed ? &transform : nullptr;
    render_info.light_context = light_context;
    auto callback = reinterpret_cast<void(__fastcall *)(GroupGraphics *, void *, GroupRenderInfo *)>(
        get_vfunc(graphics->m_vtbl, 0x1C));
    callback(graphics, nullptr, &render_info);
}

void aeps::Group::release()
{
    if (particles != nullptr)
        tlMemFree(particles);
    particles = nullptr;
    if (auxiliary != nullptr)
        tlMemFree(auxiliary);
    auxiliary = nullptr;
    auxiliary_count = auxiliary_capacity = 0;
}

void aeps::FrameSetupRenderAndThenRender()
{
    if (option(45))
        return;
    auto *player = g_cut_scene_player();
    if (player->field_E1 || player->field_E2) {
        struct pause_render_state {
            uint32_t prefix[39];
            int mode;
        };
        const auto *pause = reinterpret_cast<const pause_render_state *>(g_femanager.m_pause_menu_system->field_4[0]);
        if (pause->mode == 3)
            return;
    }
    const auto view = nglGetMatrix(NGLMTX_WORLD_TO_VIEW);
    const float far_plane = geometry_manager::PROJ_FAR_PLANE_D;
    const float distance_limit = std::min(far_plane * far_plane, 400.0f);
    const auto position = g_game_ptr->get_current_view_camera(0)->get_abs_position();
    setup_frame(view, position);
    auto &active = s_activeFx();
    for (unsigned i = active.size(); i != 0; --i) {
        auto &effect = *active[i - 1];
        if (effect.render_seen && effect.initialized) {
            if (effect.playing)
                append_groups(effect);
        } else {
            effect.render_seen = true;
        }
    }
    auto &entities = s_entityFx();
    const auto &fade_distances = var<float[16]>(0x0095BB40);
    for (unsigned i = entities.size(); i != 0; --i) {
        auto &effect = *entities[i - 1];
        if (!effect.render_seen || !effect.initialized) {
            effect.render_seen = true;
            continue;
        }
        if (!effect.playing || !effect.initialized) {
            auto &owner = *effect.owner;
            const float distance = std::min(fade_distances[owner.fade_index], distance_limit);
            const float x = effect.transform[3].x - position.x;
            const float z = effect.transform[3].z - position.z;
            if (x * x + z * z > distance)
                continue;
            if (owner.effect != nullptr)
                owner.effect->playing = true;
            owner.started = 1;
        }
        append_groups(effect);
    }
    RenderAll();
}

void aeps::RenderAll()
{
    auto &list = s_renderList();
    for (auto *group : list)
        group->render(nullptr);
    if (!list.empty())
        release_vector(list);
}

void aeps::RefreshDevOptions()
{
    const auto limit = [](int index, int normal) {
        const int value = os_developer_options::instance->get_int(static_cast<os_developer_options::ints_t>(index));
        return value == -1 || value == 0 ? normal : value;
    };
    max_entities() = limit(72, 64);
    max_spawners() = limit(73, 128);
    max_emitters() = limit(74, 400);
    max_particles() = limit(75, 4096);
}

void aeps::release_groups(Group *first, unsigned count, int reason)
{
    auto &groups = pool();
    const auto begin = static_cast<unsigned>(first - groups.first);
    unsigned end = begin + count;
    assert(end <= groups.capacity);
    while (end < groups.capacity && groups.in_use[end] == 1 && groups.callbacks[end] == nullptr)
        ++end;
    for (unsigned i = begin; i < end; ++i) {
        groups.in_use[i] = 0;
        if (groups.callbacks[i] != nullptr) {
            groups.callbacks[i](groups.contexts[i], reason);
            groups.callbacks[i] = nullptr;
        }
        groups.first[i].release();
        groups.first[i].field_0 = nullptr;
        groups.first[i].active = false;
    }
}

aeps::Group *aeps::allocate_groups(unsigned count, group_callback callback, void *context)
{
    auto &groups = pool();
    if (groups.capacity == 0 || count > groups.capacity)
        return nullptr;
    if (count == 0)
        return groups.cursor;
    const unsigned cursor = static_cast<unsigned>(groups.cursor - groups.first);
    const auto find_run = [&](int priority) -> Group * {
        for (unsigned scanned = 0; scanned < groups.capacity; ++scanned) {
            const unsigned start = (cursor + scanned) % groups.capacity;
            if (start + count > groups.capacity)
                continue;
            bool available = true;
            for (unsigned i = start; i < start + count; ++i) {
                if (groups.in_use[i] == 0)
                    continue;
                if (priority == 0 || groups.in_use[i] != 1 ||
                    (groups.callbacks[i] != nullptr && groups.callbacks[i](groups.contexts[i], priority) != 1)) {
                    available = false;
                    unsigned next = i + 1;
                    while (next < groups.capacity && groups.in_use[next] == 1 && groups.callbacks[next] == nullptr)
                        ++next;
                    scanned += next - start - 1;
                    break;
                }
            }
            if (!available)
                continue;
            if (priority != 0)
                release_groups(groups.first + start, count, 6);
            groups.cursor = start + count == groups.capacity ? groups.first : groups.first + start + count;
            groups.callbacks[start] = callback;
            groups.contexts[start] = context;
            for (unsigned i = start; i < start + count; ++i)
                groups.in_use[i] = 1;
            return groups.first + start;
        }
        return nullptr;
    };
    auto *result = find_run(0);
    if (result == nullptr)
        result = find_run(4);
    if (result == nullptr)
        result = find_run(5);
    if (callback != nullptr)
        callback(context, result == nullptr ? 1 : 2);
    return result;
}

void aeps::RemUpdater(UpdateStruct *updater)
{
    remove_pointer(s_activeStructs(), updater);
}

void aeps::RemFx(Effect *effect)
{
    remove_pointer(s_activeFx(), effect);
}

void aeps::RemEntityFx(Effect *effect)
{
    if (effect != nullptr && effect->initialized) {
        for (auto *group = effect->groups_first; group < effect->groups_last; ++group)
            group->release();
    }
    remove_pointer(s_entityFx(), effect);
}

void aeps::Reset()
{
    auto &updaters = s_activeStructs();
    for (unsigned i = updaters.size(); i != 0; --i) {
        auto *updater = updaters[i - 1];
        RemUpdater(updater);
        destroy(updater);
    }
    release_vector(updaters);
    auto &effects = s_activeFx();
    for (unsigned i = effects.size(); i != 0; --i) {
        auto *effect = effects[i - 1];
        RemFx(effect);
        destroy(effect);
    }
    release_vector(effects);
    auto &entities = s_entityFx();
    for (unsigned i = entities.size(); i != 0; --i)
        RemEntityFx(entities[i - 1]);
    release_vector(entities);
    release_vector(var<_std::vector<void *>>(0x0095AB88));
    release_vector(s_renderList());
    auto &groups = pool();
    for (unsigned i = 0; i < groups.capacity; ++i) {
        groups.in_use[i] = 0;
        groups.callbacks[i] = nullptr;
    }
}

void aeps::Destroy()
{
    Reset();
    if (manager() != nullptr) {
        if (manager()->first != nullptr)
            tlMemFree(manager()->first);
        tlMemFree(manager());
        manager() = nullptr;
    }
    auto &groups = pool();
    groups.capacity = 0;
    if (groups.first != nullptr)
        tlMemFree(groups.first);
    groups.first = nullptr;
    if (groups.in_use != nullptr)
        tlMemFree(groups.in_use);
    groups.in_use = nullptr;
    if (groups.callbacks != nullptr)
        tlMemFree(groups.callbacks);
    groups.callbacks = nullptr;
    if (groups.contexts != nullptr)
        tlMemFree(groups.contexts);
    groups.contexts = nullptr;
}

void aeps::Init()
{
    RefreshDevOptions();
    auto &groups = pool();
    if (groups.first == nullptr && max_emitters() != 0) {
        groups.capacity = max_emitters();
        groups.first = static_cast<Group *>(tlMemAlloc(groups.capacity * sizeof(Group), 16, 0));
        groups.available = groups.cursor = groups.first;
        for (unsigned i = 0; i < groups.capacity; ++i) {
            auto &group = *new (groups.first + i) Group{};
            group.transform = matrix4x4{};
            group.transform[0].x = group.transform[1].y = group.transform[2].z = group.transform[3].w = 1.0f;
            group.render_info.scale = vector3d{1.0f, 1.0f, 1.0f};
            group.render_info.field_38 = -1;
        }
        groups.in_use = static_cast<uint8_t *>(tlMemAlloc(groups.capacity, 16, 0));
        groups.callbacks = static_cast<group_callback *>(tlMemAlloc(groups.capacity * sizeof(group_callback), 16, 0));
        groups.contexts = static_cast<void **>(tlMemAlloc(groups.capacity * sizeof(void *), 16, 0));
        std::memset(groups.in_use, 0, groups.capacity);
        std::memset(groups.callbacks, 0, groups.capacity * sizeof(group_callback));
        std::memset(groups.contexts, 0, groups.capacity * sizeof(void *));
    }
    struct initializer {
        void(__cdecl *initialize)();
        initializer *next;
    };
    for (auto *entry = var<initializer *>(0x009712A0); entry != nullptr; entry = entry->next)
        entry->initialize();
    auto &identity = var<matrix4x4>(0x00971B20);
    identity = matrix4x4{};
    identity[0].x = identity[1].y = identity[2].z = identity[3].w = 1.0f;
    var<int>(0x0093AAC0) = 0x02000000;
    constexpr unsigned particle_capacity = 2000;
    var<unsigned>(0x00971AAC) = particle_capacity;
    auto &buffers = var<nglVertexBuffer *[2]>(0x00971A98);
    for (auto &buffer : buffers) {
        buffer = new (tlMemAlloc(sizeof(nglVertexBuffer), 4, 0)) nglVertexBuffer{};
        const HRESULT result = nglVertexBuffer::createIndexOrVertexBuffer(
            buffer, ResourceType::VertexBuffer, 96 * particle_capacity, 0x208, 0, D3DPOOL_DEFAULT);
        assert(SUCCEEDED(result));
    }
    auto &indices = var<nglVertexBuffer *>(0x00971AA0);
    indices = new (tlMemAlloc(sizeof(nglVertexBuffer), 4, 0)) nglVertexBuffer{};
    const HRESULT result = nglVertexBuffer::createIndexOrVertexBuffer(
        indices, ResourceType::IndexBuffer, 12 * particle_capacity, 0, 0, D3DPOOL_DEFAULT);
    assert(SUCCEEDED(result));
    uint16_t *data;
    const HRESULT locked = IDirect3DIndexBuffer9_Lock(
        indices->getIndexBuffer(), 0, 12 * particle_capacity, reinterpret_cast<void **>(&data), 0);
    assert(SUCCEEDED(locked));
    for (unsigned i = 0; i < particle_capacity; ++i) {
        const uint16_t vertex = static_cast<uint16_t>(i * 4);
        *data++ = vertex;
        *data++ = vertex + 1;
        *data++ = vertex + 2;
        *data++ = vertex + 2;
        *data++ = vertex + 1;
        *data++ = vertex + 3;
    }
    IDirect3DIndexBuffer9_Unlock(indices->getIndexBuffer());
    if (manager() == nullptr)
        manager() = new (tlMemAlloc(sizeof(group_manager), 4, 0)) group_manager{};
    var<Effect *>(0x0095A5E8) = nullptr;
    Reset();
}

void aeps_patch()
{
    REDIRECT(0x005584F4, aeps::FrameAdvance);
    REDIRECT(0x005AD2DF, aeps::Init);
}
