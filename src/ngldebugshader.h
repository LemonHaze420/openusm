#pragma once

#include "nglshader.h"

struct nglDebugShader : nglShader {
    nglDebugShader();

    //0x00783790
    //virtual
    int Register();
};

#if STANDALONE_SYSTEM
void initialize_debug_material_shader();
#endif
