#pragma once

#include "float.hpp"
#include "subdivision_visitor.h"
#include "vector3d.h"
#include <cstdint>

struct region;
struct hull;
struct scene_entity;
struct matrix4x4;

template <typename T, int N> struct fixed_bitvector;

struct lego_render_visitor : subdivision_visitor {
    const hull *frustum;
    struct plane_packet {
        const hull *source;
        float x[8], y[8], z[8], w[8];
    } packed_frustum;
    vector3d camera_position;
    float field_98;
    region *reg;
    struct buffered_lego {
        scene_entity *lego;
        float distance_squared;
        float min_y;
        float max_y;
        vector3d position;
        float fade;
    } *current;
    int buffered_count;

    //0x0052DEC0
    lego_render_visitor(region *reg, const hull *frustum, const vector3d &position, Float scale);

    void render_lego(int index);

    void render_epilog(const buffered_lego &record);

    void render_buffered_legos();
};

struct static_lego_list_methods {
    static void prepare_for_scene_traversal();

    static int traverse_all(const subdivision_node &node, subdivision_visitor &visitor);

    static void init_region_traversal(int count, scene_entity *legos, const fixed_bitvector<uint32_t, 2048> *previous);

    static void term_region_traversal(fixed_bitvector<uint32_t, 2048> *previous);
};
