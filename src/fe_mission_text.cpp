#include "fe_mission_text.h"

#include "common.h"
#include "fetext.h"
#include "func_wrapper.h"
#include "panelanimfile.h"
#include "panelfile.h"
#include "panelquad.h"
#include "utility.h"
#include "variables.h"

#include <d3dx9tex.h>

VALIDATE_SIZE(fe_mission_text, 0xB8u);

VALIDATE_SIZE(fe_mission_text, 0xB8);
VALIDATE_OFFSET(fe_mission_text, detail, 0x10);
VALIDATE_OFFSET(fe_mission_text, gradient_text, 0x9C);
VALIDATE_OFFSET(fe_mission_text, field_B0, 0xB0);

fe_mission_text::fe_mission_text()
{
#if STANDALONE_SYSTEM
    if constexpr (1)
#else
    if constexpr (0)
#endif
    {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00893E70;
}

    this->field_4 = nullptr;
    this->field_B4 = false;
    this->field_18 = nullptr;
}

void fe_mission_text::Init()
{
    assert(this->field_4 == nullptr && "Mission text widget already loaded.");

    this->field_4 = PanelFile::UnmashPanelFile("text_box_small", static_cast<panel_layer>(7));

    this->field_AC = this->field_4->field_28.at(0);
    this->field_8 = this->field_4->GetPQ("tb_small_back_01");
    this->field_C = this->field_4->GetPQ("tb_small_back_02");
    this->field_10 = this->field_4->GetPQ("tb_detail_01");
    this->field_9C = this->field_4->GetPQ("tb_small_gradient_text");
    this->field_A0 = this->field_4->GetPQ("tb_small_gradient_hints");
    this->field_A4 = this->field_4->GetPQ("tb_small_gradient_reward");
    this->field_A8 = this->field_4->GetTextPointer("tb_body_text_EMPTY");

    this->field_8->TurnOn(true);
    this->field_C->TurnOn(true);
    this->field_10->TurnOn(true);
    this->field_9C->TurnOn(true);
    this->field_A0->TurnOn(true);
    this->field_A4->TurnOn(true);
    this->field_A8->TurnOn(true);

    {
        FEText::string v22{{}};
        this->field_A8->SetTextNoLocalize(v22);
    }

    this->field_A8->SetNoFlash(color32{0xC8, 0xC8, 0xC8, 0xFF});

    this->field_14 = new PanelQuad{};
    this->field_14->CopyFrom(this->field_A0);

    D3DXCreateTextureFromFileExW(g_Direct3DDevice,
                                 L"data\\packs\\back_panel.dat",
                                 0,
                                 0,
                                 1,
                                 0,
                                 D3DFMT_A8R8G8B8,
                                 D3DPOOL_MANAGED,
                                 3,
                                 3,
                                 0,
                                 nullptr,
                                 nullptr,
                                 (IDirect3DTexture9 **)&this->field_18);

    this->field_1C.DXTexture = (IDirect3DTexture9 *)this->field_18;
    this->field_14->SetTexture(&this->field_1C);
    this->field_A8->SetNumLines(4);

    this->field_B0 = 0;
    this->field_B5 = false;
    this->field_B6 = false;
}

void fe_mission_text::draw_v10()
{
    if (this->panel == nullptr) {
        return;
    }

    if (!this->shown && !this->anim->field_2D) {
        return;
    }

    this->back_1->Draw();
    this->back_2->Draw();
    this->detail->Draw();

    switch (this->field_B0) {
    case 1:
        this->gradient_hints->Draw();
        break;
    case 2:
        this->gradient_reward->Draw();
        break;
    default:
        this->gradient_text->Draw();
        break;
    }

    if (!this->anim->field_2D) {
        this->text->Draw();
    }
}

void fe_mission_text::set_flavor(int a2)
{
    this->field_B0 = a2;
}

void fe_mission_text::set_text(string a2)
{
    sp_log("fe_mission_text::set_text: %s", a2.data);

    THISCALL(0x0060D440, this, a2);
}

void fe_mission_text::SetShown(bool a2)
{
    sp_log("fe_mission_text::SetShown: %d", a2);

    THISCALL(0x0061AA00, this, a2);
}

#ifdef OPENUSM_XBPACK_V10
void fe_mission_text_v10_patch()
{
    {
        FUNC_ADDRESS(address, &fe_mission_text::draw_v10);
        REDIRECT(0x00635AAD, address);
    }
}
#endif

void fe_mission_text_patch()
{
    {
        FUNC_ADDRESS(address, &fe_mission_text::Init);
        REDIRECT(0x00647E1A, address);
    }

    {
        FUNC_ADDRESS(address, &fe_mission_text::set_text);
        REDIRECT(0x00672EBC, address);
    }

    {
        FUNC_ADDRESS(address, &fe_mission_text::SetShown);
        REDIRECT(0x00672ECB, address);
    }
}