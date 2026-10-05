#include "web_sounds.h"

#include "actor.h"
#include "common.h"
#include "fixed_pool.h"
#include "func_wrapper.h"
#include "sound_and_pfx_interface.h"
#include "sound_interface.h"

#include <algorithm>

VALIDATE_SIZE(web_sound_params, 0x20);
VALIDATE_SIZE(web_sound, 0x20);

fixed_pool &web_sound::pool = []() -> fixed_pool & {
    auto &result = var<fixed_pool>(0x00922108);
    if constexpr (STANDALONE_SYSTEM)
        result.init(32, 64, 4, 1, 0, nullptr);
    return result;
}();

_std::list<web_sound *> *&s_web_sounds = var<_std::list<web_sound *> *>(0x0095C870);

void web_sounds_manager::create_inst()
{
    s_web_sounds = new _std::list<web_sound *>;
}

void web_sounds_manager::frame_advance(Float elapsed)
{
    for (auto it = s_web_sounds->begin(); it != s_web_sounds->end();) {
        auto *sound = *it;
        if (sound->frame_advance(elapsed)) {
            delete sound;
            it = s_web_sounds->erase(it);
        } else {
            ++it;
        }
    }
}

void web_sounds_manager::delete_inst()
{
    for (auto *sound : *s_web_sounds)
        delete sound;
    delete s_web_sounds;
    s_web_sounds = nullptr;
}

void web_sounds_manager::add_web_sound(actor *owner, const vector3d &anchor, string_hash category)
{
    auto *params = owner->my_sound_and_pfx_interface->get_web_sound_params(category);
    if (params)
        s_web_sounds->push_back(new web_sound(params, owner, anchor));
}

web_sound::web_sound(web_sound_params *parameters, actor *actor_ptr, const vector3d &anchor_position)
    : owner(actor_ptr->get_my_handle()), travel_sound(0), remaining_time(0.0f), anchor(anchor_position),
      params(parameters)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x00556D00, this, parameters, actor_ptr, &anchor_position);
        return;
    }
    auto *interface_ptr = actor_ptr->my_sound_and_pfx_interface;
    interface_ptr->play_sound_grp(params->launch_group, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
    const auto position = actor_ptr->get_abs_position();
    travel_sound =
        interface_ptr->play_sound_grp_at(params->travel_group, &position, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, nullptr, 25);
    remaining_time = params->travel_factor / (position - anchor).length();
    travel_time = remaining_time;
}

bool web_sound::frame_advance(Float elapsed)
{
    if constexpr (!STANDALONE_SYSTEM)
        return THISCALL(0x00520CC0, this, elapsed) != 0;
    remaining_time = std::max(0.0f, remaining_time - elapsed.value);
    auto *actor_ptr = static_cast<actor *>(owner.get_volatile_ptr());
    if (!actor_ptr)
        return true;
    if (auto *sound = travel_sound.get_sound_instance_ptr()) {
        const float fraction = 1.0f - remaining_time / travel_time;
        const auto position = actor_ptr->get_abs_position();
        const auto travelling_position = position + (anchor - position) * fraction;
        sound->position[0] = travelling_position.x;
        sound->position[1] = travelling_position.y;
        sound->position[2] = travelling_position.z;
        nslSetSourceSpatial(sound->source_id, sound->position, &ZEROVEC.x, sound->min_distance, sound->max_distance);
    }
    if (remaining_time <= 0.0f) {
        actor_ptr->my_sound_and_pfx_interface->play_sound_grp(params->impact_group, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
        return true;
    }
    return false;
}

void *web_sound::operator new(size_t)
{
    return pool.allocate_new_block();
}

void web_sound::operator delete(void *storage)
{
    pool.remove(storage);
}
