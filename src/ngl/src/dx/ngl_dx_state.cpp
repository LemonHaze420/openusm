#include "ngl_dx_state.h"

#include "common.h"
#include "func_wrapper.h"
#include "variables.h"

VALIDATE_SIZE(RenderState_t, 0xD8);
VALIDATE_OFFSET(RenderState_t, m_cullingMode, 0xAC);

Var<RenderState_t> g_renderState{0x009739A0};

void RenderState_t::Init() {
    THISCALL(0x00774E40, this);
}

void RenderState_t::Clear()
{
    THISCALL(0x00774610, this);
}

void RenderState_t::setStencilCheckEnabled(bool enabled)
{
    if (g_renderState().m_stencilCheckEnabled != enabled) {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_STENCILENABLE, enabled);
        g_renderState().m_stencilCheckEnabled = enabled;
    }
}

void RenderState_t::setStencilFailOperation(int a1) {
    if (g_renderState().m_stencilFailOp != a1) {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_STENCILFAIL, a1);
        g_renderState().m_stencilFailOp = a1;
    }
}

void RenderState_t::setStencilDepthFailOperation(int a1) {
    if (g_renderState().field_4 != a1) {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_STENCILZFAIL, a1);
        g_renderState().field_4 = a1;
    }
}

void RenderState_t::setStencilRefValue(uint32_t v11)
{
    if (g_renderState().m_stencilRefValue != v11) {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_STENCILREF, v11);
        g_renderState().m_stencilRefValue = v11;
    }
}

void RenderState_t::setSrcBlend(D3DBLEND a2)
{
    if ( this->field_54 != a2 )
    {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_SRCBLEND, a2);
        this->field_54 = a2;
    }
}

void RenderState_t::setDestBlend(D3DBLEND a2)
{
    if ( this->field_58 != a2 )
    {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_DESTBLEND, a2);
        this->field_58 = a2;
    }
}

void RenderState_t::setBlendingFactor(uint32_t a2)
{
    if (this->field_5C != a2) {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_BLENDFACTOR, a2);
        this->field_5C = a2;
    }
}

void RenderState_t::setAlphaReferenceValue(uint32_t a2)
{
    if (this->field_4C != a2)
    {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_ALPHAREF, a2);
        this->field_4C = a2;
    }
}

void RenderState_t::setStencilPassOperation(D3DSTENCILOP op) {
    if constexpr (1)
    {
        if (this->field_8 != op) {
            g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_STENCILPASS, op);
            this->field_8 = op;
        }
    } else {
        THISCALL(0x00401AA0, this, op);
    }
}

void RenderState_t::setBlendOperation(D3DBLENDOP op)
{
    if (this->field_60 != op) {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_BLENDOP, op);
        this->field_60 = op;
    }
}

void RenderState_t::setFogEnable(bool a2) {
    if (this->field_88 != a2) {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_FOGENABLE, a2);
        this->field_88 = a2;
    }
}

void RenderState_t::setFogColor(uint32_t a2)
{
    if (this->field_8C != a2) {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_FOGCOLOR, a2);
        this->field_8C = a2;
    }
}

void RenderState_t::sub_55DFE0(Float a2)
{
    if (not_equal<float>(this->field_94, a2)) {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_FOGSTART, a2);
        this->field_94 = a2;
    }
}

void RenderState_t::sub_55E010(Float a2)
{
    if (not_equal<float>(this->field_98, a2))
    {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_FOGEND, a2);
        this->field_98 = a2;
    }
}

void RenderState_t::setStencilBufferTestFunction(D3DCMPFUNC func)
{
    if constexpr (1)
    {
        if (this->field_C != func) {
            g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_STENCILFUNC, func);
            this->field_C = func;
        }
    } else {
        THISCALL(0x00401AD0, this, func);
    }
}

void RenderState_t::setDepthBufferWriteEnabled(bool enabled)
{
    if (g_renderState().field_74 != enabled) {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_ZWRITEENABLE, enabled);
        g_renderState().field_74 = enabled;
    }
}

void RenderState_t::setAlphaBlending(bool a2)
{
    if (this->field_50 != a2)
    {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_ALPHABLENDENABLE, a2);
        this->field_50 = a2;
    }
}

void RenderState_t::setColourBufferWriteEnabled(unsigned int a2)
{
    if constexpr (1)
    {
        if (this->field_A8 != a2) {
            g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(),
                                                       D3DRS_COLORWRITEENABLE,
                                                       a2);
            this->field_A8 = a2;
        }
    } else {
        THISCALL(0x00401DA0, this, a2);
    }
}

void RenderState_t::setAlphaFunction(D3DCMPFUNC a2) {
    if (this->field_48 != a2) {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_ALPHAFUNC, a2);
        this->field_48 = a2;
    }
}

void RenderState_t::setDepthBuffer(D3DZBUFFERTYPE Type)
{
    if ( g_renderState().field_78 != Type )
    {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_ZENABLE, Type);
        g_renderState().field_78 = Type;
    }
}

void RenderState_t::setDepthBufferFunction(D3DCMPFUNC func)
{
    if (this->field_7C != func) {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_ZFUNC, func);
        this->field_7C = func;
    }
}

void RenderState_t::setBlending(nglBlendModeType blend_mode, uint32_t BlendModeConst, uint32_t ref_value)
{
    if constexpr (0)
    {
        if ( this->m_blend_mode != blend_mode || this->field_D0 != BlendModeConst || blend_mode == 1 )
        {
            switch ( blend_mode )
            {
            case NGLBM_OPAQUE:
                this->setAlphaTesting(false);
                this->setAlphaBlending(false);
                break;
            case NGLBM_PUNCHTHROUGH:
                this->setAlphaTesting(true);
                this->setAlphaFunction(D3DCMP_GREATER);
                this->setAlphaReferenceValue(ref_value);

                this->setAlphaBlending(false);
                break;
            case NGLBM_BLEND:
                this->setAlphaTesting(true);
                this->setAlphaFunction(D3DCMP_GREATER);
                this->setAlphaReferenceValue(0);

                this->setAlphaBlending(true);
                this->setBlendOperation(D3DBLENDOP_ADD);
                this->setSrcBlend(D3DBLEND_SRCALPHA);
                this->setDestBlend(D3DBLEND_INVSRCALPHA);
                break;
            case NGLBM_ADDITIVE:
                this->setAlphaTesting(true);
                this->setAlphaFunction(D3DCMP_GREATER);
                this->setAlphaReferenceValue(0);

                this->setAlphaBlending(true);
                this->setBlendOperation(D3DBLENDOP_ADD);
                this->setSrcBlend(D3DBLEND_SRCALPHA);
                this->setDestBlend(D3DBLEND_ONE);
                break;
            case NGLBM_SUBTRACTIVE:
                this->setAlphaTesting(true);
                this->setAlphaFunction(D3DCMP_GREATER);
                this->setAlphaReferenceValue(0);

                this->setAlphaBlending(true);
                this->setBlendOperation(D3DBLENDOP_REVSUBTRACT);
                this->setSrcBlend(D3DBLEND_SRCALPHA);
                this->setDestBlend(D3DBLEND_ONE);
                break;
            case NGLBM_CONST_BLEND:
                this->setAlphaTesting(true);
                this->setAlphaFunction(D3DCMP_GREATER);
                this->setAlphaReferenceValue(0);

                this->setAlphaBlending(true);
                this->setBlendOperation(D3DBLENDOP_ADD);
                this->setSrcBlend(D3DBLEND_BLENDFACTOR);
                this->setDestBlend(D3DBLEND_INVBLENDFACTOR);
                this->setBlendingFactor(BlendModeConst << 24);
                break;
            case NGLBM_CONST_ADDITIVE:
                this->setAlphaTesting(true);
                this->setAlphaFunction(D3DCMP_GREATER);
                this->setAlphaReferenceValue(0);

                this->setAlphaBlending(true);
                this->setBlendOperation(D3DBLENDOP_ADD);
                this->setSrcBlend(D3DBLEND_BLENDFACTOR);
                this->setDestBlend(D3DBLEND_ONE);
                this->setBlendingFactor(BlendModeConst << 24);
                break;
            case NGLBM_CONST_SUBTRACTIVE:
                this->setAlphaTesting(true);
                this->setAlphaFunction(D3DCMP_GREATER);
                this->setAlphaReferenceValue(0);

                this->setAlphaBlending(true);
                this->setBlendOperation(D3DBLENDOP_REVSUBTRACT);
                this->setSrcBlend(D3DBLEND_BLENDFACTOR);
                this->setDestBlend(D3DBLEND_ONE);
                this->setBlendingFactor(BlendModeConst << 24);
                break;
            case NGLBM_DESTALPHA_ADDITIVE:
                this->setAlphaTesting(true);
                this->setAlphaFunction(D3DCMP_GREATER);
                this->setAlphaReferenceValue(0);

                this->setAlphaBlending(true);
                this->setBlendOperation(D3DBLENDOP_ADD);
                this->setSrcBlend(D3DBLEND_ZERO);
                this->setDestBlend(D3DBLEND_SRCALPHA);
                break;
            default:
                assert(0 && "Unsupported blending mode !");
                break;
            }

            this->field_D4 = ref_value;
            this->m_blend_mode = blend_mode;
            this->field_D0 = BlendModeConst;
        }
    }
    else
    {
        THISCALL(0x00774A90, this, blend_mode, BlendModeConst, ref_value);
    }
}

void RenderState_t::setAlphaTesting(bool a2)
{
    if (this->field_44 != a2)
    {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_ALPHATESTENABLE, a2);
        this->field_44 = a2;
    }
}

void RenderState_t::setLighting(uint8_t a2) {
    if (this->field_A0 != a2) {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_LIGHTING, a2);
        this->field_A0 = a2;
    }
}

void RenderState_t::setScissorTestEnabled(bool a2)
{
    if ( this->field_A1 != a2)
    {
        g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_SCISSORTESTENABLE, a2);
        this->field_A1 = a2;
    }
}

void RenderState_t::setCullingMode(D3DCULL a2) {
    //sp_log("setCullingMode:");

    if constexpr (1) {
        if (this->m_cullingMode != a2) {
            g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_CULLMODE, a2);
            this->m_cullingMode = a2;
        }
    } else {
        THISCALL(0x00401DD0, this, a2);
    }
}

void RenderState_t::setStencilBufferWriteMask(uint32_t op)
{
    if constexpr (1)
    {
        if (this->field_1C != op) {
            g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(),
                                                       D3DRS_STENCILWRITEMASK,
                                                       op);
            this->field_1C = op;
        }
    } else {
        THISCALL(0x00401B90, this, op);
    }
}

void RenderState_t::setStencilBufferCompareMask(unsigned int a2) {
    if constexpr (1)
    {
        if (this->m_stencilCompareMask != a2) {
            g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_STENCILMASK, a2);
            this->m_stencilCompareMask = a2;
        }
    } else {
        THISCALL(0x00401B60, this, a2);
    }
}
