#pragma once

#include "info_node.h"
#include "physical_interface.h"
#include "string_hash.h"
#include "variable.h"

struct entity_base;
struct vector3d;
struct po;
struct from_mash_in_place_constructor;

namespace ai {

struct physics_inode : info_node {
    physical_interface *field_1C;

    //0x006A1BE0
    physics_inode(from_mash_in_place_constructor *a2);

    bool get_collided_last_frame() const;

    //0x0068AFF0
    vector3d get_abs_position();

    vector3d get_last_collision_normal() const;

    //0x0068B030
    po &get_abs_po();

    vector3d &get_y_facing();

    vector3d &get_z_facing();

    //0x00695190
    void setup_for_crawl_zip();

    //0x00695030
    void setup_for_bounce();

    //0x00694B13
    void set_adv_standing(bool a2);

    //0x00694B09
    void set_always_standing(bool a2);

    //0x00694B27
    void set_stationary(bool a2);

    //0x00694A80
    void setup_for_jump(const vector3d &a2);

    void set_gravity_multiplier(Float a2);

    //0x0068B170
    float get_gravity_multiplier();

    //0x0068B120
    [[nodiscard]] vector3d get_velocity();

    //0x0068B0B0
    void set_velocity(const vector3d &a2, bool a3);

    //0x00698E50
    void setup_for_walk();

    //0x00694D80
    void cleanup_from_swing();

    //0x00694AF0
    void setup_for_swing();

    void set_collisions_active(bool a1, bool a2);

    //0x0068B080
    void suspend();

    //0x00694B30
    void unsuspend();

    //0x00694B3A
    void enable(bool a2);

    //0x0068B060
    void disable();

    //0x00694B1F
    void set_gravity(bool a2);

    //virtual
    void apply_force_increment(const vector3d &a2, physical_interface::force_type a3, const vector3d &a4, int a5);

    static const inline string_hash default_id{static_cast<int>(to_hash("physics"))};
};
}  // namespace ai

extern void physics_inode_patch();
