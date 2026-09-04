#include "fe_controller_disconnect.h"

#include "femanager.h"
#include "femultilinetext.h"
#include "func_wrapper.h"
#include "game.h"
#include "input_mgr.h"
#include "localized_string_table.h"
#include "memory.h"
#include "panelfile.h"
#include "panelquad.h"
#include "pausemenusystem.h"

bool fe_controller_disconnect::update()
{
    return (bool)CDECL_CALL(0x00629E60);
}

void fe_controller_disconnect::draw()
{
    if constexpr (1) {
        if (back_ground_quad() == nullptr) {
            back_ground_quad() = new PanelQuad{};

            auto *v2 = g_femanager.m_pause_menu_system->field_2C;

            auto *v4 = v2->GetPQ("pm_splash_dialog_box_02");
            back_ground_quad()->CopyFrom(v4);
            auto a3 = fe_controller_disconnect::back_ground_quad()->GetCenterX();
            auto a4 = fe_controller_disconnect::back_ground_quad()->GetCenterY();

            text_box() =
                new FEMultiLineText{static_cast<font_index>(1), a3, a4, 0, static_cast<panel_layer>(2), 1.0, 0, 0, {0}};

            text_box()->SetNumLines(10);
        }

        if (dialog_up()) {
            if (currently_plugged_in()) {
                text_box()->SetText(static_cast<global_text_enum>(472));
            } else {
                auto v8 = input_mgr::instance->field_58;
                int v9 = 1;
                if (v8 != -1) {
                    v9 = v8 - 999999;
                }

                char Dest[256]{};
                sprintf(Dest, g_game_ptr->field_7C->field_0->field_0[455], v9);
                auto *v10 = back_ground_quad();
                auto v11 = v10->GetMin();
                auto v12 = v11[0] - v10->GetMax()[0];
                auto v15 = -1.0f;
                int v14 = (v12 * 0.89999998f);

                mString v13{Dest};
                text_box()->SetTextBoxNoLocalize(*bit_cast<FEMultiLineText::string *>(&v13), v14, v15);
            }

            back_ground_quad()->Draw();
            text_box()->Draw();
        }

    } else {
        CDECL_CALL(0x0063A510);
    }
}

bool fe_controller_disconnect::get_currently_plugged_in()
{
    return currently_plugged_in();
}
