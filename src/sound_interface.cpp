#include "sound_interface.h"

#include "func_wrapper.h"
#include "common.h"
#include "trace.h"
#include "utility.h"
#include "entity.h"
#include "time_interface.h"
#include "variable.h"
#include "vtbl.h"
#include "wds.h"
#include "sound_and_pfx_interface.h"
#include "sound_manager.h"
#include "sound_source.h"
#include "oldmath_po.h"
#include <cstdlib>
#include "physical_interface.h"
#include "sound_alias_database.h"
#include <algorithm>

namespace {
struct emitter_slot {
    uint32_t handle;
    uint32_t nsl_id;
    vector3d velocity;
    vector3d position;
    _std::list<sound_instance_id> sounds;
    float pitch[5];
    uint32_t generation;
};
emitter_slot emitters[192]{};
uint16_t emitter_generation;

emitter_slot *get_emitter(uint32_t handle)
{
    const auto index = static_cast<uint16_t>(handle);
    return index < 192 && (handle >> 16) != 0 && emitters[index].handle == handle ? &emitters[index] : nullptr;
}

emitter_slot *get_emitter(sound_interface *owner)
{
    return get_emitter(static_cast<uint32_t>(owner->field_C.field_0));
}

void attach_sound(sound_interface *owner, sound_instance_id id)
{
    auto *sound = id.get_sound_instance_ptr();
    if (sound == nullptr)
        return;
    auto *emitter = get_emitter(owner);
    if (emitter == nullptr) {
        for (int i = 0; i != 192; ++i) {
            if (emitters[i].generation == 0) {
                if (++emitter_generation == 0)
                    ++emitter_generation;
                emitter = &emitters[i];
                emitter->generation = emitter_generation;
                emitter->handle = (static_cast<uint32_t>(emitter_generation) << 16) | i;
                emitter->position = reinterpret_cast<entity *>(owner->field_4)->get_abs_position();
                emitter->velocity = ZEROVEC;
                std::fill(std::begin(emitter->pitch), std::end(emitter->pitch), 1.0f);
                owner->field_C.field_0 = emitter->handle;
                break;
            }
        }
    }
    if (emitter == nullptr)
        return;
    if (emitter->sounds.size() >= 16) {
        for (auto it = emitter->sounds.begin(); it != emitter->sounds.end();) {
            if ((*it).get_sound_instance_ptr() == nullptr)
                it = emitter->sounds.erase(it);
            else
                ++it;
        }
    }
    if (emitter->sounds.size() >= 16) {
        sound->stop();
        return;
    }
    emitter->sounds.push_back(id);
    sound->emitter_id = emitter->handle;
    nslSetSourceSpatial(
        sound->source_id, &emitter->position.x, &emitter->velocity.x, sound->min_distance, sound->max_distance);
}

sound_interface_resource_info *next_group_entry(sound_interface_event_info &group)
{
    auto *list = group.available;
    if (list == nullptr) {
        list = new _std::list<sound_interface_resource_info *>;
        group.available = list;
    }
    if ((list->empty() && group.resources.size() != 0) ||
        (group.exclusion_count > 0 &&
         static_cast<int>(group.resources.size() - list->size()) > group.exclusion_count)) {
        list->clear();
        for (auto &entry : group.resources)
            list->push_back(&entry);
    }
    double total = 0.0;
    for (auto *entry : *list)
        total += entry->weight;
    const double target = static_cast<double>(std::rand()) / 32768.0 * total;
    double weight = 0.0;
    for (auto it = list->begin(); it != list->end(); ++it) {
        auto *entry = *it;
        weight += entry->weight;
        if (weight >= target) {
            if (group.exclusion_count != 0)
                list->erase(it);
            return entry;
        }
    }
    return nullptr;
}
}  // namespace
void release_native_sound_emitter(sound_interface *owner)
{
    if (auto *emitter = get_emitter(owner)) {
        for (auto id : emitter->sounds)
            if (auto *sound = id.get_sound_instance_ptr())
                sound->stop();
        emitter->sounds.clear();
        emitter->handle = emitter->generation = 0;
    }
    owner->field_C.field_0 = 0;
}

bool native_sound_emitter_count(uint32_t emitter_id, unsigned &count)
{
    auto *emitter = get_emitter(emitter_id);
    if (emitter == nullptr)
        return false;
    count = 0;
    for (auto it = emitter->sounds.begin(); it != emitter->sounds.end();) {
        if ((*it).get_sound_instance_ptr() == nullptr)
            it = emitter->sounds.erase(it);
        else {
            ++count;
            ++it;
        }
    }
    return true;
}

bool stop_first_native_emitter_sound(sound_interface *owner)
{
    auto *emitter = get_emitter(owner);
    if (emitter == nullptr)
        return false;
    while (!emitter->sounds.empty()) {
        auto id = emitter->sounds.front();
        emitter->sounds.pop_front();
        if (auto *sound = id.get_sound_instance_ptr()) {
            sound->stop();
            return true;
        }
    }
    return false;
}


void frame_advance_native_sound_emitter(sound_interface *interface_ptr, Float elapsed)
{
    interface_ptr->field_24 = std::max(0.0f, interface_ptr->field_24 - elapsed.value);
    auto *emitter = get_emitter(interface_ptr);
    if (emitter == nullptr)
        return;
    auto *owner = reinterpret_cast<entity *>(interface_ptr->field_4);
    emitter->position = owner->get_abs_position();
    auto *physical = owner->has_physical_ifc() ? owner->physical_ifc() : nullptr;
    if (physical != nullptr && !physical->field_184)
        emitter->velocity = physical->get_velocity();
    else if (elapsed.value > 0.0f)
        emitter->velocity = (emitter->position - owner->get_last_position()) * (1.0f / elapsed.value);
    else
        emitter->velocity = ZEROVEC;
    interface_ptr->field_14 = emitter->velocity;
    const float scale = owner->field_58 != nullptr ? static_cast<float>(owner->field_58->sub_4ADE50())
                                                   : g_world_ptr->time_manager.field_0;
    for (auto it = emitter->sounds.begin(); it != emitter->sounds.end();) {
        auto *sound = (*it).get_sound_instance_ptr();
        if (sound == nullptr) {
            it = emitter->sounds.erase(it);
            continue;
        }
        nslSetSourceSpatial(
            sound->source_id, &emitter->position.x, &emitter->velocity.x, sound->min_distance, sound->max_distance);
        const float alias_pitch = sound->alias != nullptr ? sound->alias->pitch : 1.0f;
        nslSetSourcePitch(sound->source_id, sound->pitch * alias_pitch * scale);
        ++it;
    }
}


sound_instance_id sound_interface::play_sound(sound_source source, float volume, float pitch, float doppler,
                                              float min_distance, float max_distance)
{
    auto id = create_native_sound_instance(22, source.wave_id, source.alias);
    if (auto *sound = id.get_sound_instance_ptr()) {
        sound->volume = volume;
        sound->pitch = pitch;
        sound->doppler = doppler;
        sound->min_distance = min_distance < 0.0f ? source.get_min_distance() : min_distance;
        sound->max_distance = max_distance < 0.0f ? source.get_max_distance() : max_distance;
        sound->play();
        attach_sound(this, id);
    }
    return id;
}

sound_instance_id sound_interface::play_sound_grp(string_hash name, float volume, float pitch, float doppler,
                                                  float min_distance, float max_distance)
{
    return play_sound_grp_at(name, nullptr, volume, pitch, doppler, min_distance, max_distance);
}

sound_instance_id sound_interface::play_sound_grp_at(string_hash name, const vector3d *position, float volume,
                                                     float pitch, float doppler, float min_distance, float max_distance,
                                                     sound_interface *emitter_owner, uint32_t instance_scope)
{
    if (field_10 == nullptr)
        return {};
    for (auto &group : field_10->events) {
        if (group.name != name)
            continue;
        auto *entry = next_group_entry(group);
        if (entry == nullptr)
            return {};
        auto source = sound_manager::get_sound_source(entry->sound);
        const float random_pitch = 1.0f + (static_cast<float>(std::rand()) / 16384.0f - 1.0f) * entry->pitch_randomness;
        auto id = create_native_sound_instance(instance_scope != 0   ? instance_scope
                                               : position == nullptr ? 23
                                                                     : 24,
                                               source.wave_id,
                                               source.alias);
        if (auto *sound = id.get_sound_instance_ptr()) {
            sound->volume = volume;
            sound->pitch = pitch * random_pitch;
            sound->doppler = doppler;
            sound->min_distance = min_distance < 0.0f ? source.get_min_distance() : min_distance;
            sound->max_distance = max_distance < 0.0f ? source.get_max_distance() : max_distance;
            sound->pitch_variation = entry->emitter_pitch_modulation;
            sound->play();
            if (position != nullptr) {
                sound->position[0] = position->x;
                sound->position[1] = position->y;
                sound->position[2] = position->z;
                nslSetSourceSpatial(
                    sound->source_id, sound->position, &ZEROVEC.x, sound->min_distance, sound->max_distance);
            } else {
                attach_sound(emitter_owner != nullptr ? emitter_owner : this, id);
            }
        }
        return id;
    }
    return {};
}

sound_instance_id sound_interface::play_terrain_sound(eTerrainSoundType type, string_hash terrain, float volume,
                                                      const vector3d *position)
{
    static constexpr const char *names[] = {"GROUND_FOOTSTEP_L",
                                            "GROUND_FOOTSTEP_R",
                                            "WALL_FOOTSTEP_L",
                                            "WALL_FOOTSTEP_R",
                                            "WALL_CRAWLSTEP_L",
                                            "WALL_CRAWLSTEP_R",
                                            "ATTACK_IMPACT",
                                            "PHYSICS_IMPACT",
                                            "ATTACH_TO_WALL",
                                            "JUMP_LAND",
                                            "BODY_BULLET_HIT",
                                            "BODY_BULLET_BLOCK"};
    if (field_10 == nullptr)
        return {};
    const auto event_hash = to_hash(names[type]);
    const std::array<uint32_t, 4> *fallback = nullptr;
    for (auto &entry : field_10->web_parameters) {
        if (entry[1] != event_hash)
            continue;
        if (entry[0] == to_hash("DEFAULT"))
            fallback = &entry;
        if (entry[0] == terrain.source_hash_code)
            return play_sound_grp_at(
                string_hash{static_cast<int>(entry[2])}, position, volume, 1.0f, 1.0f, -1.0f, -1.0f);
    }
    return fallback != nullptr
               ? play_sound_grp_at(
                     string_hash{static_cast<int>((*fallback)[2])}, position, volume, 1.0f, 1.0f, -1.0f, -1.0f)
               : sound_instance_id{};
}

sound_instance_id sound_instance::play_and_add(sound_interface *owner, string_hash sound, float volume, float pitch,
                                               float doppler, float min_distance, float max_distance)
{
    return owner->play_sound(
        sound_manager::get_sound_source(sound), volume, pitch, doppler, min_distance, max_distance);
}

VALIDATE_SIZE(sound_interface, 0x28);

sound_interface::sound_interface() {}

void sound_interface::frame_advance_all_sound_ifc(Float elapsed)
{
    TRACE("sound_interface::frame_advance_all_sound_ifc");
    static auto &interfaces = var<_std::vector<sound_interface *> *>(0x0095A6A4);
    if (interfaces == nullptr) {
        return;
    }
    for (auto *interface_ptr : *interfaces) {
        if (interface_ptr == nullptr) {
            continue;
        }
        if (interface_ptr->m_vtbl == 0) {
            continue;
        }
        auto *owner = reinterpret_cast<entity *>(interface_ptr->field_4);
        const float scale = owner != nullptr && owner->field_58 != nullptr
                                ? static_cast<float>(owner->field_58->sub_4ADE50())
                                : g_world_ptr->time_manager.field_0;
        auto *address = get_vfunc(interface_ptr->m_vtbl, 0x28);
        if (address != nullptr) {
            void(__fastcall * frame_advance)(sound_interface *, void *, Float) = CAST(frame_advance, address);
            frame_advance(interface_ptr, nullptr, Float{scale * elapsed.value});
        }
    }
}

void sound_interface_patch()
{
    REDIRECT(0x005584FA, sound_interface::frame_advance_all_sound_ifc);
}
