#include "nglemptyshader.h"

#include "variables.h"

#if !STANDALONE_SYSTEM
nglEmptyShader & gEmptyShader = var<nglEmptyShader>(0x0093AF40);
#else
nglEmptyShader & gEmptyShader = []() -> auto & {
    static nglEmptyShader gEmptyShader {};
    return gEmptyShader;
}();
#endif

void __fastcall nglEmptyShader_GetName(nglEmptyShader *self, void *, tlFixedString *a1) {
    *a1 = self->_GetName();
}

nglEmptyShader::nglEmptyShader()
{
    static void * g_vtbl[] {
        func_address(&_Register),
        (void *)nglEmptyShader_GetName,
        func_address(&_AddNode),
        func_address(&_BindMaterial),
        func_address(&_ReleaseMaterial),
        func_address(&_RebaseMaterial),
        func_address(&_CheckMaterialVersion),
        func_address(&_CheckVertexDefVersion),
        func_address(&_BindSection)
    };
    this->m_vtbl = CAST(m_vtbl, &g_vtbl);

}

tlFixedString nglEmptyShader::_GetName() const
{
    return tlFixedString {"nglEmpty"};
}

