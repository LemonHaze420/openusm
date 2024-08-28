#include "character_viewer.h"

#include "camera.h"
#include "comic_panels.h"
#include "common.h"
#include "cursor.h"
#include "cut_scene_player.h"
#include "entity.h"
#include "fe_menu_nav_bar.h"
#include "femenusystem.h"
#include "fetext.h"
#include "fileusm.h"
#include "func_wrapper.h"
#include "game.h"
#include "marky_camera.h"
#include "mission_stack_manager.h"
#include "mstring.h"
#include "oldmath_po.h"
#include "panelquad.h"
#include "resource_manager.h"
#include "return_address.h"
#include "sound_instance_id.h"
#include "string_hash.h"
#include "us_lighting.h"
#include "utility.h"
#include "variables.h"
#include "wds.h"

#include <cassert>

VALIDATE_SIZE(character_viewer, 0x300);
VALIDATE_OFFSET(character_viewer, field_8C, 0x8C);
VALIDATE_OFFSET(character_viewer, field_2A4, 0x2A4);

VALIDATE_SIZE(character_viewer::internal, 0xC);

Var<const char *[1]> character_files { 0x00937948 };

character_viewer::character_viewer(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0) {
    for (auto &v : this->field_B0) {
        v.field_8 = 0;
    }

    this->field_2C = a2;

    this->field_34 = new menu_nav_bar{};
    this->field_2E0[0] = 0;
    this->field_2FC = 0;
    this->field_2F0 = 0.30000001;
    this->field_2F8 = 1;
    this->field_38 = 0;
    this->field_3C = 0;
    this->field_40 = 0;
    this->field_44 = 0;
    this->field_48 = 0;
    this->field_4C = 0;
    this->field_50 = 0;
    this->field_54 = 0;
    this->field_58 = 0;
    this->field_5C = 0;
    this->field_60 = 0;
    this->field_64 = 0;
    this->field_68 = 0;
    this->field_6C = 0;
    this->field_70 = 0;
    this->field_74 = 0;
    this->field_78 = 0;
    this->field_7C = 0;
    this->field_80 = 0;
    this->field_84 = 0;
    this->field_88 = nullptr;
}

void character_viewer::Init() {
    THISCALL(0x00646000, this);
}

void character_viewer::set_info() {
    THISCALL(0x006142B0, this);
}

void character_viewer::update_selected(int a2) {
    if constexpr (1) {
        auto *v3 = this->field_B0;

        for (auto i = 40; i; --i) {
            auto v7 = (float) (a2 * this->field_290);

            auto *vtbl = bit_cast<std::intptr_t(*)[1]>(v3->field_0->m_vtbl);

            auto func = (*vtbl)[48];
            assert(func == 0x00616710);
            //sp_log("func = 0x%08X", func);

            v3->field_0->sub_616710(Float{0}, Float{v7});
            ++v3;
        }

        auto v5 = this->field_298;

        int idx;
        if (a2 >= 0) {
            idx = (v5 + 39) % 40;
        } else {
            idx = (v5 + 38) % 40;
        }

        auto v8 = (float) -(a2 * this->field_294);

        {
            auto &_this = this->field_B0[idx].field_0;

            auto *vtbl = bit_cast<std::intptr_t(*)[1]>(_this->m_vtbl);

            auto func = (*vtbl)[48];
            assert(func == 0x00616710);
            //sp_log("func = 0x%08X", func);

            _this->sub_616710(Float{0}, Float{v8});
        }

        this->sub_6143B0(a2);
        this->set_info();
        this->sub_63FEC0(this->field_29C);
        this->sub_63FC90(this->field_298);
        this->field_29C = this->field_298;
    } else {
        THISCALL(0x00640070, this, a2);
    }
}

void character_viewer::sub_63FEC0(int idx)
{
    if constexpr (1)
    {
        if (this->field_B0[idx].field_8)
        {
            mString a3 {character_files()[idx]};

            auto *v3 = g_cut_scene_player();
            auto *v4 = v3->field_7C;
            if (v4 != nullptr) {
                ::operator delete(v4);
            }

            v3->field_7C = nullptr;
            v3->field_80 = 0;
            v3->field_84 = 0;
            g_world_ptr->ent_mgr.destroy_entity(this->field_2DC);
            auto v6 = "ch_vwr_" + a3;

            if (mission_stack_manager::s_inst->is_pack_pushed(v6)) {
                mission_stack_manager::s_inst->pop_mission_pack_immediate(v6, v6);
            }
        }
    } else {
        THISCALL(0x0063FEC0, this, idx);
    }
}

void character_viewer::sub_6143B0(int a2) {
    if constexpr (1) {
        auto *v3 = &this->field_2A4;

        for (auto i = 7; i; --i) {
            auto v5 = v3->field_0;
            if (v3->field_0) {
                if (v5 == 1) {
                    v3->field_0 = (a2 >= 0 ? 0 : 2);
                } else if (v5 == 2) {
                    v3->field_0 = (a2 >= 0);
                }

            } else {
                v3->field_0 = (a2 >= 0) + 1;
            }

            nglTexture *tex;
            if (v3->field_0) {
                if (v3->field_0 == 1) {
                    tex = this->field_8C[4];
                } else {
                    tex = this->field_A4;
                }

            } else {
                tex = this->field_8C[2];
            }

            {
                auto &self = v3[-1].field_4;
                auto *vtbl = bit_cast<std::intptr_t(*)[1]>(self->m_vtbl);
                auto func = (*vtbl)[29];
                assert(func = 0x00616290);

                self->SetTexture(tex);

                //sp_log("func = 0x%08X", func);

                //func(self, v6);
            }

            ++v3;
        };
    } else {
        THISCALL(0x006143B0, this, a2);
    }
}

void character_viewer::OnDown(int a2) {
    sp_log("character_viewer::OnDown(): %d", a2);

    THISCALL(0x00640230, this, a2);
}

void character_viewer::OnUp([[maybe_unused]] int a2) {
    sp_log("character_viewer::OnUp(): %d", a2);

    sp_log("index = %d", field_2C->m_index);

    if constexpr (0) {
        if (--this->field_298 < 0) {
            this->field_298 = 39;
        }

        this->update_selected(1);

        static string_hash sfx_id_hash{"FE_PS_UDScroll"};

        [[maybe_unused]] sound_instance_id v4 = sub_60B960(sfx_id_hash, {1.0}, {1.0});

    } else {
        THISCALL(0x00640180, this, a2);
    }
}

void character_viewer::OnActivate()
{
    TRACE("character_viewer::OnActivate");

    if constexpr (0)
    {
        mString v34 {"unlockables_chars_costumes"};

        mission_stack_manager::s_inst->push_mission_pack_immediate(v34, v34);
        auto *v2 = resource_manager::get_best_context(RESOURCE_PARTITION_MISSION);
        resource_manager::push_resource_context(v2);

        {
            auto *vtbl = bit_cast<fastcall_call(*)[4]>(this->m_vtbl);
            void (__fastcall *func)(void *) = CAST(func, (*vtbl)[3]);
            sp_log("func = 0x%08X", func);

            func(this);
        }

        resource_manager::pop_resource_context();
        auto *v3 = this->field_34;
        v3->field_4 = "";

        v3->field_28 = 0;
        this->field_34->AddButtons(menu_nav_bar::button_type{15},
                                   menu_nav_bar::button_type{17},
                                   static_cast<global_text_enum>(3));
        auto *v4 = this->field_34;

        {
            auto *v5 = v4->field_4.c_str();
            mString v26{v5};
        }

        {
            auto *vtbl = bit_cast<fastcall_call(*)[1]>(v4->text_box->m_vtbl);

            void (__fastcall *func)(void *) = CAST(func, (*vtbl)[35]);
            sp_log("func = 0x%08X", func);

            func(v4->text_box);
        }

        {
            auto *v6 = get_msg(g_fileUSM, "RESUME");
            mString v26 {v6};
        }

        {
            auto *vtbl = bit_cast<fastcall_call(*)[1]>(v4->text_box->m_vtbl);

            void (__fastcall *func)(void *) = CAST(func, (*vtbl)[35]);

            func(v4->text_box);
        }

        auto *v7 = g_world_ptr->get_hero_ptr(0);
        if (v7 != nullptr) {
            auto &v8 = v7->get_abs_position();
            this->field_2E0[1] = v8[0];
            this->field_2E0[2] = v8[1];
            this->field_2E0[3] = v8[2];
        } else {
            auto *v9 = g_game_ptr->get_current_view_camera(0);

            auto &v11 = v9->get_abs_position();
            this->field_2E0[1] = v11[0];
            this->field_2E0[2] = v11[1];
            this->field_2E0[3] = v11[2];
        }

        this->field_2E0[2] = 350.0;
        auto *v12 = comic_panels::acquire_panel("Viewer");

        this->field_2D8 = v12;
        v12->add_camera_component("MARKY_CAM", true, 2);
        this->field_2D8->field_4C = g_cut_scene_player();

        float v28, v27;
        {
            auto *vtbl = bit_cast<fastcall_call(*)[1]>(this->field_88->m_vtbl);

            void (__fastcall *func)(void *, void *, float *, float *) = CAST(func, (*vtbl)[42]);
            sp_log("func = 0x%08X", func);

            func(this->field_88, nullptr, &v28, &v27);
        }

        auto *v13 = this->field_88;

        auto v14 = v13->GetMax();
        auto v15 = v13->GetMin();

        auto *v18 = this->field_2D8;
        v14[0] = v15[0] - v14[0];
        v14[1] = v15[1] - v14[1];

        v18->set_size(v14);
        auto *v19 = this->field_2D8;

        vector3d v33;
        v33[0] = v28;
        v33[1] = flt_88E518 - v27;
        v33[2] = 10.0;
        v19->set_loc(v33);
        this->field_2D8->field_68 = 1;
        if (!g_world_ptr->field_28.field_48)
		{
            g_game_ptr->enable_marky_cam(true, true, -1000.0, 0.0);
            auto *v20 = g_world_ptr->field_28.field_44;

            auto *v22 = g_game_ptr->get_current_view_camera(0);

            auto *vtbl = bit_cast<fastcall_call(*)[1]>(v20->m_vtbl);
            void (__fastcall *func)(void *, void *, void *) = CAST(func, (*vtbl)[165]);

            func(v20, nullptr, v22);
        }

        g_cursor->sub_5A6790();
        g_cursor->sub_5A67D0(275, 420, 355, 445);
        auto v23 = 40;
        auto v24 = 7;
        do {
            g_cursor->sub_5A67D0(50, v23, 98, v23 + 50);
            v23 += 54;
            --v24;
        } while (v24);

        this->sub_63FC90(this->field_298);
        this->field_2E0[0] = 0.0;
        this->field_2F4 = g_TOD;
        us_lighting_switch_time_of_day(0);
        auto v25 = g_world_ptr->field_158.field_8;
        this->field_28 |= 0x80u;
        this->field_2FC = v25;
        this->field_2A = -1;

    } else {
        THISCALL(0x00641EE0, this);
    }
}

void character_viewer::sub_63FC90(int a2) {
    THISCALL(0x0063FC90, this, a2);
}

void character_viewer::OnDeactivate(FEMenu *a2) {
    sp_log("character_viewer::OnDeactivate():");

    THISCALL(0x00614260, this, a2);
}

void character_viewer_patch() {
    {
        FUNC_ADDRESS(address, &character_viewer::OnActivate);
        //set_vfunc(0x008949F4, address);
    }

    {
        FUNC_ADDRESS(address, &character_viewer::OnUp);
        set_vfunc(0x00894A04, address);
    }

    {
        FUNC_ADDRESS(address, &character_viewer::OnDown);
        set_vfunc(0x00894A08, address);
    }
}
