#pragma once

#include "mashable_vector.h"
#include "variable.h"
#include "vector3d.h"

#include <cstdint>

struct nglTexture;
struct nglMesh;
struct nglMaterialBase;
struct generic_mash_header;
struct generic_mash_data_ptrs;
struct hull;


struct lod_building {
    vector3d position;
    uint16_t color;
    uint16_t height;
    uint8_t width;
    uint8_t depth;
    uint8_t flags;
    uint8_t rotation;
    uint32_t field_14;
};

struct lod_batch {
    uint32_t field_0;
    mashable_vector<lod_building> buildings;
    vector3d corners[4];
    float height;
    uint32_t field_40;
    uint32_t field_44;


    bool outside_frustum(const hull &frustum) const;

    void render(const vector3d &camera_position, bool zoom_map) const;
};

struct strip_lod {
    uint32_t field_0;
    uint32_t field_4;
    mashable_vector<lod_batch> batches;
    mashable_vector<nglMesh *> meshes;


    void un_mash_start(generic_mash_header *, void *, generic_mash_data_ptrs *, void *);

    void render_meshes();
};

struct city_lod {
    strip_lod *field_0;
    bool field_4;


    city_lod(const char *name);

    void render();

    static inline Var<nglMesh *> map_mesh{0x0095C188};
    static inline Var<nglMesh *> sides_mesh{0x0095C16C};
    static inline Var<nglMesh *> top_mesh{0x0095C748};
    static inline Var<nglMesh *> ground_mesh{0x0095C714};
    static inline Var<nglMesh *> box_mesh{0x0095C1D8};
    static inline Var<nglTexture *> top_texture{0x0095C710};
    static inline Var<nglMaterialBase *> top_material{0x00956358};
};

extern void city_lod_patch();
