#include "script_sound_manager.h"

#include "common.h"
#include "func_wrapper.h"
#include "string_hash.h"
#include "trace.h"
#include "variable.h"

#include "common.h"

VALIDATE_SIZE(script_sound_instance_slot, 0x40);

static Var<int> s_script_sound_instance_key_generator{0x0096BB3C};

Var<script_sound_instance_slot *> s_script_sound_instance_slots{0x0096BB38};

void script_sound_manager::create_inst()
{
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

            void (__fastcall *constructor)(void *) = CAST(constructor, 0x00670E20);
            fastcall_call destructor = CAST(destructor, 0x004ACEE0);

            auto vector_constructor = [](void *a1,
                                         uint32_t size,
                                         int count,
                                         void (__fastcall *constructor)(void *),
                                         [[maybe_unused]] fastcall_call destructor) -> void {
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
}

void script_sound_manager::delete_inst()
{
    TRACE("script_sound_manager::delete_inst");

    CDECL_CALL(0x0065F190);
}

void script_sound_manager::frame_advance(Float a1) {
    CDECL_CALL(0x0065F240, a1);
}
