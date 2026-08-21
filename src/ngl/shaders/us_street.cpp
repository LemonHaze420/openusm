#include "us_street.h"

#include <ngl_dx_shader.h>

#include "func_wrapper.h"
#include "log.h"
#include "utility.h"
#include "variable.h"

#include <d3d9.h>

void sub_411390()
{
    static Var<IDirect3DPixelShader9 *> dword_970920{0x00970920};
    static Var<const DWORD *> off_939E58{0x00939E58};

    static const char text[] =
#if 1
        "tex t0\n"
        "mul r0.xyz, t0, v0\n"
        "mul r0.w, t0.w, v0.w\n";
#else
        "def c0, 0, 1, 0, 1\n"
        "mov r0, c0\n";
#endif

    nglCreatePShader(&(&dword_970920())[0], text);

    if constexpr (0) {
        static const char text[] = "tex t0\n"
                                   "tex t1\n"
                                   "mul r0.xyz, t0, v0\n"
                                   "mul r0.xyz, r0, t1.w\n"
                                   "+mul r0.w, t0.w, v0.w\n";

        nglCreatePShader(&(&dword_970920())[1], text);
    } else {
        CreatePixelShader(&(&dword_970920())[1], (&off_939E58())[1]);
    }

    if constexpr (1) {
        static const char text[] =
#if 1
            "tex t0\n"
            "tex t1\n"
            "tex t2\n"
            "mul r0.xyz, t0, v0\n"
            "mul r0.xyz, r0, t1.w\n"
            "mul r0.xyz, r0, t2.w\n"
            "mul r0.w, t0.w, v0.w\n";
#else
            "def c0, 0, 1, 0, 1\n"
            "mov r0, c0\n";
#endif

        nglCreatePShader(&(&dword_970920())[2], text);
    } else {
        CreatePixelShader(&(&dword_970920())[2], (&off_939E58())[2]);
    }
}

void us_street_patch()
{
    REDIRECT(0x00411339, sub_411390);
}
