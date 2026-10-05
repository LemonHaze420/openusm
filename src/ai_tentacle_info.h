#pragma once

#include "float.hpp"
#include "entity_base_vhandle.h"
#include "quaternion.h"
#include "mashable_vector.h"
#include "msimpletemplates.h"
#include "oldmath_po.h"
#include "resource_key.h"
#include "vector3d.h"

struct ai_tentacle_engine;
struct entity_base;
struct line_info;
struct polytube;
struct polytube_misc_render_object;

namespace ai {
struct ai_core;
}

struct polytube_render_info {
    resource_key texture;
    int blend_mode;
    float radius;
    float texture_scale;
    int num_sides;
    int spline_flags;
    int field_1C;
    float field_20;
    float field_24;

    polytube_render_info();
};

struct ai_tentacle_info {
    resource_key field_0;
    ai::ai_core *my_ai;
    polytube *tentacle;
    polytube_render_info *render_info;
    float field_14;
    float field_18;
    float field_1C;
    float tween_timer;
    float tween_duration;
    float tween_amount;
    vector3d field_2C;
    quaternion field_38;
    mashable_vector<vector3d> tween_positions;
    entity_base *base_node;
    entity_base *end_node;
    mashable_vector<entity_base *> nodes;
    vector3d field_60;
    vector3d end_pos;
    quaternion field_78;
    mashable_vector<vector3d> positions;
    mashable_vector<po> node_po_storage;
    float field_98;
    float field_9C;
    float field_A0;
    float field_A4;
    int field_A8;
    simple_list<ai_tentacle_engine *> engines;
    int field_B8;
    entity_base_vhandle field_BC;
    simple_list<polytube_misc_render_object *> misc_render_objects;
    float field_CC;
    int field_D0;
    int field_D4;


    explicit ai_tentacle_info(ai::ai_core *core);
    ~ai_tentacle_info();
    void create_tentacle(polytube *tube);
    void frame_advance(Float time_step);
    void update_spline();
    void kill_all_engines();
    void apply_render_info();
    po get_end_po() const;
    po get_abs_end_po() const;
    void create_line(const vector3d &end, const vector3d *facing);

    const vector3d &get_end_position() const
    {
        return this->end_pos;
    }

    void set_end_position(const vector3d &a2)
    {
        this->end_pos = a2;
    }

    auto get_num_positions() const
    {
        return this->positions.size();
    }

    void set_position(int index, const vector3d &a1);

    void set_code_blend(Float a2, Float a3);

    void init_code_tween(Float a2);

    void init_positions(bool a2);

    vector3d correct_tentacle_pos(line_info &a3, bool &a4, vector3d &a5, vector3d &a6);

    int push_engine(ai_tentacle_engine *eng);
};
