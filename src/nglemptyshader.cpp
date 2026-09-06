#include "nglemptyshader.h"

#include "variables.h"

#if !STANDALONE_SYSTEM
nglEmptyShader &gEmptyShader = var<nglEmptyShader>(0x0093AF40);
#else
nglEmptyShader &gEmptyShader = []() -> auto & {
    static nglEmptyShader gEmptyShader{};
    return gEmptyShader;
}();
#endif

void __fastcall nglEmptyShader_GetName(nglEmptyShader *self, void *, tlFixedString *a1)
{
    *a1 = self->_GetName();
}

nglEmptyShader::nglEmptyShader()
{
    static void *g_vtbl[]{func_address(&nglEmptyShader::_Register),
                          (void *)nglEmptyShader_GetName,
                          func_address(&nglEmptyShader::_AddNode),
                          func_address(&nglEmptyShader::_BindMaterial),
                          func_address(&nglEmptyShader::_ReleaseMaterial),
                          func_address(&nglEmptyShader::_RebaseMaterial),
                          func_address(&nglShader::_CheckMaterialVersion),
                          func_address(&nglShader::_CheckVertexDefVersion),
                          func_address(&nglShader::_BindSection)};
    this->m_vtbl = CAST(m_vtbl, &g_vtbl);
}

tlFixedString nglEmptyShader::_GetName() const
{
    return tlFixedString{"nglEmpty"};
}
