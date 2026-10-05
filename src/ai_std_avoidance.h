#pragma once

#include "info_node.h"

#include <vector.hpp>

namespace ai {

struct pedestrian_inode;


struct avoidance_obstacle {
    entity_base_vhandle handle;
    bool is_entity;
    bool has_box;
    char padding[2];
    vector3d axes[4];
    vector3d center;
    float top;
};

struct avoidance_inode : info_node {
    bool field_1C;
    _std::vector<avoidance_obstacle> *field_20;
    vector3d desired_direction;
    unsigned field_30;
    int field_34;
    float field_38;
    float field_3C;
    int field_40;
    bool field_44;
    char empty[3];

    //0x006D74E0
    void set_respect_obbs(bool a2);

    avoidance_inode();
    explicit avoidance_inode(from_mash_in_place_constructor *);
    ~avoidance_inode();
    static void *native_vtable();
    void destruct_mashed_class();
    void _activate(ai_core *);
    bool _does_need_advance() const;
    void _frame_advance(Float);
    void refresh_parameters();
    void clear_obstacles();


    void collect_obstacles();
    void steer(const vector3d &, const vector3d &, bool, float, float);
    bool get_entity_box(vector3d *, vector3d &, float &, entity *);
    float obstacle_radius(const avoidance_obstacle &, const vector3d &);
    bool accepts_entity(entity *);
    bool accepts_height(float);

    void dispatch_collect();
    bool dispatch_box(vector3d *, vector3d &, float &, entity *);
    float dispatch_radius(const avoidance_obstacle &, const vector3d &);
    bool dispatch_accepts_entity(entity *);
    bool dispatch_accepts_height(float);
    void add_entity_obstacle(avoidance_obstacle &, entity *);
    vector3d avoidance_force(const avoidance_obstacle &, const vector3d &, const vector3d &);
    float target_overlap(const avoidance_obstacle &, const vector3d &, float);
};

struct ped_avoidance_inode : avoidance_inode {
    bool field_48;
    pedestrian_inode *field_4C;
    float field_50;

    static inline const string_hash default_id{int(to_hash("AVOIDANCE_INODE"))};

    ped_avoidance_inode();
    explicit ped_avoidance_inode(from_mash_in_place_constructor *);
    static void *native_vtable();
    void _activate(ai_core *);
    bool _does_need_advance() const;
    void _frame_advance(Float);
    void collect_obstacles();
    void steer(const vector3d &, const vector3d &, bool, float, float);
    float obstacle_radius(const avoidance_obstacle &, const vector3d &);
    void collect_boxes();
    bool add_pedestrian(entity *);
    void collect_pedestrians();
    bool blocks_translation(const vector3d &);
};

}  // namespace ai
