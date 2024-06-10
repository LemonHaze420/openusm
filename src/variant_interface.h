#pragma once

#include "conglomerate_interface.h"

#include "mashable_vector.h"
#include "string_hash.h"

struct conglomerate;
struct variant_info;
struct nglMeshFile;
struct nglMesh;
struct nglMorphSet;
struct variant_speaker_id_set;

struct variant_interface : conglomerate_interface {
    mashable_vector<variant_info> variants;
    mashable_vector<variant_speaker_id_set> field_14;
    variant_info *current_variant;
    nglMesh *current_mesh;
    nglMorphSet *field_24;
    nglMeshFile *field_28;
    char *field_2C[1];
    int field_30;
    int field_34;
    int field_38;
    int field_3C;
    int field_40;
    int field_44;
    int field_48;
    int field_4C;
    int field_50;
    int field_54;

    variant_interface(conglomerate *);

    variant_info *get_random_variant();

    void apply_variant(string_hash a2);

    void apply_variant(variant_info *info);

    void destroy_ifl_frames();

    void destroy_mesh_concatenation(nglMesh *mesh);

    void destroy_morph_concatenation(nglMorphSet *a1);

    //virtual
    void release_ifc();
};

extern void variant_interface_patch();
