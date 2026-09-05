#include "fetext.h"

#include "common.h"
#include "config.h"
#include "fetextflashinfo.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"
#include "mash_config.h"
#include "game.h"
#include "localized_string_table.h"
#include "matrix4x4.h"
#include "ngl.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"
#include "vtbl.h"

#include <algorithm>
#include <cmath>

VALIDATE_SIZE(FEText, 0x68);

FEText::FEText()
{
    if constexpr (STANDALONE_SYSTEM) {
        m_vtbl = 0x00879FE0;
        field_10 = 4;
        field_4 = 1.0f;
        field_C = 0;
        field_4C = color32 {};
        flash_info = nullptr;
    } else {
        THISCALL(0x00617360, this);
    }
}

FEText::FEText(font_index a2, global_text_enum a3, Float a4, Float a5, int a6,
               panel_layer a7, Float a8, int a9, int a10, color32 a11)
{
    if constexpr (STANDALONE_SYSTEM) {
        m_vtbl = 0x00879FE0;
        field_10 = 4;
        field_4 = 1.0f;
        field_C = 0;
        field_18 = a2;
        if (g_game_ptr != nullptr && g_game_ptr->field_7C != nullptr)
            field_1C = g_game_ptr->field_7C->lookup_localized_string(a3);
        else
            field_1C = "";
        field_34[0] = a4;
        field_34[1] = a5;
        if (a7 == static_cast<panel_layer>(8))
            field_8 = static_cast<float>(a6);
        else
            SetZvalue(static_cast<float>(a6), a7);
        field_3C = a8;
        field_40 = a8;
        field_44[0] = a8;
        field_44[1] = a8;
        field_60 = -1;
        field_4C = a11;
        flash_info = nullptr;
        field_64 = static_cast<int8_t>(a9 | a10 | 4);
        if (a11.get_alpha() != 0)
            field_64 |= 1;
    } else {
        THISCALL(0x00617500, this, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
    }
}

void FEText::_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("FEText::unmash");
    if constexpr (1) {
        a1->unmash_class_in_place(this->field_1C, this);
        a1->unmash_class_in_place(this->field_50, this);
    } else {
        void (__fastcall *func)(void *, void *, mash_info_struct *, void *) = CAST(func, get_vfunc(m_vtbl, 0x4));
        func(this, nullptr, a1, a3);
    }
}

int FEText::_get_mash_sizeof()
{
#if OPENUSM_XBOX_MASH_FORMAT && !defined(OPENUSM_XBPACK_V10)
    return 0x60;
#else
    return 0x68;
#endif
}

void FEText::Draw()
{
    if constexpr (STANDALONE_SYSTEM) {
        if (!IsShown() || field_1C == mString{""})
            return;

        auto color = field_4C;
        if ((field_64 & 8) != 0 && flash_info != nullptr)
            color = flash_info->GetColor(field_4C);

        const auto alpha = color.get_alpha();
        color.set_alpha(static_cast<uint8_t>(static_cast<double>(alpha) * field_4));
        if ((field_64 & 1) == 0)
            color = color32 {0xFFFFFFFFu};

        const auto x = field_34[0];
        const auto y = field_34[1];
        nglFont *font = g_femanager.GetFont(field_18);
        nglListAddString(font, field_1C.c_str(), x, y, GetZvalue(),
                         color32::to_int(color), field_3C, field_40);
    } else {
        THISCALL(0x00617640, this);
    }
}

void FEText::_TurnOn(bool a2)
{
    this->SetShown(a2);
}

void FEText::TurnOn(bool a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        SetShown(a2);
    } else {
        void(__fastcall *func)(void *, void *, bool) =
            CAST(func, get_vfunc(m_vtbl, 0x64));
        func(this, nullptr, a2);
    }
}

mString FEText::_GetName() const
{
    return this->field_50;
}

mString FEText::GetName() const
{
    if constexpr (STANDALONE_SYSTEM)
        return field_50;

    void(__fastcall *func)(const void *, void *, mString *out) =
        CAST(func, get_vfunc(m_vtbl, 0xB0));
    mString result {};
    func(this, nullptr, &result);
    return result;
}

void FEText::Update(Float delta_time)
{
#if STANDALONE_SYSTEM
    PanelAnimObject::Update(delta_time);
    if ((field_64 & 8) != 0 && flash_info != nullptr) {
        flash_info->field_4 += delta_time;
        flash_info->field_8 =
            std::sin(flash_info->field_4 / flash_info->field_C * 6.283185307f) * 0.5f;
    }
#else
    void(__fastcall *func)(FEText *, void *, Float) =
        CAST(func, get_vfunc(m_vtbl, 0x18));
    func(this, nullptr, delta_time);
#endif
}

void FEText::Animate(const matrix4x4 &transform, Float visibility)
{
    const bool relative = (field_10 & 1) != 0;
    if (relative) {
        field_34[0] += transform[3].x;
        field_34[1] += transform[3].y;
        field_3C *= transform[0].x;
        field_40 *= transform[1].y;
    } else {
        field_34[0] = bit_cast<float>(field_2C) + transform[3].x;
        field_34[1] = bit_cast<float>(field_30) + transform[3].y;
        field_3C = field_44[0] * transform[0].x;
        field_40 = field_44[1] * transform[1].y;
    }

    const float alpha = std::clamp(
        relative ? float(visibility) * field_4 : float(visibility), 0.0f, 1.0f);
    field_4C.set_alpha(static_cast<uint8_t>(alpha * 255.0f));
    field_10 |= 1;
}

void FEText::_SetText(global_text_enum a2)
{
    TRACE("FEText::SetText");
    if constexpr (STANDALONE_SYSTEM) {
        auto *table = g_game_ptr->field_7C;
        mString text {table->lookup_localized_string(a2)};
        SetTextNoLocalize(*bit_cast<FEText::string *>(&text));
    } else {
        void(__fastcall *func)(FEText *, void *, global_text_enum) =
            CAST(func, 0x00617760);
        func(this, nullptr, a2);
    }
}

void FEText::SetText(global_text_enum a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        _SetText(a2);
    } else {
        void (__fastcall *func)(FEText *, void *, global_text_enum) =
            CAST(func, get_vfunc(m_vtbl, 0x88));
        func(this, nullptr, a2);
    }
}


void FEText::SetPos(Float x, Float y)
{
    if constexpr (STANDALONE_SYSTEM) {
        field_34.x = x;
        field_34.y = y - flt_965BDC;
    } else {
        void (__fastcall *func)(FEText *, void *, Float, Float) =
            CAST(func, get_vfunc(m_vtbl, 0x90));
        func(this, nullptr, x, y);
    }
}

void FEText::SetNoFlash(color32 a2)
{
    this->field_4C = a2;
    this->field_64 = ((this->field_64 & 0xF7) | 1);
}

void FEText::_SetNoColor()
{
    this->field_64 &= 0xF6u;
}


void FEText::SetNoColor()
{
    if constexpr (STANDALONE_SYSTEM) {
        field_64 &= 0xF6u;
    } else {
        void(__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0xAC));
        func(this);
    }
}

void FEText::SetScale(Float a2, Float a3)
{
    if constexpr (STANDALONE_SYSTEM) {
        field_3C = a2;
        field_40 = a3;
    } else {
        float(__fastcall *func)(FEText *, void *, Float, Float) =
            CAST(func, get_vfunc(m_vtbl, 0x78));
        func(this, nullptr, a2, a3);
    }
}

void FEText::SetScale(Float a2)
{
    this->field_3C = a2;
    this->field_40 = a2;
}

struct string {
    int field_0;
    int m_size;
    char *guts;
    void *field_C;
};

void FEText::_SetTextNoLocalize(string a1)
{
    TRACE("FEText::SetTextNoLocalize");
    if constexpr (STANDALONE_SYSTEM) {
        field_1C = *reinterpret_cast<const mString *>(&a1);
    } else {
        THISCALL(0x0043C410, this, a1);
    }
}

void FEText::SetTextNoLocalize(string a1)
{
    if constexpr (STANDALONE_SYSTEM) {
        _SetTextNoLocalize(a1);
    } else {
        void(__fastcall *func)(FEText *, void *, string) =
            CAST(func, get_vfunc(m_vtbl, 0x8C));
        func(this, nullptr, a1);
    }
}

void FEText::SetX(Float a2)
{
    this->field_34[0] = a2;
}

void FEText::SetY(Float a2)
{
    this->field_34[1] = a2 - flt_965BDC;
}

bool FEText::GetFlag(int a2)
{
    return (this->field_64 & a2) != 0;
}

void FEText::AdjustForJustification(float *a2, float *a3)
{
    if constexpr (1) {
        auto *str = this->field_1C.c_str();
        nglFont *font = g_femanager.GetFont(this->field_18);

        uint32_t v13, v14;
        nglGetStringDimensions(font, bit_cast<char *>(str), &v14, &v13, this->field_3C, this->field_40);
        float v8 = v13;
        float v7 = v14;

        //sp_log("%f %f", v8, v7);

        if (this->GetFlag(32)) {
            *a2 = *a2 - v7;
        } else if (!this->GetFlag(16)) {
            *a2 = *a2 - (v7 * 0.5);
        }

        if (this->GetFlag(128)) {
            *a3 = *a3 - v8;
        } else if (!this->GetFlag(64)) {
            *a3 = *a3 - (v8 * 0.5);
        }
    } else {
        THISCALL(0x00617860, this, a2, a3);
    }
}

void FEText::SetFlash(color32 a2, color32 a3, Float a4)
{
    this->field_4C = a2;
    auto *v5 = this->flash_info;
    this->field_64 |= 9u;
    if (v5 != nullptr) {
        v5->field_0 = a3;
        this->flash_info->field_C = a4;
    } else {
        this->flash_info = new FETextFlashInfo{a3, a4};
    }
}

float FEText::GetX()
{
    if constexpr (STANDALONE_SYSTEM)
        return field_34.x;

    float(__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0xD4));
    return func(this);
}

float FEText::GetY()
{
    if constexpr (STANDALONE_SYSTEM)
        return field_34.y;

    float(__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0xD8));
    return func(this);
}

void FEText::SetNumLines(int a2)
{
    void(__fastcall * func)(void *, void *edx, int) = CAST(func, get_vfunc(m_vtbl, 0x144));

    return func(this, nullptr, a2);
}

void FEText_patch()
{
    {
        FUNC_ADDRESS(address, &FEText::_unmash);
        SET_JUMP(0x0062E540, address);
    }

    {
        FUNC_ADDRESS(address, &FEText::_get_mash_sizeof);
        set_vfunc(0x0087A02C, address);
    }
    return;

    {
        FUNC_ADDRESS(address, &FEText::SetTextNoLocalize);
        set_vfunc(0x0087A06C, address);
        set_vfunc(0x0087A17C, address);
    }


    if constexpr (0) {
        {
            FUNC_ADDRESS(address, &FEText::Update);
            SET_JUMP(0x006098D0, address);
        }

        {
            FUNC_ADDRESS(address, &FEText::Draw);
            set_vfunc(0x0087A034, address);

            void (*p_func)(nglFont *, const char *, Float, Float, Float, uint32_t, Float, Float) = &nglListAddString;
            REDIRECT(0x00617745, p_func);
        }

        {
            FUNC_ADDRESS(address, &FEText::AdjustForJustification);
            set_vfunc(0x0087A0E8, address);
        }
    }
}
