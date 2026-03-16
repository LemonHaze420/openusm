#include "fe_timer_widget.h"

#include "common.h"
#include "fetext.h"
#include "func_wrapper.h"
#include "game.h"
#include "os_developer_options.h"
#include "panelfile.h"
#include "variables.h"
#include "vtbl.h"

VALIDATE_SIZE(fe_timer_widget, 0x30);

fe_timer_widget::fe_timer_widget()
{
#if STANDALONE_SYSTEM
    if constexpr (1)
#else
    if constexpr (0)
#endif
    {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00894F48;
    }

    this->field_2C = 540.0;
    this->field_4 = nullptr;
    this->field_8 = nullptr;
    this->field_C = nullptr;
    this->field_10 = nullptr;
    this->field_14 = nullptr;
    this->field_18 = nullptr;
    this->field_1C = nullptr;
    this->field_24 = 0.0;
    this->field_28 = 0;
    this->field_29 = 0;
    this->field_2A = 0;
}

void fe_timer_widget::_Init()
{
    TRACE("fe_timer_widget::Init");

    if constexpr (1)
    {
        this->field_4 = PanelFile::UnmashPanelFile("timer", static_cast<panel_layer>(7));
        if ( this->field_4 != nullptr )
        {
            this->field_8  = this->field_4->GetTextPointer("timer_01");
            this->field_C  = this->field_4->GetTextPointer("timer_02");
            this->field_10 = this->field_4->GetTextPointer("timer_03");
            this->field_14 = this->field_4->GetTextPointer("timer_04");
            this->field_18 = this->field_4->GetTextPointer("timer_05");
            this->field_1C = this->field_4->GetTextPointer("timer_06");
            this->field_20 = this->field_4->field_28.at(0);

            {
                FEText::string v14 {":"};
                this->field_C->SetTextNoLocalize(v14);
            }

            {
                FEText::string v14 {";"};
                this->field_18->SetTextNoLocalize(v14);
            }

            this->field_8->SetNoColor();
            this->field_C->SetNoColor();
            this->field_10->SetNoColor();
            this->field_14->SetNoColor();
            this->field_18->SetNoColor();
            this->field_1C->SetNoColor();
        }
    } else {
        void (__fastcall *func)(void *) = CAST(func, 0x00647BA0);
        func(this);
    }
}

void fe_timer_widget::Init() {
    void (__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x4));
    func(this);
}

void fe_timer_widget::_Update(Float a2)
{
    if ( this->field_4 )
    {
        if ( this->field_28 && !g_game_ptr->flag.game_paused )
        {
            auto v3 = (double) os_developer_options::instance->get_int(mString("TIMER_WIDGET_TIME_DELTA_PERCENT")) * LARGE_EPSILON * a2;
            if ( this->field_29 )
            {
                auto v4 = v3 + this->field_24;
                this->field_24 = v4;
                if ( v4 > this->field_2C ) {
                    this->field_24 = this->field_2C;
                }
            }
            else
            {
                auto v5 = this->field_24 - v3;
                this->field_24 = v5;
                if ( v5 < 0.0f ) {
                    this->field_24 = 0.0;
                }
            }
        }
        if ( this->field_2A || (this->field_20 != nullptr && this->field_20->field_2D) )
        {
            auto v7 = (uint64_t)(this->field_24 * 10.0f);
            auto v8 = (int)v7 % 600 % 100;
            auto v9 = (int)v7 % 600 / 100;

            char Dest[32] {};

            sprintf(Dest, "%d", (int)v7 / 600);
            this->field_8->field_1C.update_guts(Dest, 1);

            sprintf(Dest, "%d", v9);
            this->field_10->field_1C.update_guts(Dest, 1);

            sprintf(Dest, "%d", v8 / 10);
            this->field_14->field_1C.update_guts(Dest, 1);

            sprintf(Dest, "%d", v8 % 10);
            this->field_1C->field_1C.update_guts(Dest, 1);

            this->field_4->Update(a2);
        }
    }
}


void fe_timer_widget::Update(Float a2) {
    void (__fastcall *func)(void *, void *edx, Float) = CAST(func, get_vfunc(m_vtbl, 0x8));
    func(this, nullptr, a2);
}

void fe_timer_widget::_Draw()
{
    if ( this->field_4 != nullptr )
    {
        if ( this->field_2A || (this->field_20 != nullptr && this->field_20->field_2D) )
        {
            this->field_8->Draw();
            this->field_C->Draw();
            this->field_10->Draw();
            this->field_14->Draw();
            this->field_18->Draw();
            this->field_1C->Draw();
        }
    }
}

void fe_timer_widget::Draw() {
    void (__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0xC));
    func(this);
}

void fe_timer_widget::_SetShown(bool a2)
{
    if constexpr (0) {
    } else {
        THISCALL(0x006280F0, this, a2);
    }
}

void fe_timer_widget::SetShown(bool a2)
{
    void (__fastcall *func)(void *, void *edx, bool) = CAST(func, get_vfunc(m_vtbl, 0x10));
    func(this, nullptr, a2);
}

void fe_timer_widget_patch()
{
    {
        FUNC_ADDRESS(address, &fe_timer_widget::_Init);
        set_vfunc(0x00894F4C, address);
    }
}
