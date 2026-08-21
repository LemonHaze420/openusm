#pragma once

#include "float.hpp"
#include "ngl.h"
#include "variable.h"

#include <d3d9.h>

struct RenderState_t {
    bool m_stencilCheckEnabled;
    int field_4;
    D3DSTENCILOP field_8;
    D3DCMPFUNC field_C;
    int m_stencilFailOp;
    uint32_t m_stencilRefValue;
    uint32_t m_stencilCompareMask;
    uint32_t field_1C;
    uint8_t field_20;
    uint8_t field_21;
    int field_24;
    int field_28;
    int field_2C;
    int field_30;
    int field_34;
    int field_38;
    int field_3C;
    int field_40;
    bool field_44;
    D3DCMPFUNC field_48;
    uint32_t field_4C;
    bool field_50;
    D3DBLEND field_54;
    D3DBLEND field_58;
    uint32_t field_5C;
    D3DBLENDOP field_60;
    int field_64;
    int field_68;
    int field_6C;
    int field_70;
    bool field_74;
    D3DZBUFFERTYPE field_78;
    D3DCMPFUNC field_7C;
    int field_80;
    int field_84;
    bool field_88;
    uint32_t field_8C;
    int field_90;
    float field_94;
    float field_98;
    uint32_t field_9C;
    uint8_t field_A0;
    bool field_A1;
    int field_A4;
    uint32_t field_A8;
    D3DCULL m_cullingMode;
    int field_B0;
    int field_B4;
    int field_B8;
    int field_BC;
    int field_C0;
    int field_C4;
    int field_C8;
    uint32_t m_blend_mode;
    int field_D0;
    int field_D4;

    bool operator<(const RenderState_t &a1) const
    {
        auto *ptr0 = bit_cast<uint32_t *>(&this->m_stencilCheckEnabled);
        auto *ptr1 = bit_cast<uint32_t *>(&a1.m_stencilCheckEnabled);

        bool result = false;

        constexpr auto num = sizeof(*this) / sizeof(int);

        for (uint32_t i = 0; i < num; ++i) {
            auto &field0 = ptr0[i];
            auto &field1 = ptr1[i];

            if (field0 < field1) {
                result = true;

                break;
            }
        }

        return result;
    }

    bool operator==(const RenderState_t &a1) const
    {
        auto *ptr0 = bit_cast<uint32_t *>(&this->m_stencilCheckEnabled);
        auto *ptr1 = bit_cast<uint32_t *>(&a1.m_stencilCheckEnabled);

        bool result = true;

        constexpr auto num = sizeof(*this) / sizeof(int);

        for (uint32_t i = 0; i < num; ++i) {
            auto &field0 = ptr0[i];
            auto &field1 = ptr1[i];

            if (field0 != field1) {
                result = false;

                break;
            }
        }

        return result;
    }

    //0x00774E40
    void Init();

    void Clear();

    //0x00401BF0
    void setAlphaFunction(D3DCMPFUNC a2);

    void setDepthBuffer(D3DZBUFFERTYPE Type);

    void setDepthBufferFunction(D3DCMPFUNC func);

    void setDepthBufferWriteEnabled(bool enabled);

    //0x00401C50
    void setAlphaBlending(bool a2);

    //0x00401DA0
    void setColourBufferWriteEnabled(unsigned int a2);

    //0x00401BC0
    void setAlphaTesting(bool a2);

    //0x00774A90
    void setBlending(nglBlendModeType a2, uint32_t a3, uint32_t ref_value);

    //0x0077E2B0
    void setLighting(uint8_t a2);

    void setScissorTestEnabled(bool a2);

    void setStencilCheckEnabled(bool enabled);

    void setStencilFailOperation(int a1);

    void setStencilDepthFailOperation(int a1);

    void setStencilRefValue(uint32_t v11);

    void setSrcBlend(D3DBLEND a2);

    void setDestBlend(D3DBLEND a2);

    //0x007745E0
    void setBlendingFactor(uint32_t a2);

    //0x00401C20
    void setAlphaReferenceValue(uint32_t a2);

    //0x00401AA0
    void setStencilPassOperation(D3DSTENCILOP op);

    //0x00401AD0
    void setStencilBufferTestFunction(D3DCMPFUNC func);

    //0x00401B60
    void setStencilBufferCompareMask(unsigned int a2);

    //0x00401B90
    void setStencilBufferWriteMask(uint32_t op);

    //0x00401DD0
    void setCullingMode(D3DCULL a2);

    //0x00401C80
    void setBlendOperation(D3DBLENDOP op);

    //0x00401D40
    void setFogEnable(bool a2);

    void setFogColor(uint32_t a2);

    void sub_55DFE0(Float a2);

    void sub_55E010(Float a2);
};

extern Var<RenderState_t> g_renderState;
