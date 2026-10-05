#include "ambient_audio_manager.h"

#include "func_wrapper.h"
#include "trace.h"
#include "sound_instance_id.h"
#include "variable.h"

#include <list.hpp>
#include <vector.hpp>

namespace {
struct ambient_track {
    uint32_t flags;
    sound_instance_id sound;
    float fade_start;
    float fade_target;
    float field_10;
    float fade_remaining;
    float fade_duration;
    string_hash id;
};
struct ambient_track_request {
    string_hash id;
    float volume;
};
Var<_std::vector<ambient_track_request> *> ambient_requests{0x0095C860};
Var<sound_instance_id> ambient_sound{0x0095F960};
Var<_std::list<ambient_track> *> ambient_tracks{0x0095C85C};
Var<int> max_playing_tracks{0x00921D70};
}  // namespace

void ambient_audio_manager::create_inst()
{
    if constexpr (STANDALONE_SYSTEM) {
        ambient_tracks() = new _std::list<ambient_track>;
        ambient_requests() = new _std::vector<ambient_track_request>;
    } else {
        CDECL_CALL(0x0053EC10);
    }
}

void ambient_audio_manager::delete_inst()
{
    TRACE("ambient_audio_manager");

    if constexpr (STANDALONE_SYSTEM) {
        if (auto *sound = ambient_sound().get_sound_instance_ptr()) {
            sound->stop();
            ambient_sound() = sound_instance_id{0};
        }
        delete ambient_tracks();
        ambient_tracks() = nullptr;
        delete ambient_requests();
        ambient_requests() = nullptr;
    } else {
        CDECL_CALL(0x00552800);
    }
}

void ambient_audio_manager::frame_advance(Float a1)
{
    if constexpr (!STANDALONE_SYSTEM) {
        CDECL_CALL(0x00559380, a1);
    }
}

void ambient_audio_manager::reset()
{
    if constexpr (STANDALONE_SYSTEM) {
        if (ambient_tracks())
            ambient_tracks()->clear();
        if (ambient_requests())
            _std::vector<ambient_track_request>{}.swap(*ambient_requests());
    } else {
        CDECL_CALL(0x0054DF90);
    }
}

void ambient_audio_manager::set_max_playing_tracks(int count)
{
    if constexpr (!STANDALONE_SYSTEM) {
        CDECL_CALL(0x00538420, count);
        return;
    }
    max_playing_tracks() = count;
    auto &tracks = *ambient_tracks();
    for (;;) {
        int playing = 0;
        ambient_track *lowest = nullptr;
        for (auto &track : tracks) {
            if ((track.flags & 4) != 0 && (track.flags & 8) == 0) {
                ++playing;
                if (lowest == nullptr || track.fade_target < lowest->fade_target)
                    lowest = &track;
            }
        }
        if (playing <= count)
            break;
        if (lowest) {
            if (lowest->sound.get_sound_instance_ptr()) {
                lowest->flags |= 8;
                const float fraction =
                    lowest->fade_duration <= 0.0f ? 1.0f : 1.0f - lowest->fade_remaining / lowest->fade_duration;
                lowest->fade_start += fraction * (lowest->fade_target - lowest->fade_start);
                lowest->fade_target = 0.0f;
                lowest->fade_duration = 0.0f;
                lowest->fade_remaining = 0.0f;
            } else {
                lowest->flags = 16;
            }
        }
    }
}
