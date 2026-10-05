#pragma once

#include "nglshader.h"
#include "variable.h"

struct nglScene;

namespace USColorVolShaderSpace {

inline Var<nglScene *> gUSColorVolScene{0x00956350};
inline Var<bool> g_enable_colorvols{0x0095634C};


struct color_volume_time_of_day {
    bool enabled[4] = {false, true, true, false};
};
inline Var<color_volume_time_of_day> volume_time_of_day{0x0091E004};
bool color_volumes_enabled();


void USColorVolPreSceneCallback(unsigned int *&, void *);
void USColorVolPostSceneCallback(unsigned int *&, void *);

struct USColorVolShader : nglShader {
    USColorVolShader();

    void Register();

    tlFixedString _GetName() const;

    void _AddNode(nglMeshNode *, nglMeshSection *, nglMaterialBase *);
};

struct USColorVolNode : nglShaderNode {
    nglMaterialBase *material;

    USColorVolNode(nglMeshNode *, nglMeshSection *, nglMaterialBase *);

    void Render();

    float GetNearestDist() const;

    void GetSortInfo(nglSortInfo &);
    void Delete(unsigned char flags);
};

USColorVolShader &getUSColorVolShader();

}
