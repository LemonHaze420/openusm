#pragma once

#include "als_layer_types.h"
#include "float.hpp"
#include "variable.h"

#include <list.hpp>

namespace als {

struct animation_logic_system;
struct animation_logic_system_shared;
struct state_machine;

struct animation_logic_system_interface {
    int m_vtbl;

    //virtual
    state_machine *get_als_layer(layer_types a2);  // = 0;

    //virtual
    void kill_all_domains(uint32_t a2);  // = 0;

    //virtual
    void suspend_logic_system(bool a2);  // = 0;

    //virtual
    void create_instance_data(animation_logic_system_shared *system_shared);  // = 0;

    //virtual
    void delete_instance_data();  // = 0;

    //virtual
    void reset_animation_player();  // = 0;

    //virtual
    bool frame_advance_should_do_frame_advance(Float a2);  // = 0;

    //virtual
    void frame_advance_main_als_advance(Float a2);  // = 0;

    //virtual
    void frame_advance_post_request_processing(Float a2);  // = 0;

    //virtual
    void frame_advance_on_layer_trans(Float a2);  // = 0;

    //virtual
    void frame_advance_post_logic_processing(Float a2);  // = 0;

    //virtual
    void frame_advance_play_new_animations(Float a2);  // = 0;

    //virtual
    void frame_advance_update_pending_params(Float a2);  // = 0;

    //virtual
    void frame_advance_change_mocomp(Float a2);  // = 0;

    //virtual
    void frame_advance_run_mocomp_pre_anim(Float a2);  // = 0;

    //virtual
    void frame_advance_controller(Float a2);  // = 0;

    //virtual
    void frame_advance_post_controller(Float a2);  // = 0;

    //virtual
    bool sub_4933E0();  // = 0;

    //virtual
    void change_mocomp();  // = 0;

    //0x0049ED90
    static void frame_advance_pre_controller_all_alses(Float a1);

    //0x0049EED0
    static void frame_advance_controller_all_als(Float a1);

    //0x0049EF00
    static void frame_advance_post_controller_all_alses(Float a1);

    void force_update();

    //0x00492FC0
    void force_update(Float a2);

    struct value_t {
        animation_logic_system *field_0;
        bool field_4;
    };

    //0x0049EF30
    static void remove_from_als_list(animation_logic_system_interface *a1);

    static _std::list<value_t> &the_als_list;
};

}  // namespace als


extern void als_animation_logic_system_interface_patch();
