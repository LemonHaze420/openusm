#pragma once

#include <cstdint>

struct nglMesh;
struct nglMaterialBase;
struct scene_entity;
struct proximity_map;
struct region;
struct scene_entity {
    uint8_t node_type;
    uint8_t fade;
    uint16_t quantized_yaw;
    float x;
    float y;
    float z;
    union {
        uint32_t flags;
        struct {
            uint8_t render_flags;
            uint8_t fade_group;
            int16_t sphere_x;
        } render;
    };
    union {
        uint32_t field_14;
        struct {
            int16_t sphere_y;
            int16_t sphere_z;
        } sphere;
    };
    nglMesh *mesh;
    union {
        uint32_t material_indices;
        struct {
            uint8_t building_material;
            uint8_t building_width;
            uint8_t building_depth;
            uint8_t building_height;
        } building;
    };
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
