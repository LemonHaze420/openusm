#pragma once

#include "nglshader.h"

namespace USStreetShaderSpace {


template <bool Interior>
struct USGroundShader : nglShader {
    USGroundShader();
    void Register();
    tlFixedString _GetName() const;
    void _AddNode(nglMeshNode *, nglMeshSection *, nglMaterialBase *);
    void _BindMaterial(nglMaterialBase *);
    void _ReleaseMaterial(nglMaterialBase *);
    void _RebaseMaterial(nglMaterialBase *, unsigned int);
    USGroundShader *Delete(unsigned char flags);
};

template <bool Interior>
struct USStreetShaderNode : nglShaderNode {
    nglMaterialBase *material;
    nglTexture *texture;

    USStreetShaderNode(nglMeshNode *, nglMeshSection *, nglMaterialBase *);
    void Render();
    void GetSortInfo(nglSortInfo &);
    USStreetShaderNode *Delete(unsigned char flags);
};

USGroundShader<false> &getUSStreetShader();
USGroundShader<true> &getUSFloorShader();

}

