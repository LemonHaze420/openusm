#pragma once

#include "info_node.h"
#include "entity_base_vhandle.h"
#include "vector3d.h"

struct from_mash_in_place_constructor;
struct trigger;

struct venom_inode : ai::info_node {
    int n11;
    char field_20;
    char field_21;
    bool field_22;
    char field_23;
    bool field_24;
    int n2;
    int n2_1;
    int n2_2;
    float optional_pb_float;
    float field_38;
    float float_NULL_1;
    float optional_pb_float_1;
    float float_NULL_2;
    float a3b;
    float float_NULL_3;
    float field_50;
    float optional_pb_float_2;
    float float_NULL;
    float field_5C;
    float optional_pb_float_3;
    float optional_pb_float_4;
    float field_68;
    int field_6C;
    float optional_pb_float_5;
    vector3d base_1;
    vector3d predicted_target_position;
    vector3d field_8C;
    entity_base_vhandle field_98;
    entity_base_vhandle field_9C;
    entity_base_vhandle field_A0;
    void *field_A4;
    trigger *field_A8;
    trigger *field_AC;
    trigger *field_B0;
    trigger *field_B4;
    trigger *field_B8;

    //0x0072F650
    venom_inode(from_mash_in_place_constructor *a2);

    static void *native_vtable();
    void _activate(ai::ai_core *core);
    void _frame_advance(Float elapsed);
    void _deactivate();
    bool is_sable_in_engage_range() const;
    bool accepts_knockdown_attack(string_hash attack);
    void reset_attack_timer();
    bool should_throw_prop();
    bool should_chase();
    bool should_jump_attack();
    void record_target_damage();
    bool should_feed() const;
};
