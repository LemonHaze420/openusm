#pragma once

#include "nglshader.h"

struct USShaderBase : nglShader {
    //virtual
    bool _IsSwitchable() const;
};
