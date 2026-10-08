#include "fe_dialog_text.h"

#include "common.h"
#include "config.h"
#include "femenusystem.h"
#include "fetext.h"
#include "femultilinetext.h"
#include "func_wrapper.h"
#include "panelfile.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"
#include "cursor.h"
#include "femanager.h"
#include "panelanim.h"
#include "panelanimfile.h"
#include "panelquad.h"
#include "pausemenusystem.h"
#include "sound_instance_id.h"
#include "variables.h"

#include <cassert>
#include <cstdio>

VALIDATE_SIZE(fe_dialog_text, 0x10C);
VALIDATE_OFFSET(fe_dialog_text, field_78, 0x78);
VALIDATE_OFFSET(fe_dialog_text, field_9C, 0x9C);

fe_dialog_text::fe_dialog_text(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    if constexpr (STANDALONE_SYSTEM) {
        m_vtbl = 0x00893E78;
        panel = nullptr;
        field_A4 = 0;
        field_100 = 1.0f;
        field_104 = 1.0f;
    } else {
        THISCALL(0x0060D570, this, a2, a3, a4);
    }
}

void fe_dialog_text::set_text(string a1)
{
    //sp_log("fe_dialog_text::set_text: %s", a1.data);
#if STANDALONE_SYSTEM
    const auto &text = *reinterpret_cast<const mString *>(&a1);
    auto *scroll = static_cast<FEMultiLineText *>(field_80);
    auto *body = static_cast<FEMultiLineText *>(field_7C);
    const auto set_body = [&text](FEMultiLineText *target) {
        target->SetButtonScale(0.8f);
        target->SetButtonColor(color32{0xFFFFFFFFu});
        target->SetTextBoxNoLocalize(FEText::string{text}, target->GetBoxWidth(), -1.0f);
    };
    set_body(scroll);
    if (scroll->field_80 <= body->line_avail_num) {
        field_A0 = false;
        set_body(body);
    } else {
        std::memcpy(field_D8, field_B8, sizeof(field_D8));
        field_A0 = true;
        field_A2 = false;
        field_A3 = false;
        if (auto *sound = reinterpret_cast<sound_instance_id *>(&field_A4)->get_sound_instance_ptr()) {
            sound->stop();
        }
        field_A1 = scroll->field_80 == 5;
    }
#else
    THISCALL(0x0060D960, this, a1);
#endif
}

void fe_dialog_text::_Load()
{
    TRACE("fe_dialog_text::Load");

    if constexpr (STANDALONE_SYSTEM) {
        assert(panel == nullptr && "Dialog text widget already loaded.");

        auto *v2 = PanelFile::UnmashPanelFile("text_box_big", static_cast<panel_layer>(1));
        this->panel = v2;
        this->field_90 = this->panel->GetAnimationPointer(0);
        this->field_94 = this->panel->GetAnimationPointer(1);
        this->field_30 = v2->GetPQ("tb_big_back_01");
        this->field_34 = v2->GetPQ("tb_big_back_02");
        this->field_38 = v2->GetPQ("tb_detail_01");
        this->field_3C = v2->GetPQ("tb_big_gradient_m_completed");
        this->field_40 = v2->GetPQ("tb_big_gradient_m_failed");
        this->field_44 = v2->GetPQ("tb_big_gradient_reward");
        this->field_48 = v2->GetPQ("tb_big_gradient_hints");
        this->field_4C = v2->GetPQ("tb_big_gradient_generic_text");
        this->field_50 = v2->GetPQ("tb_text_link_left_hilite");
        this->field_54 = v2->GetPQ("tb_text_link_center_hilite");
        this->field_58 = v2->GetPQ("tb_text_link_right_hilite");
        this->field_5C = v2->GetPQ("tb_scroll_arrow_down");
        this->field_60 = v2->GetPQ("tb_scroll_arrow_up");
        this->field_64 = v2->GetPQ("tb_scroll_bar_01");
        this->field_68 = v2->GetPQ("tb_scroll_bar_02");
        this->field_6C = v2->GetPQ("tb_scroll_spider_icon");
        this->field_70 = v2->GetPQ("tb_scroll_spider_icon_end_marker");
        this->field_74 = v2->GetPQ("tb_body_text_scroll_template");
        this->field_78 = v2->GetTextPointer("tb_header_text_MISSIONCOMPLETED");
        this->field_7C = v2->GetTextPointer("tb_body_text_generic_EMPTY");
        this->field_80 = v2->GetTextPointer("tb_body_text_scroll_EMPTY");
        this->field_84 = v2->GetTextPointer("tb_text_link_left_BLANK");
        this->field_88 = v2->GetTextPointer("tb_text_link_right_BLANK");
        this->field_8C = v2->GetTextPointer("tb_text_link_center_BLANK");

        {
            mString text{""};

            this->field_78->SetTextNoLocalize(text);

            this->field_7C->SetTextNoLocalize(text);

            this->field_80->SetTextNoLocalize(text);
        }

        bit_cast<FEMultiLineText *>(this->field_80)->SetNumLines(15);
        this->field_84->SetText(static_cast<global_text_enum>(148));
        this->field_88->SetText(static_cast<global_text_enum>(149));
        this->field_8C->SetText(static_cast<global_text_enum>(150));

        this->field_78->SetNoFlash(color32{0xFFC8C8C8});
        this->field_7C->SetNoFlash(color32{0xFFC8C8C8});
        this->field_80->SetNoFlash(color32{0xFFC8C8C8});
        this->field_84->SetNoFlash(color32{0xFFE6D03F});
        this->field_88->SetNoFlash(color32{0xFFC87238});
        this->field_8C->SetNoFlash(color32{0xFFE6D03F});

        this->field_8C->SetScale(1.2, 1.2);
        this->field_9C = 0;
        this->field_A0 = false;
        this->field_A1 = false;
        this->field_6C->GetPos(this->field_A8, this->field_B8);
        this->field_70->GetPos(this->field_A8, this->field_C8);

        std::memcpy(this->field_D8, this->field_B8, sizeof(this->field_B8));

        this->field_A2 = false;
        this->field_A3 = false;
        this->field_A4 = 0;

        this->field_E8 = this->field_80->GetY();

        float v71[4];
        float v72[4];
        this->field_74->GetPos(v71, v72);
        this->field_EC[0] = v71[0];
        this->field_EC[1] = v71[3];
        this->field_F4[0] = v72[0];
        this->field_F4[1] = v72[3];
    } else {
        THISCALL(0x00643C90, this);
    }
}

void fe_dialog_text::set_title(string a2)
{
    //sp_log("set_title: %d", a2.m_size);

#if STANDALONE_SYSTEM
    field_78->SetTextNoLocalize(*reinterpret_cast<const mString *>(&a2));
#else
    THISCALL(0x0060DB30, this, a2);
#endif
}

namespace {
void play_dialog_animation(PanelAnimFile *animation)
{
    for (int i = 0; i < animation->field_0.size(); ++i) {
        animation->field_0.m_data[i]->field_14->StartAnim(true);
    }
    animation->field_18 = 0;
    animation->field_1C = 0;
    animation->field_20 = animation->field_14;
    animation->field_28 = 0;
    animation->field_2C = false;
    animation->field_2D = true;
    animation->field_24 = 0;
}

bool dialog_accepts_input(const fe_dialog_text &dialog)
{
    return !dialog.field_90->field_2D && !dialog.field_94->field_2D &&
           !reinterpret_cast<const uint8_t *>(&dialog.field_98)[0] &&
           bit_cast<float>(dialog.field_FC) >= dialog.field_100;
}

void stop_dialog_scroll(fe_dialog_text &dialog)
{
    if (auto *sound = reinterpret_cast<sound_instance_id *>(&dialog.field_A4)->get_sound_instance_ptr()) {
        sound->stop();
    }
}

void select_dialog_answer(fe_dialog_text &dialog, bool yes)
{
    reinterpret_cast<uint8_t *>(&dialog.field_98)[2] = yes;
    dialog.field_84->SetScale(yes ? 1.2f : 1.0f, yes ? 1.2f : 1.0f);
    dialog.field_84->SetNoFlash(color32{yes ? 0xFFE6D03Fu : 0xFFC87238u});
    dialog.field_88->SetScale(yes ? 1.0f : 1.2f, yes ? 1.0f : 1.2f);
    dialog.field_88->SetNoFlash(color32{yes ? 0xFFC87238u : 0xFFE6D03Fu});
}
}

void fe_dialog_text::set_yes_no(bool enabled)
{
    reinterpret_cast<uint8_t *>(&field_98)[1] = enabled;
}

int fe_dialog_text::get_result() const
{
    const auto *state = reinterpret_cast<const uint8_t *>(&field_98);
    return state[1] ? state[2] == 0 : 2;
}

void fe_dialog_text::OnActivate()
{
    select_dialog_answer(*this, true);
    play_dialog_animation(field_90);
    reinterpret_cast<uint8_t *>(&field_98)[0] = 0;
    field_FC = 0;
    reinterpret_cast<uint8_t *>(&field_108)[0] = 0;
    reinterpret_cast<uint8_t *>(&field_108)[1] = 1;
    if (!g_cursor->field_120) {
        g_cursor->field_114 = false;
    }
    (void)sub_60B960(string_hash{"FE_BIGBOX_TEXT_IN"}, 1.0f, 1.0f);
}

void fe_dialog_text::OnDeactivate()
{
    field_100 = field_104;
}

void fe_dialog_text::OnCross(int)
{
    if (!dialog_accepts_input(*this)) {
        return;
    }
    reinterpret_cast<uint8_t *>(&field_98)[0] = 1;
    play_dialog_animation(field_94);
    stop_dialog_scroll(*this);
    (void)sub_60B960(string_hash{"FE_BIGBOX_TEXT_OUT"}, 1.0f, 1.0f);
}

void fe_dialog_text::OnTriangle(int controller)
{
    if (!dialog_accepts_input(*this)) {
        return;
    }
    reinterpret_cast<uint8_t *>(&field_98)[2] = 0;
    OnCross(controller);
}

void fe_dialog_text::OnLeft(int)
{
    auto *state = reinterpret_cast<uint8_t *>(&field_98);
    if (dialog_accepts_input(*this) && state[1] && !state[2]) {
        select_dialog_answer(*this, true);
        (void)sub_60B960(string_hash{"FE_GENERIC_LRSCROLL"}, 1.0f, 1.0f);
    }
}

void fe_dialog_text::OnRight(int)
{
    auto *state = reinterpret_cast<uint8_t *>(&field_98);
    if (dialog_accepts_input(*this) && state[1] && state[2]) {
        select_dialog_answer(*this, false);
        (void)sub_60B960(string_hash{"FE_GENERIC_LRSCROLL"}, 1.0f, 1.0f);
    }
}

void fe_dialog_text::OnUp(int)
{
    if (!field_90->field_2D && !field_94->field_2D && !reinterpret_cast<const uint8_t *>(&field_98)[0] && field_A0) {
        field_A2 = true;
        if (!reinterpret_cast<sound_instance_id *>(&field_A4)->get_sound_instance_ptr() && !field_A1) {
            const auto sound = sub_60B960(string_hash{"FE_BIGBOX_TEXT_SCROLL"}, 1.0f, 1.0f);
            std::memcpy(&field_A4, &sound, sizeof(sound));
        }
    }
}

void fe_dialog_text::OnDown(int)
{
    if (!field_90->field_2D && !field_94->field_2D && !reinterpret_cast<const uint8_t *>(&field_98)[0] && field_A0) {
        field_A3 = true;
        if (!reinterpret_cast<sound_instance_id *>(&field_A4)->get_sound_instance_ptr() && !field_A1) {
            const auto sound = sub_60B960(string_hash{"FE_BIGBOX_TEXT_SCROLL"}, 1.0f, 1.0f);
            std::memcpy(&field_A4, &sound, sizeof(sound));
        }
    }
}

void fe_dialog_text::OnAnyButtonRelease(int, int button)
{
    if (button == 4) {
        field_A2 = false;
    } else if (button == 8) {
        field_A3 = false;
    } else {
        return;
    }
    stop_dialog_scroll(*this);
}

void fe_dialog_text::OnWindowMessage(unsigned message, int, int)
{
    auto *state = reinterpret_cast<uint8_t *>(&field_98);
    auto *mouse = reinterpret_cast<uint8_t *>(&field_108);
    const auto text_hit = [](FEText *text) {
        const float x = static_cast<float>(g_cursor->field_104.x);
        const float y = static_cast<float>(g_cursor->field_104.y);
        return text->IsShown() && x > text->GetX() - 40.0f && x < text->GetX() + 40.0f && y > text->GetY() - 10.0f &&
               y < text->GetY() + 10.0f;
    };
    const auto answer_hit = [&] {
        if (state[1]) {
            if (text_hit(field_84))
                return 0;
            if (text_hit(field_88))
                return 1;
            return -1;
        }
        return text_hit(field_8C) ? 0 : -1;
    };
    if (message == WM_MOUSEMOVE) {
        if (mouse[1])
            mouse[1] = 0;
        else
            g_cursor->sub_581C60();
        if (state[1]) {
            const int answer = answer_hit();
            if (answer == 0 && !state[2])
                OnLeft(0);
            else if (answer == 1 && state[2])
                OnRight(0);
        }
        if (mouse[0]) {
            const float delta = static_cast<float>(g_cursor->field_104.y) - field_6C->GetCenterY();
            for (float &value : field_D8)
                value += delta;
            if (field_D8[0] > field_C8[0])
                std::memcpy(field_D8, field_C8, sizeof(field_D8));
            else if (field_D8[0] < field_B8[0])
                std::memcpy(field_D8, field_B8, sizeof(field_D8));
            field_6C->SetPos(field_A8, field_D8);
        }
        return;
    }
    if (message != WM_LBUTTONDOWN && message != WM_LBUTTONUP)
        return;
    if (message == WM_LBUTTONDOWN && field_A0) {
        const auto panel_hit = [](PanelQuad *quad, float radius) {
            const float x = static_cast<float>(g_cursor->field_104.x);
            const float y = static_cast<float>(g_cursor->field_104.y);
            return x > quad->GetCenterX() - radius && x < quad->GetCenterX() + radius &&
                   y > quad->GetCenterY() - radius && y < quad->GetCenterY() + radius;
        };
        if (panel_hit(field_6C, 10.0f)) {
            mouse[0] = 1;
            return;
        }
        if (panel_hit(field_60, 8.0f)) {
            OnUp(0);
            return;
        }
        if (panel_hit(field_5C, 8.0f)) {
            OnDown(0);
            return;
        }
    }
    mouse[0] = 0;
    if (field_A2)
        OnAnyButtonRelease(0, 4);
    else if (field_A3)
        OnAnyButtonRelease(0, 8);
    if (answer_hit() != -1)
        OnCross(0);
}

void fe_dialog_text::Update(Float elapsed)
{
    field_FC = bit_cast<int>(bit_cast<float>(field_FC) + float(elapsed));
    if (field_90->field_2D || field_94->field_2D) {
        panel->Update(elapsed);
        return;
    }
    if (reinterpret_cast<const uint8_t *>(&field_98)[0]) {
        g_femanager.m_pause_menu_system->Deactivate();
    }
    if (!field_A0) {
        return;
    }
    if (field_A2) {
        for (auto &value : field_D8) {
            value -= 4.0f;
        }
        if (field_D8[0] < field_B8[0]) {
            std::memcpy(field_D8, field_B8, sizeof(field_D8));
            stop_dialog_scroll(*this);
        }
    }
    if (field_A3) {
        for (auto &value : field_D8) {
            value += 4.0f;
        }
        if (field_D8[0] > field_C8[0]) {
            std::memcpy(field_D8, field_C8, sizeof(field_D8));
            stop_dialog_scroll(*this);
        }
    }
    field_6C->SetPos(field_A8, field_D8);
    const float fraction = (field_D8[0] - field_B8[0]) / (field_C8[0] - field_B8[0]);
    auto *scroll = static_cast<FEMultiLineText *>(field_80);
    unsigned width;
    unsigned height;
    const auto index = static_cast<int>(scroll->field_18);
    auto *font = index == 5 || index == 6 ? nullptr : g_femanager.field_4[index];
    nglGetStringDimensions(font, scroll->field_1C.data(), &width, &height, scroll->field_3C, scroll->field_40);
    const float total_height = (scroll->field_80 - 1) * scroll->field_74 + height;
    const float y = field_E8 - (total_height - (field_F4[1] - field_F4[0])) * fraction;
    field_80->SetPos(field_7C->GetX(), y);
}

void fe_dialog_text::Draw()
{
    const bool animating = field_90->field_2D || field_94->field_2D;
    const auto *state = reinterpret_cast<const uint8_t *>(&field_98);
    if (!animating) {
        if (state[1]) {
            field_84->SetScale(state[2] ? 1.2f : 1.0f, state[2] ? 1.2f : 1.0f);
            field_88->SetScale(state[2] ? 1.0f : 1.2f, state[2] ? 1.0f : 1.2f);
        } else {
            field_8C->SetScale(1.2f, 1.2f);
        }
    }
    nglListBeginScene(static_cast<nglSceneParamType>(1));
    if (!EnableShader) {
        matrix4x4 transform{};
        transform[0].x = 0.003125f;
        transform[1].y = 0.004166666f;
        transform[2].z = -1.0f;
        transform[3] = {-1.0f, -1.0f, 0.0f, 1.0f};
        nglSetWorldToViewMatrix({transform});
        nglSetAspectRatio(1.0f);
        nglSetOrthoMatrix(1000.0f, 10000.0f);
        nglCalculateMatrices(false);
    }
    field_30->Draw();
    field_34->Draw();
    field_38->Draw();
    PanelQuad *gradient = field_4C;
    switch (field_9C) {
    case 1:
        gradient = field_48;
        break;
    case 2:
        gradient = field_3C;
        break;
    case 3:
        gradient = field_40;
        break;
    case 4:
        gradient = field_44;
        break;
    }
    gradient->Draw();
    if (globalTextLanguage == 4) {
        field_78->SetScale(0.9f, 1.0f);
    }
    field_78->Draw();
    if (bit_cast<float>(field_FC) >= field_100) {
        if (state[1]) {
            field_84->Draw();
            field_88->Draw();
            (state[2] ? field_50 : field_58)->Draw();
        } else {
            field_8C->Draw();
            field_54->Draw();
        }
    }
    if (!field_A0 && !animating) {
        field_7C->Draw();
    }
    nglListEndScene();
    if (field_A0 && !animating) {
        nglListBeginScene(static_cast<nglSceneParamType>(1));
        nglSetClearFlags(0);
        nglSetViewport(field_EC[0], field_F4[0], field_EC[1], field_F4[1]);
        field_80->Draw();
        nglListEndScene();
        if (!field_A1) {
            field_5C->Draw();
            field_60->Draw();
            field_64->Draw();
            field_68->Draw();
            field_6C->Draw();
        }
    }
}

void fe_dialog_text_patch()
{
    {
        FUNC_ADDRESS(address, &fe_dialog_text::_Load);
        set_vfunc(0x00893E88, address);
    }

    return;
    {
        FUNC_ADDRESS(address, &fe_dialog_text::set_text);
        REDIRECT(0x0067335A, address);
    }

    {
        FUNC_ADDRESS(address, &fe_dialog_text::set_title);
        REDIRECT(0x00673379, address);
    }
}
