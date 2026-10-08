#include "script_sound_manager.h"

#include "common.h"
#include "func_wrapper.h"
#include "string_hash.h"
#include "trace.h"
#include "variable.h"
#include "script_executable.h"
#include "entity_base.h"
#include "entity_base_vhandle.h"
#include "sound_and_pfx_interface.h"
#include "sound_interface.h"
#include "sound_manager.h"
#include <cmath>

#include "common.h"

VALIDATE_SIZE(script_sound_instance_slot, 0x40);

static Var<int> s_script_sound_instance_key_generator{0x0096BB3C};

Var<script_sound_instance_slot *> s_script_sound_instance_slots{0x0096BB38};

#if STANDALONE_SYSTEM
namespace {
int native_garbage_collection_id = -1;
}
int &script_sound_manager::garbage_collection_id = native_garbage_collection_id;
#else
int &script_sound_manager::garbage_collection_id = var<int>(0x00937B94);
#endif

#if STANDALONE_SYSTEM
namespace {
void reset_slot(script_sound_instance_slot &slot)
{
    slot = {};
    slot.field_8[0] = 1.0f;
    slot.field_8[1] = 1.0f;
    slot.field_8[2] = 1.0f;
}

void remove_slot(script_sound_instance_slot &slot)
{
    if (auto *sound = slot.field_0.get_sound_instance_ptr())
        sound->stop();
    if (slot.field_30 != 0)
        reinterpret_cast<script_executable *>(slot.field_30)
            ->remove_allocated_stuff(script_sound_manager::garbage_collection_id, slot.field_34);
    reset_slot(slot);
}

void advance_volume(float *value, float elapsed)
{
    if (!(value[4] > 0.0f))
        return;
    value[4] -= elapsed;
    if (value[4] <= 0.0f) {
        value[4] = 0.0f;
        value[2] = value[1];
        return;
    }
    switch (bit_cast<int>(value[7])) {
    case 0:
        value[2] += elapsed * value[5];
        break;
    case 1:
        value[6] += elapsed * value[5];
        value[2] = (1.0f - std::sin(value[6])) * (value[1] - value[0]) + value[0];
        break;
    case 2:
        value[6] += elapsed * value[5];
        value[2] = std::sin(value[6]) * (value[1] - value[0]) + value[0];
        break;
    case 3:
        value[6] += elapsed * value[5];
        value[2] = (std::sin(value[6]) * 0.5f + 0.5f) * (value[1] - value[0]) + value[0];
        break;
    }
}
}
#endif

void script_sound_manager::create_inst()
{
#if STANDALONE_SYSTEM
    s_script_sound_instance_key_generator() = 0;
    s_script_sound_instance_slots() = new script_sound_instance_slot[128]{};
    for (int index = 0; index < 128; ++index)
        reset_slot(s_script_sound_instance_slots()[index]);
#else
    if constexpr (1) {
        s_script_sound_instance_key_generator() = 0;

        struct {
            int m_count;

            script_sound_instance_slot field_4[128];
        } *v0 = static_cast<decltype(v0)>(operator new(0x2004u));

        script_sound_instance_slot *v1 = nullptr;
        if (v0 != nullptr) {
            v1 = v0->field_4;
            v0->m_count = 128;

            void(__fastcall * constructor)(void *) = CAST(constructor, 0x00670E20);
            void(__fastcall * destructor)(void *) = CAST(destructor, 0x004ACEE0);

            auto vector_constructor = [](void *a1,
                                         uint32_t size,
                                         int count,
                                         void(__fastcall * constructor)(void *),
                                         [[maybe_unused]] void(__fastcall * destructor)(void *)) -> void {
                for (int i{0}; i < count; ++i) {
                    constructor(static_cast<int *>(a1));
                    a1 = static_cast<char *>(a1) + size;
                }
            };

            vector_constructor(&v0->field_4, 0x40, 128, constructor, destructor);
        }

        s_script_sound_instance_slots() = v1;

        float v4[8];
        v4[0] = 1.0;
        v4[1] = 1.0;
        v4[2] = 1.0;
        v4[4] = 0.0;
        v4[7] = 0.0;

        for (int i{0}; i < 128; ++i) {
            auto *v3 = &v1[i];
            v3->field_0 = 0;
            v3->field_4 = 0;
            v3->field_28 = 0;
            std::memcpy(v3->field_8, &v4, sizeof(v3->field_8));
            v3->field_2C.source_hash_code = 0;
            v3->field_30 = 0;
            v3->field_34 = 0;
            v3->field_38 = false;
            v3->field_39 = false;
            v1[i].field_3C = 0;
        }
    } else {
        CDECL_CALL(0x0065F0A0);
    }
#endif
}

void script_sound_manager::delete_inst()
{
    TRACE("script_sound_manager::delete_inst");

#if STANDALONE_SYSTEM
    for (int index = 0; index < 128; ++index)
        remove_slot(s_script_sound_instance_slots()[index]);
    delete[] s_script_sound_instance_slots();
    s_script_sound_instance_slots() = nullptr;
    s_script_sound_instance_key_generator() = 0;
#else
    CDECL_CALL(0x0065F190);
#endif
}

void script_sound_manager::frame_advance(Float a1)
{
#if STANDALONE_SYSTEM
    for (int index = 0; index < 128; ++index) {
        auto &slot = s_script_sound_instance_slots()[index];
        if (slot.field_3C == 0)
            continue;
        if (auto *sound = slot.field_0.get_sound_instance_ptr()) {
            advance_volume(slot.field_8, a1);
            float volume = slot.field_8[2];
            if (!(volume <= 1.0f))
                volume = 1.0f;
            else if (volume <= 0.0f)
                volume = 0.0f;
            sound->set_volume(volume);
            if (volume <= 0.0f)
                sound->stop();
        } else {
            remove_slot(slot);
        }
    }
#else
    CDECL_CALL(0x0065F240, a1);
#endif
}


script_sound_instance_slot *script_sound_manager::get_sound_instance(uint32_t id)
{
    const auto index = static_cast<uint16_t>(id);
    const auto generation = static_cast<uint16_t>(id >> 16);
    if (index >= 128 || generation == 0)
        return nullptr;
    auto *slot = &s_script_sound_instance_slots()[index];
    return slot->field_3C == generation ? slot : nullptr;
}


uint32_t script_sound_manager::create_sound_instance(script_executable *script, string_hash sound, bool stompable_music)
{
#if STANDALONE_SYSTEM
    int index = 0;
    while (index < 128 && s_script_sound_instance_slots()[index].field_3C != 0)
        ++index;
    if (index == 128)
        return 0;
    if (++s_script_sound_instance_key_generator() > 0xFFFF)
        s_script_sound_instance_key_generator() = 1;
    const auto id = static_cast<uint32_t>(index | (s_script_sound_instance_key_generator() << 16));
    auto &slot = s_script_sound_instance_slots()[index];
    reset_slot(slot);
    slot.field_28 = stompable_music ? 1 : 0;
    slot.field_2C = sound;
    slot.field_30 = reinterpret_cast<int>(script);
    slot.field_34 = static_cast<int>(id);
    slot.field_0 = stompable_music ? sound_manager::create_hifi_stereo_sound_instance(17, sound, true)
                                   : sound_manager::create_sound_instance(16, sound);
    slot.field_3C = s_script_sound_instance_key_generator();
    return id;
#else
    uint32_t result = 0;
    CDECL_CALL(0x0065F320, &result, script, sound, stompable_music ? 1 : 0);
    return result;
#endif
}

void script_sound_manager::release_sound_instance(uint32_t id)
{
    if (auto *slot = get_sound_instance(id)) {
#if STANDALONE_SYSTEM
        remove_slot(*slot);
#else
        THISCALL(0x0064D8C0, slot);
        slot->field_3C = 0;
#endif
    }
}


void script_sound_instance_slot::play()
{
#if STANDALONE_SYSTEM
    if (field_39)
        return;
    auto *sound = field_0.get_sound_instance_ptr();
    if (sound == nullptr)
        return;
    if (sound->state == 0)
        sound->queue();
    auto *owner = entity_base_vhandle{static_cast<uint32_t>(field_4)}.get_volatile_ptr();
    if (owner != nullptr && !field_38 && owner->has_sound_and_pfx_ifc())
        add_native_sound_to_emitter(owner->my_sound_and_pfx_interface, field_0);
    sound->play();
    field_39 = true;
#else
    THISCALL(0x0064DA00, this, 0.0f, nullptr);
#endif
}


void script_sound_instance_slot::fade_out(float duration)
{
#if STANDALONE_SYSTEM
    if (auto *sound = field_0.get_sound_instance_ptr()) {
        field_8[7] = 0.0f;
        field_8[1] = 0.0f;
        field_8[0] = sound->volume;
        if (duration <= 0.0f) {
            field_8[2] = 0.0f;
            field_8[4] = 0.0f;
        } else {
            field_8[2] = sound->volume;
            field_8[3] = duration;
            field_8[4] = duration;
            field_8[5] = -sound->volume / duration;
        }
    }
#else
    THISCALL(0x0064DC20, this, duration);
#endif
}
