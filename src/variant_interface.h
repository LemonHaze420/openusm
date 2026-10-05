#pragma once

#include "conglomerate_interface.h"

#include "mashable_vector.h"
#include "string_hash.h"

struct conglomerate;
struct variant_info {
    string_hash hash;
    unsigned __int16 field_4;
    unsigned __int16 field_6;
    unsigned __int8 *parts;
    char *ifl_frames;
};

struct nglMeshFile;
struct nglMesh;
struct nglMorphSet;
struct tlFixedString;
struct variant_speaker_id_set {
    uint16_t field_0;
    uint16_t id_count;
    uint32_t field_4;
    uint32_t *ids;
};

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
    ~variant_interface();

    void _un_mash(generic_mash_header *header, void *owner, void *object, generic_mash_data_ptrs *data);

    variant_info *get_random_variant();

    void apply_variant(string_hash a2);

    void apply_variant(variant_info *info);

    void destroy_ifl_frames();

    void destroy_mesh_concatenation(nglMesh *mesh);

    void destroy_morph_concatenation(nglMorphSet *a1);

    nglMorphSet *get_morph(const tlFixedString &name);
    nglMorphSet *create_morph_concatenation(nglMorphSet **parts, int count, nglMorphSet *source);

    //virtual
    void release_ifc();
};

extern void variant_interface_patch();
