#pragma once

#include "nglshader.h"

struct nglMaterialBase;

struct USLODShader : nglShader {
    USLODShader();
    void Register();
    tlFixedString _GetName() const;
    void _AddNode(nglMeshNode *, nglMeshSection *, nglMaterialBase *);
    //virtual
    void _BindMaterial(nglMaterialBase *a1);

    void _RebaseMaterial(nglMaterialBase *, uint32_t);
};

struct NewlodShader : nglShader {
    NewlodShader();
    void Register();
    tlFixedString _GetName() const;
    void _AddNode(nglMeshNode *, nglMeshSection *, nglMaterialBase *);
    void _BindMaterial(nglMaterialBase *);
    void _ReleaseMaterial(nglMaterialBase *);
    void _RebaseMaterial(nglMaterialBase *, uint32_t);
};

extern USLODShader &getUSLODShader();
extern NewlodShader &getNewlodShader();

extern void us_lod_patch();
