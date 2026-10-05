#pragma once

#include "info_node.h"
#include "resource_key.h"

namespace ai {


struct loco_inode : info_node {
    string_hash als_category;
    float field_20;
    float field_24;
    float goto_destination[3];
    float goto_speed;
    float goto_radius;
    float min_goto_time;
    int field_40;
    float facing_direction[3];
    float field_50;
    bool field_54;
    bool field_55;
    bool field_56;
    bool needs_repathfind;
    bool field_58;
    bool field_59;
    bool allow_facing_change;
    bool explicit_goto_speed;
    bool explicit_goto_radius;
    bool explicit_min_goto_time;
    bool field_5E;
    bool field_5F;
    bool always_update_als;
    unsigned char field_61[3];

    static constexpr unsigned virtual_type = 391;
    static void *native_vtable();

    loco_inode();


    explicit loco_inode(from_mash_in_place_constructor *tag);


    void initialize_loco_inode();

    const resource_key &get_graph() const;

    void reset_loco_defaults();

    void _activate(ai_core *core);
    void _unmash(mash_info_struct *info, void *base);
    const resource_key &_get_graph() const;
    void _reset_loco_defaults();
    void set_goto_speed(float speed);
    void set_goto_radius(float radius);
    void set_min_goto_time(float time);
    void set_facing_dir(const vector3d &direction);
};

struct biped_layer_inode : loco_inode {
    static constexpr unsigned virtual_type = 338;
    static void *native_vtable();
    biped_layer_inode();
    explicit biped_layer_inode(from_mash_in_place_constructor *tag);
    void _initialize_loco_inode();
    const resource_key &_get_graph() const;
};

struct nonpath_loco_inode : loco_inode {
    static constexpr unsigned virtual_type = 156;
    static void *native_vtable();
    nonpath_loco_inode();
    explicit nonpath_loco_inode(from_mash_in_place_constructor *tag);
    void _initialize_loco_inode();
    const resource_key &_get_graph() const;
};

}  // namespace ai
