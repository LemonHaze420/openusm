#pragma once

#include "float.hpp"
#include "mvector.h"
#include "param_block.h"
#include "resource_key.h"
#include "string_hash.h"

#include <list.hpp>

struct ghetto_mash_file_header;
struct actor;
struct capsule_alter_sys;

namespace ai {

struct core_ai_resource;
struct info_node;
struct ai_state_machine;
struct state_graph;

struct ai_core {
    enum mode_e {
        AI_KILLING_MACHINES = 2,
    };

    _std::list<resource_key> field_0;
    ai_state_machine *my_base_machine;
    ai_state_machine *my_locomotion_machine;
    _std::list<ai_state_machine *> my_machine_list;
    _std::list<void *> field_20;
    mode_e my_mode;
    resource_key field_30;
    int field_38;
    mode_e my_locomotion_mode;
    int field_40;
    int field_44;
    string_hash field_48;
    int field_4C;
    param_block field_50;
    char field_5C;
    mVector<info_node> *field_60;
    actor *field_64;
    int field_68;
    core_ai_resource *field_6C;
    capsule_alter_sys *field_70;

    //0x006AEA90
    ai_core(core_ai_resource *a3, const param_block *arg4, actor *a4);

    actor *get_actor(int)
    {
        return field_64;
    }

    param_block *get_param_block()
    {
        return &this->field_50;
    }

    bool push_base_machine(resource_key a2, int a3);

    bool pop_base_machine(int);

    bool change_base_machine(resource_key the_state_graph, int a3, string_hash a4);

    //0x00687C50
    void create_capsule_alter();

    //0x006A36E0
    void post_entity_mash();

    //0x006B48A0
    void frame_advance(Float a2);

    //0x0068FC00
    void set_allow_facing(bool a2);

    //0x006A3590
    bool stop_movement();

    //0x0069B940
    void do_machine_exit(ai_state_machine *a2);

    //0x006A34A0
    bool change_locomotion_machine(const string_hash &a2);

    //0x006B4AD0
    static void frame_advance_all_core_ais(Float a2);

    //0x006A3390
    info_node *get_info_node(string_hash the_info_node, bool a3);

    //0x0068FCF0
    state_graph *find_state_graph(resource_key a2);

    //0x0069B8F0
    ai_state_machine *find_machine(resource_key a2);

    //0x0068FCA0
    void advance_info_nodes(Float a2);

    int can_spawn_state_machine(resource_key a2);

    void spawn_state_machine_internal(ai_state_machine *a2, resource_key graph_name,
                                      ai_state_machine **base_machine_ptr, string_hash a5);

    void advance_machine_recursive(ai_state_machine *a1, Float a2, bool a3);

    static inline Var<_std::list<ai_core *> *> the_ai_core_list_high{0x0096BE24};

    static inline Var<_std::list<ai_core *> *> the_ai_core_list_low{0x0096BE28};
};

}  // namespace ai

void ai_core_patch();
