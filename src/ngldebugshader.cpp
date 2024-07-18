#include "ngldebugshader.h"

#include <ngl_dx_shader.h>

#include "func_wrapper.h"
#include "log.h"
#include "tl_instance_bank.h"
#include "tl_system.h"
#include "variables.h"
#include "vtbl.h"

nglDebugShader::nglDebugShader() {}

int nglDebugShader::Register() {
    sp_log("nglDebugShader::Register:");

    if constexpr (0) {
        this->field_8 = nglShader::NextID()++;

        tlFixedString v2 = this->GetName();

        static Var<nglDebugShader> gnglDebugShader{0x009469B0};

        nglShaderBank.Insert(v2, &gnglDebugShader());
        if ( EnableShader )
        {
            static Var<int[2]> stru_976E74{0x00976E74};
            static Var<D3DVERTEXELEMENT9> stru_946940{0x00946940};

            if constexpr (0) {
                static Var<DWORD *> off_946A30{0x00946A30};

                nglCreateVertexDeclarationAndShader(&stru_976E74(), &stru_946940(), off_946A30());
            } else {
                static const char *text =
                    "dcl_position v0\n"
                    "dp4 oPos.x, v0, c0\n"
                    "dp4 oPos.y, v0, c1\n"
                    "dp4 oPos.z, v0, c2\n"
                    "dp4 oPos.w, v0, c3\n"
                    "mov oD0, c4\n"
                    "mov oD1, c91.x\n"
                    "mov oFog, c91.z\n";

                nglCreateVShader(&stru_946940(), &stru_976E74(), 0, text);
            }

            static Var<IDirect3DPixelShader9 *> dword_976E70{0x00976E70};
            static Var<DWORD *> off_946A34{0x00946A34};

            CreatePixelShader(&dword_976E70(), off_946A34());
        }

    } else {
        return THISCALL(0x00783790, this);
    }
}
