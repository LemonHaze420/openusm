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
#include "panelfile.h"
#include "sound_instance_id.h"
#include "string_hash.h"
#include "utility.h"
#include "variables.h"


VALIDATE_SIZE(main_menu_options, 0x110u);

main_menu_options::main_menu_options(FEMenuSystem *a2, int a3, int a4)
    : FEMenu(a2, 0, a3, a4, 8, 0)
{
    if constexpr (STANDALONE_SYSTEM)
    {
        m_vtbl = 0x008946F8;
        field_2C = 1.0f;
        field_30 = 1.25f;
        field_34 = 0.8f;
        field_38 = color32 {0xFFC87238};
        field_3C = color32 {0xFFE6D03F};
        for (auto &quad : field_40)
            quad = nullptr;
        field_A0 = nullptr;
        field_A4 = nullptr;
        field_A8 = nullptr;
        field_AC = nullptr;
        field_B0 = nullptr;
        field_B4 = nullptr;
        for (auto &quad : field_B8)
            quad = nullptr;
        for (auto &quad : field_C0)
            quad = nullptr;
        field_D0 = nullptr;
        field_D4 = nullptr;
        field_D8 = nullptr;
        field_DC = nullptr;
        field_E0 = nullptr;
        field_E4 = nullptr;
        field_E8 = nullptr;
        for (auto &text : field_EC)
            text = nullptr;
        field_104 = 0;
        field_106 = 0;
        field_108 = false;
        field_109 = false;
        field_10A = false;
        field_10C = a2;
    }
    else
    {
        THISCALL(0x006138F0, this, a2, a3, a4);
    }
}

void main_menu_options::_Init()
{
    auto *front_end = static_cast<FrontEndMenuSystem *>(field_10C);
    assert(front_end != nullptr && front_end->field_7C != nullptr);
    auto *panel = front_end->field_7C;

    struct QuadBinding {
        int index;
        const char *name;
    };
    static constexpr QuadBinding background_quads[] {
        {1, "mm_bkg_city"},
        {2, "mm_bkg_city_01"},
        {3, "mm_bkg_city_02"},
        {8, "mm_bkg_detail_01"},
        {4, "mm_bkg_detail_02"},
        {9, "mm_bkg_grey_b_01"},
        {5, "mm_bkg_grey_b_02"},
        {10, "mm_bkg_grey_b_03"},
        {11, "mm_bkg_grey_b_04"},
        {6, "mm_bkg_grey_b_05"},
        {7, "mm_bkg_grey_b_06"},
        {12, "mm_bkg_detail_light_01"},
        {13, "mm_bkg_detail_light_02"},
        {14, "mm_bkg_detail_light_03"},
        {15, "mm_bkg_detail_light_04"},
        {16, "mm_bkg_detail_light_05"},
        {17, "mm_bkg_detail_light_06"},
        {18, "mm_bkg_detail_dark_01"},
        {19, "mm_bkg_detail_dark_02"},
        {20, "mm_bkg_detail_dark_03"},
        {21, "mm_bkg_detail_dark_04"},
        {22, "mm_bkg_detail_dark_05"},
        {23, "mm_bkg_detail_dark_06"},
        {0, "mm_logo_medium"},
    };
    for (const auto &binding : background_quads)
    {
        auto *quad = panel->GetPQ(binding.name);
        assert(quad != nullptr);
        quad->TurnOn(false);
        field_40[binding.index] = quad;
    }

    struct NamedQuad {
        PanelQuad **destination;
        const char *name;
    };
    const NamedQuad named_quads[] {
        {&field_A0, "mm_bkg_grey_a_01"},
        {&field_A4, "mm_bkg_grey_a_02"},
        {&field_A8, "mm_bkg_grey_a_03"},
        {&field_AC, "mm_bkg_grey_a_04"},
        {&field_B0, "mm_bkg_grey_a_06"},
        {&field_B4, "mm_bkg_grey_a_07"},
        {&field_B8[0], "mm_options_box_hilite_a"},
        {&field_B8[1], "mm_options_box_hilite_b"},
        {&field_C0[0], "mm_textbox_frame"},
        {&field_C0[1], "mm_textbox_gradient"},
        {&field_C0[2], "mm_textbox_outline"},
        {&field_C0[3], "mm_textbox_spider"},
    };
    for (const auto &binding : named_quads)
    {
        auto *quad = panel->GetPQ(binding.name);
        assert(quad != nullptr);
        quad->TurnOn(false);
        *binding.destination = quad;
    }

    field_D0 = panel->GetTextPointer("mm_options_text_header");
    if (field_D0 != nullptr)
        field_D0->SetShown(false);

    static constexpr const char *link_names[] {
        "mm_options_text_link_01",
        "mm_options_text_link_02",
        "mm_options_text_link_03",
        "mm_options_text_link_04",
    };
    for (int i = 0; i < 4; ++i)
    {
        field_EC[i] = panel->GetTextPointer(link_names[i]);
        if (field_EC[i] != nullptr)
            field_EC[i]->SetShown(false);
    }

    auto clone_text = [](const FEText &source, float y, const char *name) {
        auto *clone = new FEText {};
        clone->field_4 = source.field_4;
        clone->field_8 = source.field_8;
        clone->field_C = source.field_C;
        clone->field_10 = source.field_10;
        clone->field_18 = source.field_18;
        clone->field_1C = source.field_1C;
        clone->field_2C = -114.0f;
        clone->field_30 = -36.0f - flt_965BDC;
        clone->field_34 = source.field_34;
        clone->field_3C = source.field_3C;
        clone->field_40 = source.field_40;
        clone->field_44 = source.field_44;
        clone->field_4C = source.field_4C;
        clone->field_50 = name;
        clone->field_60 = source.field_60;
        clone->field_64 = source.field_64;
        clone->SetPos(202.0f, y + flt_965BDC);
        return clone;
    };
    field_EC[4] = clone_text(
        *field_EC[3], 358.0f, "mm_options_text_link_06");
    field_EC[5] = clone_text(
        *field_EC[3], 385.0f, "mm_options_text_link_05");
    field_EC[5]->SetShown(false);

    field_D4 = panel->GetAnimationPointer(3);
    field_DC = panel->GetAnimationPointer(1);
    field_D8 = panel->GetAnimationPointer(4);
    field_E0 = panel->GetAnimationPointer(2);
    field_E4 = panel->GetAnimationPointer(13);
    field_E8 = panel->GetAnimationPointer(12);
}

void main_menu_options::Update(Float a3) {
    FEMenu::Update(a3);
    if (this->field_108) {
        if (!this->field_E8->field_2D) {
            auto **v3 = this->field_B8;
            int v4 = 2;
            do {
                (*v3)->TurnOn(false);
                ++v3;
                --v4;
            } while (v4);
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

void main_menu_options::OnDown(int) {
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

void main_menu_options::OnCross(int a2) {
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
                static string_hash fx_accept_hash{"FE_MO_Accept"};

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

void main_menu_options::update_highlight() {
    THISCALL(0x00623340, this);
}

void main_menu_options_patch() {
    {
        FUNC_ADDRESS(address, &main_menu_options::OnActivate);
        //set_vfunc(0x00894724, address);
    }
}
