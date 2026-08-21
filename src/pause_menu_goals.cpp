#include "pause_menu_goals.h"

#include "game.h"
#include "game_settings.h"

#include "common.h"

VALIDATE_SIZE(pause_menu_goals, 0x110u);

mString pause_menu_goals::get_element_desc(int a2)
{
    mString v6;
    int v4, v5;
    auto *v3 = g_game_ptr->gamefile;
    if (a2 != 0) {
        if (a2 == 1) {
            v4 = v3->field_340.field_108;
            v5 = v3->field_340.field_F8;
        } else if (a2 == 2) {
            v4 = v3->field_340.field_10C;
            v5 = v3->field_340.field_FC;
        } else {
            v4 = v3->field_340.field_110;
            v5 = v3->field_340.field_100;
        }
    } else {
        v4 = v3->field_340.field_104;
        v5 = v3->field_340.field_F4;
    }

    if (v5 < v4) {
        if (v5 == v4 - 1) {
            v6 = this->field_90[a2];
        } else {
            char Dest[256]{};
            sprintf(Dest, this->field_50[a2].c_str(), v4 - v5);
            mString v7{Dest};
            v6 = v7;
        }
    } else {
        v6 = this->field_D0[a2];
    }

    return v6;
}
