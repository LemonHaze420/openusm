#include "game_data_meat.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "os_developer_options.h"
#include "utility.h"

#include <cassert>

VALIDATE_SIZE(game_data_meat, 0x154);

void game_data_meat::init()
{
    if constexpr (1) {
        this->m_hero_points = 0;
        this->field_4 = 0;
        this->m_upg_impact_web_pts = 0;
        this->m_upg_hero_meter_pts = 0;
        this->field_10 = 0;
        this->field_14 = 0;
        this->field_18 = 0;
        this->m_hero_meter_level_1 = 1000;
        this->m_hero_meter_level_2 = 3000;
        this->m_hero_meter_level_3 = 7000;
        this->m_cur_hero_meter_lvl = 0;
        this->m_cur_hero_meter_pts = 0;
        this->m_opt_score_display = true;
        this->field_31 = true;
        this->field_34 = 1;
        this->field_38 = 1.0;
        this->field_3C = 0.60000002f;
        this->field_40 = 0;
        this->m_mini_map_enabled = true;
        this->m_enable_web_shot = true;
        this->field_46 = 1;
        this->field_47 = 0;
        this->m_web_gloves = true;
        this->m_adv_web_gloves = false;
        this->m_enable_impact_web = false;
        this->m_adv_impact_web = false;
        this->m_web_cowboy = true;
        this->field_4D = 1;
        this->field_4E = 1;
        this->field_4F = 1;
        this->field_50 = 1;
        this->field_51 = 1;
        this->field_52 = 1;
        this->m_difficulty = os_developer_options::instance->get_int(mString{"DIFFICULTY"});
        this->m_run_sensitivity = 0.89999998f;
        this->field_5C = 0;
        this->field_5D = 0;
        this->m_show_style_points = os_developer_options::instance->get_flag(mString{"SHOW_STYLE_POINTS"});
        this->m_hero_health = 200.0;
        this->m_hero_type = 1;
        this->m_swing_speed = 2;
        this->m_invert_camera_horz = false;
        this->m_invert_camera_vert = false;
        this->field_6E = 0;
        this->field_6F = 0;
        this->field_70 = 0;
        this->field_71 = 0;
        this->field_72 = 0;
        this->field_73 = 0;
        this->field_74 = 0;
        this->field_75 = 0;
        this->field_76 = 0;
        this->field_77 = 0;
        this->field_78 = 0;
        this->field_79 = 0;
        this->field_7A = 0;
        this->field_7B = 0;
        this->field_7C = 0;
        this->field_7D = 0;
        this->field_7E = 0;
        this->field_80 = 0;
        this->field_84 = 0;
        this->field_88 = 0;
        this->field_8C = 0;
        this->field_90 = 0;
        this->field_94 = 0;
        this->field_98 = 0;
        this->field_D8 = 42;
        this->field_E0 = 42;
        this->field_E4 = 42;
        this->field_E8 = 42;
        this->field_EC = 42;
        this->field_F0 = 42;
        this->field_F8 = 42;
        this->field_9C = 0;
        this->m_miles_web_zipping = 0;
        this->m_web_fluid_used = 0;
        this->field_A8 = 0;
        this->field_AC = 0;
        this->field_B0 = 0;
        this->field_B4 = 0;
        this->field_B8 = 0;
        this->field_BC = 0;
        this->field_C0 = 0;
        this->field_C4 = 0;
        this->field_C8 = 0;
        this->field_CC = 0;
        this->field_D0 = 0;
        this->field_D4 = 41;
        this->field_DC = 41;
        this->field_F4 = 0;
        this->field_FC = 83;
        this->field_100 = 84;
        this->field_104 = 84;
        this->field_108 = 84;
        this->field_10C = 84;
        this->field_110 = 84;
        auto *v2 = os_developer_options::instance->get_hero_name()->c_str();

        if (strlen(v2) > 32) {
            sp_log("String too big for fixed size, str is '%s'", v2);
            assert(0);
        }

        this->m_hero_name = fixedstring<8>{v2};

        this->m_district_name = fixedstring<8>{"KG"};

    } else {
        THISCALL(0x00579420, this);
    }
}

game_data_meat::game_data_meat()
{
    this->init();
}

void game_data_meat_patch()
{
    FUNC_ADDRESS(address, &game_data_meat::init);
    SET_JUMP(0x00579420, address);
}
