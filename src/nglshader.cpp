#include "nglshader.h"

#include "common.h"
#include "fixedstring.h"
#include "log.h"
#include "ngl_params.h"
#include "trace.h"
#include "vector4d.h"
#include "vtbl.h"

#include "tl_instance_bank.h"
#include "tl_system.h"

VALIDATE_SIZE(nglShader, 0xC);

VALIDATE_SIZE(nglShaderNode, 0x14);

#if !STANDALONE_SYSTEM
int & nglShader::NextID = var<int>(0x00972910);
#else
int & nglShader::NextID = []() -> auto & {
    static int g_NextID {};
    return g_NextID;
}();
#endif

nglShader::nglShader()
{

}

void nglShader::Register()
{
    TRACE("nglShader::Register");

    this->field_8 = nglShader::NextID++;
    auto v4 = this->field_8;

    tlFixedString v1 = this->GetName();

    auto *v2 = v1.to_string();
    sp_log("Registering shader %s, ID: %d\n", v2, v4);

    nglShaderBank.Insert(v1, this);
}

tlFixedString nglShader::GetName() {
    void (__fastcall *func)(void *, void *, tlFixedString *) = CAST(func, get_vfunc(m_vtbl, 0x4));

    tlFixedString result;
    func(this, nullptr, &result);

    return result;
}

void nglShader::AddNode(nglMeshNode *a1, nglMeshSection *a2, nglMaterialBase *a3)
{
    void (__fastcall *func)(void *, void *, nglMeshNode *, nglMeshSection *, nglMaterialBase *) = CAST(func, get_vfunc(m_vtbl, 0x8));

    func(this, nullptr, a1, a2, a3);
}

void nglShader::BindMaterial(nglMaterialBase *mat) {
    sp_log("m_vtbl = 0x%08X", m_vtbl);
    void (__fastcall *func)(void *, void *, nglMaterialBase *) = CAST(func, get_vfunc(m_vtbl, 0xC));

    func(this, nullptr, mat);
}

void nglShader::ReleaseMaterial(nglMaterialBase *mat) {
    void (__fastcall *func)(void *, void *, nglMaterialBase *) = CAST(func, get_vfunc(m_vtbl, 0x10));

    func(this, nullptr, mat);
}

void nglShader::RebaseMaterial(nglMaterialBase *Material, unsigned int a2) {
    void (__fastcall *func)(void *, void *, nglMaterialBase *, uint32_t)
        = CAST(func, get_vfunc(m_vtbl, 0x14));

    if constexpr (1)
    {
        sp_log("0x%08X 0x%08X", m_vtbl, (int) func);
    }

    func(this, nullptr, Material, a2);
}

bool nglShader::CheckMaterialVersion(nglMaterialBase *mat) {
    if constexpr (1) {
        bool (__fastcall *func)(nglShader *, void *, nglMaterialBase *) = CAST(func, get_vfunc(m_vtbl, 0x18));

        return func(this, nullptr, mat);
    } else {
        return true;
    }
}

bool nglShader::CheckVertexDefVersion(nglMeshSection *Section) {
    if constexpr (1) {
        bool (__fastcall *func)(void *, void *, nglMeshSection *) = CAST(func, get_vfunc(m_vtbl, 0x1C));

        return func(this, nullptr, Section);
    } else {
        return true;
    }
}

void nglShader::BindSection(nglMeshSection *Section) {
    void (__fastcall *func)(void *, void *, nglMeshSection *) = CAST(func, get_vfunc(m_vtbl, 0x20));

    func(this, nullptr, Section);
}

bool nglShader::IsSwitchable() {
    bool (__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x24));

    return func(this);
}

void nglShaderNode::Render()
{
    void (__fastcall *func)(void *) = CAST(func, get_vfunc(this->m_vtbl, 0x0));
    func(this);
}

void nglShaderNode::sub_413AF0() {
    THISCALL(0x00413AF0, this);
}

void sub_413850(nglMaterialBase *a1, nglParamSet<nglShaderParamSet_Pool> *a2, color *a3)
{
    if (a2->IsSetParam<nglTintParam>()) {
        auto *c = bit_cast<color *>(a2->Get<nglTintParam>()->field_0);
        sp_log("TintParam = %f", c->a);
    }

    CDECL_CALL(0x00413850, a1, a2, a3);
}

color *sub_413F80(color *a1, nglMaterialBase *a2, nglParamSet<nglShaderParamSet_Pool> *a3, uint32_t a4)
{
    return (color *) CDECL_CALL(0x00413F80, a1, a2, a3, a4);
}

void nglShader_patch()
{
    REDIRECT(0x00413F93, sub_413850);
    REDIRECT(0x00418C84, sub_413850);
    REDIRECT(0x00419F34, sub_413850);
}
