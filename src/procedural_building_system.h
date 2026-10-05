#pragma once

#include <cstddef>
#include <cstdint>

struct scene_entity;
struct matrix4x4;
struct nglShaderParamSet_Pool;
template <typename> struct nglParamSet;

struct procedural_building_record {
    uint8_t node_type;
    uint8_t fade;
    uint16_t quantized_yaw;
    float x, y, z;
    uint8_t render_flags;
    uint8_t fade_group;
    uint8_t floor_spacing;
    uint8_t facade_flags;
    uint8_t materials[9];
    uint8_t width;
    uint8_t depth;
    uint8_t height;
};
static_assert(sizeof(procedural_building_record) == 0x20);
static_assert(offsetof(procedural_building_record, floor_spacing) == 0x12);
static_assert(offsetof(procedural_building_record, facade_flags) == 0x13);
static_assert(offsetof(procedural_building_record, materials) == 0x14);
static_assert(offsetof(procedural_building_record, width) == 0x1D);

void USProcBlgTopAdd(const scene_entity *building, const matrix4x4 *local_to_world,
                    nglParamSet<nglShaderParamSet_Pool> *params);
void USProcBlgTopRender(void *payload, void *);
