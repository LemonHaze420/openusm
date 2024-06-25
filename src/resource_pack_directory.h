#pragma once

#include "tlresource_directory.h"

struct resource_directory;
struct nglTexture;
struct tlFixedString;
struct nglMeshFile;
struct tlHashString;
struct nglMesh;
struct nglMorphFile;
struct nglMorphSet;
struct nglMaterialFile;
struct nglMaterialBase;
struct nalBaseSkeleton;
struct nalAnimFile;
struct nalAnyPose;
struct nalSceneAnim;

template<typename T>
struct nalAnimClass;

struct resource_pack_directory {
    resource_directory *field_0;
    tlresource_directory<nglTexture, tlFixedString> field_4;
    tlresource_directory<nglMeshFile, tlFixedString> field_C;
    tlresource_directory<nglMesh, tlHashString> field_14;
    tlresource_directory<nglMorphSet, tlHashString> field_1C;
    tlresource_directory<nglMorphFile,tlFixedString> field_24;
    tlresource_directory<nglMaterialBase, tlHashString> field_2C;
    tlresource_directory<nglMaterialFile, tlFixedString> field_34;
    tlresource_directory<nalAnimFile, tlFixedString> field_3C;
    tlresource_directory<nalAnimClass<nalAnyPose>, tlFixedString> field_44;
    tlresource_directory<nalSceneAnim, tlFixedString> field_4C;
    tlresource_directory<nalBaseSkeleton, tlFixedString> field_54;

    //0x0052A9A0
    resource_pack_directory();

    resource_directory *get_resource_directory() {
        return this->field_0;
    }

    void set_resource_directory(resource_directory *dir);

    void clear();
};

extern void resource_pack_directory_patch();
