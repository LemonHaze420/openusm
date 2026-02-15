#include "usbuildingsimpleshader.h"

#include <ngl_dx_shader.h>

#include "fixedstring.h"
#include "func_wrapper.h"
#include "log.h"
#include "tl_instance_bank.h"
#include "tl_system.h"
#include "variables.h"
#include "vtbl.h"

#include <d3d9.h>

USBuildingSimpleShader::USBuildingSimpleShader() {}

int USBuildingSimpleShader::Register() {
    sp_log("USBuildingSimpleShader::Register:");

    if constexpr (1) {
        this->field_8 = nglShader::NextID++;

        void (__fastcall *GetName)(void *, void *, tlFixedString *) = CAST(GetName, get_vfunc(this->m_vtbl, 0x4));

        tlFixedString v2;
        GetName(this, nullptr, &v2);

        static auto & gUSBuildingSimpleShader = var<USBuildingSimpleShader>(0x0091E448);

        nglShaderBank.Insert(v2, &gUSBuildingSimpleShader);

        static Var<IDirect3DVertexDeclaration9 *> dword_9738F8{0x009738F8};
        static Var<D3DVERTEXELEMENT9> stru_939480{0x00939480};
        static Var<int[2]> stru_970510{0x00970510};

        if ( EnableShader )
        {
            static Var<IDirect3DPixelShader9 *> dword_970518{0x00970518};

            if constexpr (0) {
                static Var<DWORD *> off_939AA0{0x00939AA0};

                nglCreateVertexDeclarationAndShader(&stru_970510(), &stru_939480(), off_939AA0());
            } else {
                const char *text =
                    "dcl_position v0\n"
                    "dcl_texcoord v1\n"
                    "dcl_color v2\n"
                    "dp4 oPos.x, v0, c11\n"
                    "dp4 oPos.y, v0, c12\n"
                    "dp4 oPos.z, v0, c13\n"
                    "dp4 oPos.w, v0, c14\n"
                    "mov oT0.xy, v1\n"
                    "dp4 r0.y, v0, c1\n"
                    "add r2.y, r0.y, c9.y\n"
                    "mul oT2.xy, r2.y, c9.x\n"
                    "mul oD0.xyz, v2.x, c8\n"
                    "mov oD0.w, c8\n";

                nglCreateVShader(&stru_939480(), &stru_970510(), 0, text);
            }

            if constexpr (0) {
                static Var<DWORD *> off_939AA4{0x00939AA4};
                CreatePixelShader(&dword_970518(), off_939AA4());
            } else {
                static const char *text =
#if 1
                    "tex t0\n"
                    "tex t2\n"
                    "mul r0, v0, t2\n"
                    "mul r0.xyz, r0, t0\n";
#else
                    "def c0, 0, 1, 0, 1\n"
                    "mov r0, c0\n";
#endif

                nglCreatePShader(&dword_970518(), text);
            }

        }
        else
        {
            if (dword_9738F8() == nullptr)
                IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice,
                                                                    &stru_939480(),
                                                                    &dword_9738F8());
        }

        return 0;

    } else {
        return THISCALL(0x004020E0, this);
    }
}
