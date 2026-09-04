#pragma once

#include "float.hpp"

#include "fetext.h"
#include "mvector.h"
#include "panelanimfile.h"
#include "panelquad.h"

struct nglMeshFile;

struct PanelFile {
    mVector<PanelQuad> pquads;
    mVector<FEText> ptext;
    mVector<PanelAnimFile> field_28;

    //0x00616A60
    void Draw();

    //0x00628960
    void PostUnmashFixup(panel_layer a3);

    PanelAnimFile *GetAnimationPointer(int a1);

    //0x00616B40
    FEText *GetTextPointer(const char *a2);

    //0x00639E40
    PanelQuad *GetPQ(const char *a2);

    //0x0062E4D0
    void Update(Float a2);

    void unmash(mash_info_struct *a1, void *);

    //0x00643000
    static PanelFile *UnmashPanelFile(const char *a1, panel_layer a2);

    static nglMeshFile *&g_curmeshfile;
};

extern void PanelFile_patch();
extern void PanelFile_xbpack_patch();
