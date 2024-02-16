#include "cut_scene_player.h"

#include "common.h"
#include "func_wrapper.h"
#include "input_mgr.h"
#include "nal_system.h"
#include "resource_pack_standalone.h"
#include "utility.h"

#include <cstdlib>

VALIDATE_SIZE(cut_scene_player, 360u);

cut_scene_player::cut_scene_player() {
    this->field_8 = 0;
    this->field_C = 0;
    this->field_10 = 1;
    this->field_1C = 0;
    this->field_20 = 0;
    this->field_24 = 0;
    this->field_2C = 0;
    this->field_30 = 0;
    this->field_34 = 0;
    this->field_3C = 0;
    this->field_40 = 0;
    this->field_44 = 0;
    this->field_48 = {};
    this->field_58 = {};

    auto vector_constructor = [](void *a1,
                                 uint32_t size,
                                 int count,
                                 void (__fastcall *a4)(void *),
                                 [[maybe_unused]] fastcall_call a5) -> void {
        for (int i = 0; i < count; ++i) {
            a4(static_cast<int *>(a1));
            a1 = static_cast<char *>(a1) + size;
        }
    };

    void (__fastcall *sub_682D80)(void *) = CAST(sub_682D80, 0x00682D80);
    fastcall_call sub_5E46C0 = CAST(sub_5E46C0, 0x005E46C0);
    vector_constructor(&this->field_68, 16u, 5, sub_682D80, sub_5E46C0);

    this->field_B8 = 0;
    this->field_C0 = 0;
    this->field_C4 = 0;
    this->field_C8 = 0;
    this->field_D7 = 0;
    this->field_E1 = 0;
    game_button &v2 = this->field_E4;
    v2.field_4 = input_mgr::instance()->field_58;
    v2.field_C = nullptr;
    v2.field_10 = nullptr;
    v2.m_flags = 0;
    v2.m_trigger_type = 4;
    v2.field_2C = 0.0;
    this->field_E4.set_control((game_control_t) 96);
    this->field_E4.clear();
    game_button &v3 = this->field_118;
    v3.field_4 = input_mgr::instance()->field_58;
    v3.field_C = nullptr;
    v3.field_10 = nullptr;
    v3.m_flags = 0;
    v3.m_trigger_type = 4;
    v3.field_2C = 0.0;
    this->field_118.set_control((game_control_t) 114);
    this->field_118.clear();
    this->field_14C = -1;
    this->field_154 = -1.0;
    this->field_158 = -1.0;
    this->field_164 = 0;
}

static Var<cut_scene_player> player{0x0096FEB8};

void cut_scene_player::advance_lip_syncing(Float a2) {
    THISCALL(0x00737F60, this, a2);
}

bool cut_scene_player::advance_panel_anims(Float a2) {
    return (bool) THISCALL(0x007382E0, this, a2);
}

void cut_scene_player::clean_up_finished_segment() {
    THISCALL(0x0073FFB0, this);
}

void cut_scene_player::finalize() {
    CDECL_CALL(0x0086DE50);
}

void cut_scene_player::play(cut_scene *a2) {
    THISCALL(0x00742190, this, a2);
}

void cut_scene_player::play_current_segment() {
    THISCALL(0x007414E0, this);
}

void cut_scene_player::frame_advance(Float a2) {
    //sp_log("cut_scene_player::frame_advance");

    THISCALL(0x00741EC0, this, a2);
}

bool cut_scene_player::sub_741220(Float a2) {
    return (bool) THISCALL(0x00741220, this, a2);
}

bool cut_scene_player::is_playing() {
    return this->field_E1 || this->field_E2;
}

cut_scene_player *g_cut_scene_player() {
    if constexpr (0) {
        static Var<uint32_t> dword_970020{0x00970020};
        if (!(dword_970020() & 1)) {
            dword_970020() |= 1u;
            player() = cut_scene_player{};
            atexit(&cut_scene_player::finalize);
        }

        return &player();
    } else {
        return (cut_scene_player *) CDECL_CALL(0x007411C0);
    }
}

void cut_scene_player::stop(cut_scene *a2) {
    THISCALL(0x00740660, this, a2);
}

void cut_scene_player_patch() {
    {
        FUNC_ADDRESS(address, &cut_scene_player::play_current_segment);
        REDIRECT(0x00742419, address);
    }

    {
        FUNC_ADDRESS(address, &cut_scene_player::frame_advance);
        REDIRECT(0x0054FC7A, address);
        REDIRECT(0x0054FC8D, address);
    }

    //cut_scene_player::play_current_segment
    {
        {
            FUNC_ADDRESS(address, &resource_pack_standalone::get_nfl_file_handle);
            REDIRECT(0x007416A8, address);
        }

        REDIRECT(0x007416AE, create_stream_instance);
    }
}
