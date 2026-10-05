#include "vehicle_sounds.h"

#include "common.h"
#include "func_wrapper.h"
#include "actor.h"
#include "ai_voice_box_inode.h"
#include "base_ai_core.h"
#include "femanager.h"
#include "pausemenusystem.h"
#include "sound_and_pfx_interface.h"
#include "sound_manager.h"
#include "vtbl.h"
#include <algorithm>
#include <cstdlib>

VALIDATE_SIZE(vehicle_sounds, 0x50);

vehicle_sounds::vehicle_sounds()
    : field_4(true), field_5(true), field_6(true), field_8(0.6f), field_C(0.0f), field_10(0), field_14(0),
      field_18(0.0f), field_1C(0), field_20(0), field_24(0), field_28(0.0f), field_2C(0), field_30(false),
      field_31(false), field_34(0.0f), field_38(0), field_3C(0), field_40(0), field_44(0), field_48(0), field_4C(0)
{
#if STANDALONE_SYSTEM

    static const bool initialized = [] {
        var<bool>(0x00937FEC) = true;
        var<bool>(0x00937FED) = true;
        var<bool>(0x00937FEE) = true;
        var<bool>(0x00937FEF) = true;
        return true;
    }();
    (void)initialized;
#endif
}

void vehicle_sounds::stop_horn()
{
    if (auto *instance = field_2C.get_sound_instance_ptr()) {
        instance->stop();
        field_2C.field_0 = 0;
    }
}

void vehicle_sounds::stop_engine_sounds()
{
    sound_instance_id *ids[] = {&field_38, &field_3C, &field_40, &field_44, &field_48, &field_4C};
    for (auto *id : ids) {
        if (auto *instance = id->get_sound_instance_ptr()) {
            instance->stop();
            id->field_0 = 0;
        }
    }
}

namespace {
actor *vehicle_sound_actor(vehicle_sounds *self)
{
    auto get_actor = reinterpret_cast<actor *(__fastcall *)(vehicle_sounds *, void *)>(get_vfunc(self->m_vtbl, 0));
    return get_actor(self, nullptr);
}

sound_instance_id play_vehicle_sound(vehicle_sounds *self, string_hash group, float volume)
{
    auto *owner = vehicle_sound_actor(self);
    if (owner->has_sound_and_pfx_ifc() && self->field_4)
        return owner->my_sound_and_pfx_interface->play_sound_grp(group, volume, 1.0f, 1.0f, -1.0f, -1.0f);
    return sound_instance_id{0};
}

void manage_vehicle_loop(vehicle_sounds *self, sound_instance_id &id, string_hash group, float volume, bool enabled)
{
    if (auto *instance = id.get_sound_instance_ptr()) {
        if (enabled && volume >= LARGE_EPSILON) {
            instance->set_volume(volume);
        } else {
            instance->stop();
            id.field_0 = 0;
        }
    } else if (enabled && volume >= LARGE_EPSILON) {
        id = play_vehicle_sound(self, group, 0.0f);
    }
}
}  // namespace

void vehicle_sounds::manage_engine_sounds(Float, bool audible)
{
    const bool enabled = var<bool>(0x00937FEC) && audible;
    const float idle = field_C < 0.1f ? std::max(0.0f, std::min(1.0f - field_C * 10.0f, 1.0f)) : 0.0f;
    const float driving = 1.0f - idle;
    static const string_hash idle_group{int(to_hash("idle"))};
    static const string_hash driving_group{int(to_hash("driving"))};
    manage_vehicle_loop(this, field_3C, idle_group, idle, enabled);
    if (auto *instance = field_38.get_sound_instance_ptr(); instance && enabled && driving >= LARGE_EPSILON) {
        float phase = field_C;
        field_1C = 0;
        while (phase >= 0.5f) {
            phase -= 0.5f;
            ++field_1C;
        }
        field_8 = std::max(0.0f, std::min(phase * 2.0f, 1.0f));
        instance->set_pitch(field_8 * 0.65f + 0.6f);
    }
    manage_vehicle_loop(this, field_38, driving_group, driving, enabled);
}

void vehicle_sounds::manage_tire_sounds(Float, bool audible)
{
    const float skid = std::max(0.0f, std::min(field_18, 1.0f));
    const float volume = field_C > 0.11f ? std::min((field_C - 0.11f) * 7.142857074737549f, 1.0f) : 0.0f;
    static const string_hash rolling_group{int(to_hash("tireloop"))};
    static const string_hash corner_group{int(to_hash("tirecorner"))};
    static const string_hash skid_group{int(to_hash("tireskid"))};
    const bool tires_enabled = audible && var<bool>(0x00937FEE);
    manage_vehicle_loop(this, field_40, rolling_group, (1.0f - field_14) * volume, tires_enabled);
    manage_vehicle_loop(this, field_44, corner_group, volume * field_14, tires_enabled);
    manage_vehicle_loop(this, field_48, skid_group, skid, audible && var<bool>(0x00937FED));
}

void vehicle_sounds::set_horn(bool enabled)
{
    if (enabled) {
        if (field_28 <= 0.0f && !field_2C.get_sound_instance_ptr()) {
            field_28 = 0.0f;
            field_30 = true;
        }
    } else {
        stop_horn();
    }
}

void vehicle_sounds::handle_horn_sounds(Float time, bool)
{
    const float dt = time;
    field_34 = std::max(0.0f, std::min(field_34 + (field_31 ? dt : -dt), 1.0f));
    static const string_hash siren_group{int(to_hash("siren_long"))};
    manage_vehicle_loop(this, field_4C, siren_group, field_34, var<bool>(0x00937FEF));
    if (field_28 > 0.0f)
        return;
    if (field_2C.get_sound_instance_ptr()) {
        stop_horn();
        if (static_cast<unsigned>(rand() * (3.0 / 32768.0)) != 0) {
            field_28 = field_24;
        } else {
            const double random = rand() * (1.0 / 32768.0);
            field_24 = field_28 = (random + random - 1.0) * 1.25 + 2.0;
        }
        field_30 = true;
    } else if (field_30) {
        auto *owner = vehicle_sound_actor(this);
        if (static_cast<unsigned>(rand() * (20.0 / 32768.0)) != 0) {
            if (rand() * (1.0 / 32768.0) <= 0.75) {
                if (!owner->has_sound_and_pfx_ifc())
                    return;
                static const string_hash horn_group{int(to_hash("horn"))};

                field_2C =
                    owner->my_sound_and_pfx_interface->play_sound_grp(horn_group, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
                if (static_cast<unsigned>(rand() * (3.0 / 32768.0)) != 0) {
                    field_28 = field_20;
                } else {
                    field_20 = field_28 = (rand() * (2.0 / 32768.0) - 1.0) * 0.5 + 0.75;
                }
            }
        } else {
            auto *voice = static_cast<ai::voice_box_inode *>(
                owner->get_ai_core()->get_info_node(ai::voice_box_inode::default_id, false));
            if (!voice || !voice->can_gab())
                return;
            static const string_hash driver_group{int(to_hash("driver"))};
            voice->say_gab(driver_group, 0, 0, nullptr);
            field_28 = 10.0f;
        }
        field_30 = false;
    }
}

void vehicle_sounds::audio_advance(Float dt)
{
    bool audible = false;
    if (field_6 && field_4 && !g_femanager.m_pause_menu_system->IsDialogActivated()) {
        const auto difference = sound_manager::get_listener_position() - vehicle_sound_actor(this)->get_abs_position();
        audible = difference.length2() < 2500.0f;
    }
    field_28 = std::max(field_28 - static_cast<float>(dt), 0.0f);
    manage_tire_sounds(dt, audible);
    manage_engine_sounds(dt, audible);
    handle_horn_sounds(dt, audible);
}

void vehicle_sounds::play_stopped_sound()
{
    if (field_6 && field_4) {
        static const string_hash stopped_group{int(to_hash("stopped"))};
        play_vehicle_sound(this, stopped_group, 1.0f);
    }
}
