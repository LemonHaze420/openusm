#pragma once

#include <cstdint>

struct nglMesh;
struct nglMaterialBase;
struct scene_entity;
struct proximity_map;
struct region;
struct scene_entity {
    uint8_t quantized_yaw;
    uint8_t fade;
    uint16_t field_2;
    float x;
    float y;
    float z;
    uint32_t flags;
    uint32_t field_14;
    nglMesh *mesh;
    uint32_t material_indices;
};

static_assert(sizeof(scene_entity) == 0x20);

struct lego_map_root_node {
    nglMesh **field_0;
    nglMaterialBase **field_4;
    scene_entity *field_8;
    proximity_map *field_C;
    int field_10;
    int field_14;
    int field_18;
    uint16_t field_1C;

    lego_map_root_node();

    //0x0054E5A0
    void un_mash(char *image, int *a3, region *reg);
};
