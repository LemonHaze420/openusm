#pragma once

#include "float.hpp"
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

namespace ai {
struct ai_core;
}

struct ai_tentacle_info {
    resource_key field_0;
    ai::ai_core *my_ai;
    polytube *tentacle;
    int field_10;
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
    struct {
        int field_0;
        uint16_t m_size;

        auto size() const
        {
            return m_size;
        }
    } field_58;
    vector3d field_60;
    vector3d end_pos;
    quaternion field_78;
    mashable_vector<vector3d> positions;
    mashable_vector<po> node_po_storage;
    int field_98;
    int field_9C;
    int field_A0;
    int field_A4;
    int field_A8;
    simple_list<ai_tentacle_engine *> engines;
    int field_B8;
    int field_BC;
    int field_C0;
    int field_C4;
    int field_C8;
    int field_CC;
    int field_D0;
    int field_D4;

    po get_end_po() const;

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
