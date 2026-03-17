#include "fe_track_and_field.h"

#include "common.h"
#include "custom_math.h"
#include "panelfile.h"
#include "panelquad.h"
#include "variables.h"
#include "vtbl.h"

VALIDATE_SIZE(fe_track_and_field, 0x74u);

fe_track_and_field::fe_track_and_field() : field_68(0), field_6C(0), field_70(0)
{
#if STANDALONE_SYSTEM
    if constexpr (1)
#else
    if constexpr (0)
#endif
    {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00893F28;
    }

    this->field_4 = nullptr;

    for (int i = 0; i < 14; ++i) {
        this->field_8[i] = nullptr;
    }

    this->field_40 = nullptr;
    this->field_44 = nullptr;
    this->field_4C = false;
    this->field_50 = 0.0;
    this->field_5C = 1.0;
    this->field_60 = 0.30000001;
    this->field_64 = 0.69999999;
}

void fe_track_and_field::_Init()
{
    assert(this->field_4 == nullptr);

    this->field_4 = PanelFile::UnmashPanelFile("fieldandtrack", static_cast<panel_layer>(7));
    this->field_8[6] = this->field_4->GetPQ("FT_bar_center");
    this->field_8[7] = this->field_4->GetPQ("FT_bar_fast_01");
    this->field_8[8] = this->field_4->GetPQ("FT_bar_fast_02");
    this->field_8[9] = this->field_4->GetPQ("FT_bar_fast_03");
    this->field_8[10] = this->field_4->GetPQ("FT_bar_fast_04");
    this->field_8[11] = this->field_4->GetPQ("FT_bar_fast_05");
    this->field_8[12] = this->field_4->GetPQ("FT_bar_fast_06");
    this->field_8[5] = this->field_4->GetPQ("FT_bar_slow_01");
    this->field_8[4] = this->field_4->GetPQ("FT_bar_slow_02");
    this->field_8[3] = this->field_4->GetPQ("FT_bar_slow_03");
    this->field_8[2] = this->field_4->GetPQ("FT_bar_slow_04");
    this->field_8[1] = this->field_4->GetPQ("FT_bar_slow_05");
    this->field_8[0] = this->field_4->GetPQ("FT_bar_slow_06");
    this->field_8[13] = this->field_4->GetPQ("FT_frame_back");
    this->field_40 = this->field_4->GetPQ("FT_arrow_black");
    this->field_44 = this->field_4->GetPQ("FT_arrow_white");
    this->field_48 = 0.0;

    auto *v35 = this->field_4->GetPQ("FT_axis_placeholder");
    this->field_54 = v35->GetCenterX();
    this->field_58 = v35->GetCenterY();

    this->field_68 = this->field_8[6]->GetColor();
    this->field_6C = this->field_8[4]->GetColor();
    this->field_70 = this->field_8[1]->GetColor();

    this->field_4D = false;
}

void fe_track_and_field::Init()
{
    void (__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x0));
    func(this);
}

void fe_track_and_field::_Update(Float a2)
{
    if ( this->field_4 != nullptr && this->field_4C )
    {
        if ( this->field_48 > 0.0f )
        {
            auto v3 = this->field_48 - a2;
            this->field_48 = v3;
            if ( v3 < 0.0f ) {
                this->field_48 = 0.0;
            }
        }

        auto v4 = this->field_50 * half_PI;
        this->field_40->Rotate(this->field_54, this->field_58, v4, 1);
        auto v5 = this->field_50 * half_PI;
        this->field_44->Rotate(this->field_54, this->field_58, v5, 1);
        this->field_4->Update(a2);
    }
}

void fe_track_and_field::Update(Float a2)
{
    void (__fastcall *func)(void *, void *edx, Float) = CAST(func, get_vfunc(m_vtbl, 0x4));
    func(this, nullptr, a2);
}

void fe_track_and_field::_Draw()
{
    if ( this->field_4 != nullptr && this->field_4C && this->field_48 <= 0.0f )
    {
        for ( int i = 0; i < 14; ++i )
        {
            auto *v2 = this->field_8[i];
            v2->Draw();
        }

        this->field_40->Draw();
        this->field_44->Draw();
    }
}

void fe_track_and_field::Draw()
{
    void (__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x8));
    func(this);
}

void fe_track_and_field_patch()
{
    {
        FUNC_ADDRESS(address, &fe_track_and_field::_Init);
        set_vfunc(0x00893F28, address);
    }
}
