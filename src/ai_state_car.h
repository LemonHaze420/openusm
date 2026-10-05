#pragma once

#include "info_node.h"

#include "launch_layer_state.h"

#include "entity.h"
#include "entity_base_vhandle.h"
struct traffic_path_road;
struct traffic;

namespace ai {

struct drive_car_state : launch_layer_state {
    drive_car_state();
    explicit drive_car_state(from_mash_in_place_constructor *constructor);
    static void *native_vtable();
    void activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                  const param_block *params, activate_flag_e flags);
    void deactivate(const mashed_state *next);
    void get_info_node_list(info_node_desc_list &nodes);
    int get_next_direction(traffic_path_road **roads) const;

    //0x0046C6D0
    //virtual
    state_trans_action _check_transition(Float a3);

    //0x0044DBD0
    //virtual
    state_trans_action _get_default_return_code() const;
};

struct ai_car_inode : info_node {
    bool field_1C;
    char field_1D;
    char field_1E;
    bool field_1F;
    vhandle_type<entity> field_20;
    vhandle_type<entity> field_24;
    vhandle_type<entity> field_28;
    int field_2C;
    bool field_30;
    drive_car_state *field_34;
    float field_38;
    ai_car_inode();
    explicit ai_car_inode(from_mash_in_place_constructor *constructor);
    static void *native_vtable();
    void _destruct_mashed_class();
    void _frame_advance(Float time);
    void cancel_search_internal();
    void cancel_search();
    bool start_search(bool occupied_first);
    bool can_use_car(const traffic *car, int seat, int *selected_seat, bool ignore_stealable) const;
    void set_selected_car(entity *car, int seat);

    bool inside_car() const
    {
        return this->field_1C;
    }

    entity *get_selected_car();

    bool search_finished() const;
    int get_next_direction(traffic_path_road **roads);

    //0x0045EED0
    bool car_is_dead() const;

    //0x0046C380
    void clear_car();

    //0x0045EBE0
    void set_inside_car(bool inside);

    static inline const string_hash default_id{int(to_hash("AI_CAR_INODE"))};

    static int &cars_occupied;
    static int &searches_in_progress;
    static int &cars_selected;
};

}  // namespace ai
