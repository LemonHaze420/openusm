#include "nglshader.h"

#include "common.h"
#include "fixedstring.h"
#include "geometry_manager.h"
#include "log.h"
#include "ngl.h"
#include "ngl_params.h"
#include "ngl_scene.h"
#include "trace.h"
#include "variables.h"
#include "vector4d.h"
#include "vtbl.h"
#include <ngl_dx_state.h>

#include <d3d9.h>

#include "tl_instance_bank.h"
#include "tl_system.h"

VALIDATE_SIZE(nglShader, 0xC);

VALIDATE_SIZE(nglShaderNode, 0x14);

#if !STANDALONE_SYSTEM
int &nglShader::NextID = var<int>(0x00972910);
#else
int &nglShader::NextID = []() -> auto & {
    static int g_NextID{};
    return g_NextID;
}();
#endif

nglShader::nglShader() {}

void nglShader::_Register()
{
    TRACE("nglShader::Register");

    this->field_8 = nglShader::NextID++;

    tlFixedString v1 = this->GetName();
    nglShaderBank.Insert(v1, this);
}

tlFixedString nglShader::GetName()
{
    void(__fastcall * func)(void *, void *, tlFixedString *) = CAST(func, get_vfunc(m_vtbl, 0x4));

    tlFixedString result;
    func(this, nullptr, &result);

    return result;
}

void nglShader::AddNode(nglMeshNode *a1, nglMeshSection *a2, nglMaterialBase *a3)
{
    void(__fastcall * func)(void *, void *, nglMeshNode *, nglMeshSection *, nglMaterialBase *) =
        CAST(func, get_vfunc(m_vtbl, 0x8));

    func(this, nullptr, a1, a2, a3);
}

void nglShader::BindMaterial(nglMaterialBase *mat)
{
    void(__fastcall * func)(void *, void *, nglMaterialBase *) = CAST(func, get_vfunc(m_vtbl, 0xC));
    func(this, nullptr, mat);
}

void nglShader::ReleaseMaterial(nglMaterialBase *mat)
{
    void(__fastcall * func)(void *, void *, nglMaterialBase *) = CAST(func, get_vfunc(m_vtbl, 0x10));

    func(this, nullptr, mat);
}

void nglShader::RebaseMaterial(nglMaterialBase *Material, unsigned int a2)
{
    void(__fastcall * func)(void *, void *, nglMaterialBase *, uint32_t) = CAST(func, get_vfunc(m_vtbl, 0x14));

    func(this, nullptr, Material, a2);
}

bool nglShader::_CheckMaterialVersion(nglMaterialBase *)
{
    return true;
}

bool nglShader::CheckMaterialVersion(nglMaterialBase *mat)
{
    bool(__fastcall * func)(nglShader *, void *, nglMaterialBase *) = CAST(func, get_vfunc(m_vtbl, 0x18));

    return func(this, nullptr, mat);
}

bool nglShader::_CheckVertexDefVersion(nglMeshSection *)
{
    TRACE("nglShader::_CheckVertexDefVersion");

    return true;
}

bool nglShader::CheckVertexDefVersion(nglMeshSection *Section)
{
    bool(__fastcall * func)(void *, void *, nglMeshSection *) = CAST(func, get_vfunc(m_vtbl, 0x1C));

    return func(this, nullptr, Section);
}

void nglShader::_BindSection(nglMeshSection *)
{
    TRACE("nglShader::_BindSection");
}

void nglShader::BindSection(nglMeshSection *Section)
{
    void(__fastcall * func)(void *, void *, nglMeshSection *) = CAST(func, get_vfunc(m_vtbl, 0x20));

    func(this, nullptr, Section);
}

bool nglShader::IsSwitchable()
{
    bool(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x24));

    return func(this);
}

nglShaderNode::nglShaderNode(nglMeshNode *a2, nglMeshSection *a3)
{
    this->m_meshNode = a2;
    this->m_meshSection = a3;
}

void nglShaderNode::Render()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(this->m_vtbl, 0x0));
    func(this);
}

void nglShaderNode::sub_413AF0()
{
    THISCALL(0x00413AF0, this);
}

void sub_417C10(nglShaderNode *a1)
{
    auto v1 = sub_414360(a1->m_meshSection->SphereCenter, a1->m_meshNode->LocalToWorld);
    auto v2 = sub_414360(v1, nglCurScene->WorldToView);
    *(float *)&a1->m_tex = v2[2] + a1->m_meshSection->SphereRadius;
    a1->m_next_node = nglCurScene->TransNodes;
    nglCurScene->TransNodes = a1;
    ++nglCurScene->TransListCount;
}

void sub_413850(nglMaterialBase *a1, nglParamSet<nglShaderParamSet_Pool> *a2, color *a3)
{
#if STANDALONE_SYSTEM
    // Materials carry one RGBA float quartet per time-of-day at offset 0x1C.
    const auto *material_color = reinterpret_cast<const float *>(&a1->field_1C) + 4 * g_TOD;
    a3->r = material_color[0];
    a3->g = material_color[1];
    a3->b = material_color[2];
    a3->a = material_color[3];

    if (a2->IsSetParam<nglTintParam>()) {
        const auto *tint = a2->Get<nglTintParam>()->field_0;
        const color tinted{
            a3->r * tint->x,
            a3->g * tint->y,
            a3->b * tint->z,
            a3->a * tint->w,
        };
        *a3 = tinted;
    }
#else
    CDECL_CALL(0x00413850, a1, a2, a3);
#endif
}

color *sub_413F80(color *a1, nglMaterialBase *a2, nglParamSet<nglShaderParamSet_Pool> *a3, uint32_t a4)
{
#if STANDALONE_SYSTEM
    color constant_data;
    sub_413850(a2, a3, &constant_data);

    if (EnableShader) {
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, a4, &constant_data.r, 1);
    } else {
        const auto to_byte = [](float value) -> uint32_t {
            return static_cast<uint32_t>(value * 255.0f) & 0xFFu;
        };

        const uint32_t packed_color = (to_byte(constant_data.a) << 24) |
                                       (to_byte(constant_data.r) << 16) |
                                       (to_byte(constant_data.g) << 8) |
                                       to_byte(constant_data.b);
        auto &render_state = g_renderState();
        if (render_state.field_9C != packed_color) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_TEXTUREFACTOR, packed_color);
            render_state.field_9C = packed_color;
        }
    }

    g_renderState().setBlending(
        bit_cast<uint32_t>(constant_data.a) == bit_cast<uint32_t>(1.0f) ? NGLBM_OPAQUE : NGLBM_BLEND, 0, 128);

    auto &render_state = g_renderState();
    if (render_state.field_A8 != 7) {
        IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_COLORWRITEENABLE, 7);
        render_state.field_A8 = 7;
    }

    *a1 = constant_data;
    return a1;
#else
    return (color *)CDECL_CALL(0x00413F80, a1, a2, a3, a4);
#endif
}

void nglShader_patch()
{
    REDIRECT(0x00413F93, sub_413850);
    REDIRECT(0x00418C84, sub_413850);
    REDIRECT(0x00419F34, sub_413850);

    set_vfunc(0x00871504, func_address(&nglShader::_CheckVertexDefVersion));
}
