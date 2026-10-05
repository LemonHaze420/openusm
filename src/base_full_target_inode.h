#pragma once

#include "actor.h"
#include "entity_base_vhandle.h"
#include "info_node.h"
#include "mVectorBasic.h"

namespace ai {

struct base_full_target_inode : info_node {
    mVectorBasic<vhandle_type<actor>> *field_1C;
    int field_20;
    int field_24;
    mVectorBasic<vhandle_type<actor>> *field_28;
    int field_2C;
    int field_30;
    vhandle_type<actor> field_34;
    int field_38;
    vector3d last_known_position;
    float field_48;
    int field_4C;
    int field_50;
    bool field_54;
    bool field_55;
    unsigned char pad_56[2];
    float target_selection_delay;
    bool field_5C;
    bool allow_target_maintenance;
    bool perfect_perception;
    unsigned char pad_5F;
    float vision_range;
    float vision_angle_cos;
    float viable_radius;
    float aware_radius;
    float ellipse_length;
    float ellipse_width;
    float close_vision_angle_cos;
    float close_vision_range;
    float touch_range;

    //0x006A1910
    base_full_target_inode();

    //0x006A1990
    base_full_target_inode(from_mash_in_place_constructor *a2);
    static void *native_vtable();
    void _deactivate();
    vhandle_type<actor> find_target();


    void _activate(ai_core *core);
    void register_as_targetable();
    void unregister_as_targetable();
    void update_cached_params(bool force);
    void reset_target_selection_delay();
    int _get_virtual_type_enum() const
    {
        return 349;
    }
    void _frame_advance(Float delta);
    void player_style_frame_advance(Float delta);
    vector3d get_controller_look_direction();
    void start_multi_frame_search(int budget);
    void update_targeting();
    void calc_and_update_target(vhandle_type<actor> candidate);
    vector3d get_look_direction()
    {
        return get_controller_look_direction();
    }
    bool is_perfect_perception() const
    {
        return perfect_perception;
    }
    float get_vision_angle_cos() const
    {
        return vision_angle_cos;
    }
    float get_vision_range() const
    {
        return vision_range;
    }
    float get_viable_radius() const
    {
        return viable_radius;
    }
    float get_aware_radius() const
    {
        return aware_radius;
    }
    bool is_multi_frame_search_active() const
    {
        return field_54;
    }
    void end_multi_frame_search()
    {
        field_54 = false;
    }
    void clear_target()
    {
        field_34 = {0};
    }

    //virtual
    vhandle_type<actor> quick_targeting();

    //virtual
    bool is_target_known();
};


struct targetable_inode_285 : info_node {
    int field_1C;
    int field_20;

    targetable_inode_285();
    explicit targetable_inode_285(from_mash_in_place_constructor *tag);
    ~targetable_inode_285();
    static void *native_vtable();
    static mVectorBasic<vhandle_type<actor>> &target_list();
    int _get_virtual_type_enum() const
    {
        return 285;
    }
    void _activate(ai_core *core);
    void unregister_as_targetable();
    void _destruct_mashed_class();
};
}  // namespace ai
