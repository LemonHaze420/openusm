#include "main_menu_options.h"

#include "common.h"
#include "cursor.h"
#include "fetext.h"
#include "frontendmenusystem.h"
#include "func_wrapper.h"
#include "game.h"
#include "localized_string_table.h"
#include "panelanim.h"
#include "panelanimfile.h"
#include "panelquad.h"
#include "sound_instance_id.h"
#include "string_hash.h"
#include "utility.h"
#include "variables.h"

VALIDATE_SIZE(main_menu_options, 0x110u);

main_menu_options::main_menu_options(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    THISCALL(0x006138F0, this, a2, a3, a4);
}

void main_menu_options::Update(Float a3)
{
    FEMenu::Update(a3);
    if (this->field_108) {
        if (!this->field_E8->field_2D) {
            auto **v3 = this->field_B8;
            int v4 = 2;
            do {
                (*v3)->TurnOn(false);
                ++v3;
                --v4;
            } while (v4 > 0);
        }

        if (this->field_108 && !this->field_E4->field_2D) {
            bit_cast<FrontEndMenuSystem *>(this->field_10C)->GoNextState();
        }
    }

    if (this->field_109 && this->field_E4->field_18 / this->field_E4->field_20 > 0.66000003f) {
        auto **v5 = this->field_B8;
        int v6 = 2;
        do {
            (*v5)->TurnOn(true);
            ++v5;
            --v6;
        } while (v6);

        auto *v7 = this->field_E8;
        for (uint16_t i = 0; i < v7->field_0.size(); ++i) {
            auto *v9 = v7->field_0.m_data[i]->field_14;
            v9->StartAnim(true);
        }

        v7->field_18 = 0.0;
        v7->field_1C = 0.0;
        v7->field_20 = v7->field_14;
        v7->field_28 = 0;
        v7->field_2C = 0;
        v7->field_2D = 1;
        v7->field_24 = 0;
        this->field_109 = false;
    }

    if (this->field_D8->field_2D) {
        auto *v11 = this->field_EC[4];
        auto v13 = this->field_EC[3]->GetX();
        v11->SetX(v13);

        auto *v12 = this->field_EC[5];
        auto v14 = this->field_EC[3]->GetX();
        v12->SetX(v14);
    }
}

void main_menu_options::OnActivate()
{
    {
        auto &table = g_game_ptr->field_7C;

        sp_log("%s", table->lookup_localized_string(static_cast<global_text_enum>(259)));
        //assert(0);
    }

    THISCALL(0x0062CE60, this);
}

void main_menu_options::OnDown(int)
{
    if (!this->field_E4->field_2D) {
        static string_hash fx_scroll_hash{"FE_MO_UDScroll"};

        [[maybe_unused]] sound_instance_id v5 = sub_60B960(fx_scroll_hash, 1.0, 1.0);
        auto v3 = this->field_104;
        this->field_106 = v3++;
        this->field_104 = v3;
        if (v3 == 6) {
            this->field_104 = 0;
        }

        this->update_highlight();
    }
}

void main_menu_options::OnCross(int a2)
{
    if constexpr (1) {
        auto v3 = this->field_104;
        if (v3 || !this->field_10A) {
            if (v3 == 5) {
                auto *v4 = g_cursor;
                dword_922908 = 2;
                if (!g_cursor->field_120) {
                    g_cursor->field_114 = false;
                    v4 = g_cursor;
                }

                v4->field_120 = true;
                byte_922994 = true;
            } else if (!this->field_E4->field_2D) {
                static string_hash fx_accept_hash{int(to_hash("FE_MO_Accept"))};

                [[maybe_unused]] sound_instance_id v15 = sub_60B960(fx_accept_hash, 1.0, 1.0);

                auto *v5 = this->field_E4;
                for (uint16_t i = 0; i < v5->field_0.size(); ++i) {
                    auto *v7 = v5->field_0.m_data[i]->field_14;
                    v7->StartAnim(true);
                }

                v5->field_18 = 0.0;
                v5->field_1C = 0.0;
                v5->field_20 = v5->field_14;
                v5->field_28 = 0;
                v5->field_2C = 0;
                v5->field_2D = 1;
                v5->field_24 = 1;

                auto *v9 = this->field_E8;
                for (uint16_t j = 0; j < v9->field_0.size(); ++j) {
                    auto *v11 = v9->field_0.m_data[j]->field_14;
                    v11->StartAnim(true);
                }

                v9->field_18 = 0.0;
                v9->field_1C = 0.0;
                v9->field_20 = v9->field_14;
                v9->field_28 = 0;
                v9->field_2C = 0;
                v9->field_2D = 1;
                v9->field_24 = 0;
                this->field_108 = 1;

                if (auto v13 = this->field_104; v13 == 0 || v13 == 1) {
                    sub_582A30();
                }
            }
        }

    } else {
        THISCALL(0x006236C0, this, a2);
    }
}

void main_menu_options::update_highlight()
{
    THISCALL(0x00623340, this);
}

void main_menu_options_patch()
{
    {
        FUNC_ADDRESS(address, &main_menu_options::OnActivate);
        //set_vfunc(0x00894724, address);
    }
}
