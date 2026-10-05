#include "script_sound_manager.h"

#include "common.h"
#include "func_wrapper.h"
#include "string_hash.h"
#include "trace.h"
#include "variable.h"


#include "script_executable.h"
#include <cmath>

#include "common.h"

VALIDATE_SIZE(script_sound_instance_slot, 0x40);

static Var<int> s_script_sound_instance_key_generator{0x0096BB3C};

Var<script_sound_instance_slot *> s_script_sound_instance_slots{0x0096BB38};


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
            ->remove_allocated_stuff(var<int>(0x00937B94), slot.field_34);
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
