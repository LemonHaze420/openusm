#pragma once

#include "nglshader.h"

#include "fixedstring.h"

#include <cstdint>

struct nglEmptyShader : nglShader {
    nglEmptyShader();

    //virtual
    void _Register() {}

    //virtual
    tlFixedString _GetName() const;

    //virtual
    void _AddNode(nglMeshNode *, nglMeshSection *, nglMaterialBase *) {}

    //virtual
    void _BindMaterial(nglMaterialBase *) {}

    //virtual
    void _ReleaseMaterial(nglMaterialBase *) {}

    //virtual
    void _RebaseMaterial(nglMaterialBase *, unsigned int ) {}

};

extern nglEmptyShader & gEmptyShader;
