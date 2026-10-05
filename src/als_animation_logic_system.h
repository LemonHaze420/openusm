#pragma once

#include <cstdint>

#include "als_animation_logic_system_interface.h"
#include "als_inode.h"
#include "base_state_machine.h"

#include <vector.hpp>

struct actor;
struct conglomerate;
struct animation_controller;

namespace als {

struct animation_logic_system_shared;
struct motion_compensator;

struct animation_logic_system : animation_logic_system_interface {
private:
    animation_logic_system_shared *als_shared;
    _std::vector<als::state_machine *> field_8;
    base_state_machine field_18;
    actor *field_6C;
    animation_controller *the_controller;
    motion_compensator *field_74;
    void *field_78;
    bool field_7C;
    bool field_7D;
    bool field_7E;
    bool field_7F;

public:
    //0x004ABB80
    animation_logic_system(actor *a1);

    //0x004A9000
    ~animation_logic_system();

    void *operator new(std::size_t sz);

    void operator delete(void *, std::size_t sz);

    //0x004933C0
    als_meta_anim_table_shared *get_meta_anim_table();

    actor *get_actor()
    {
        return field_6C;
    }

    animation_controller *get_animation_controller() const
    {
        return the_controller;
    }

    bool sub_49F2A0();

    //0x0049F360
    float convert_layer_id_to_priority(layer_types a2);

    void sub_4A6630(layer_types a2);
    void kill_animation_domain(uint32_t domain);

    base_state_machine *get_als_layer_internal(layer_types a2);

    void transition_layer(layer_types a2, string_hash a3);

    //0x004A6400
    //virtual
    void _frame_advance_play_new_animations(Float a2);

    //virtual
    void _frame_advance_update_pending_params(Float a2);

    //virtual
    void _frame_advance_change_mocomp(Float a2);

    //virtual
    void _frame_advance_run_mocomp_pre_anim(Float a2);

    //virtual
    void _frame_advance_controller(Float a2);

    //virtual
    void _frame_advance_post_controller(Float a2);

    //0x00498F70
    void enter_biped_physics();

    void exit_biped_physics();

    //virtual
    void suspend_logic_system(bool a2);

    //0x004A63F0
    /* virtual */ state_machine *_get_als_layer(layer_types a2);

    //0x0049F150
    //virtual
    void _kill_all_domains(uint32_t a2);

    //0x004931F0
    //virtual
    void _suspend_logic_system(bool a2);

    //0x004ABC60
    //virtual
    void _create_instance_data(animation_logic_system_shared *a2);

    //virtual
    void _delete_instance_data();

    //virtual
    void _reset_animation_player();

    //0x00498D10
    /* virtual */ bool _frame_advance_should_do_frame_advance(Float a2);

    /* virtual */ void _frame_advance_main_als_advance(Float a2);

    //virtual
    //0x0049F1A0
    void _frame_advance_post_request_processing(Float a2);

    //virtual
    void _frame_advance_on_layer_trans(Float a2);

    //0x0049CC90
    //virtual
    void _frame_advance_post_logic_processing(Float a2);

    //virtual
    bool _sub_4933E0();

    //0x00498F30
    //virtual
    void _change_mocomp();

    static inline const string_hash anim_start_frame_hash{static_cast<int32_t>(to_hash("anim_start_frame"))};
};

}  // namespace als

extern void animation_logic_system_patch();
