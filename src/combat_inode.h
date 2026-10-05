#pragma once

#include "info_node.h"
#include "actor.h"

#include "combo_system_move.h"

struct ai_tentacle_info;

namespace ai {

struct als_inode;
struct combat_target_inode;
struct damage_inode;
struct controller_inode;

struct combat_inode : info_node {
    struct incoming_move {
        std::intptr_t m_vtbl;
        int field_4;
        int field_8;
        float field_C;
        int field_10;
        combo_system_move::results field_14;
        bool field_90;

        //0x00467020
        incoming_move();
        explicit incoming_move(from_mash_in_place_constructor *tag);
        static void *native_vtable();

        //0x0048DB20
        incoming_move(const incoming_move &a2);
        incoming_move &operator=(const incoming_move &) = default;

        void _unmash(mash_info_struct *info, void *owner);
        int _get_virtual_type_enum() const
        {
            return 344;
        }
    };

    int field_1C;
    int field_20;
    controller_inode *field_24;
    als_inode *field_28;
    combat_target_inode *field_2C;
    damage_inode *field_30;
    ai_tentacle_info *field_34;
    int field_38;
    int field_3C;
    int field_40;
    int field_44;
    int field_48;
    int field_4C;
    int field_50;
    float field_54;
    float field_58;
    float field_5C;
    float field_60;
    int field_64;
    int field_68;
    float field_6C;
    int field_70;
    int field_74;
    int field_78;
    int field_7C;
    bool field_80;
    bool field_81;
    bool field_82;
    bool field_83;
    bool field_84;
    bool field_85;
    bool field_86;
    int field_88;
    int field_8C;
    int field_90;
    int field_94;
    int field_98;
    int field_9C;
    int field_A0;
    int field_A4;
    int field_A8;
    int field_AC;
    int field_B0;
    bool field_B4;

    int field_B8;
    int field_BC;
    int field_C0;
    int field_C4;
    int field_C8;
    int field_CC;
    bool field_D0;
    int field_D4;
    incoming_move field_D8[4];

    combat_inode();
    explicit combat_inode(from_mash_in_place_constructor *tag);
    void _unmash(mash_info_struct *info, void *owner);
    int _get_virtual_type_enum() const
    {
        return 342;
    }
    static void *native_vtable();
    void _deactivate();
    void _destruct_mashed_class();
    int get_immediate_index() const;
    bool check_avoid_category(string_hash category) const;
    int find_incoming_move(string_hash category) const;
    bool needs_hit_avoid(Float delta);
    void avoid_attack();
    void avoid_all_attacks(vhandle_type<entity> source);
    void end_cur_move();
    void advance_to_next_move();
    bool performing_combat();
    void update_pending_move(const incoming_move &move);
    void apply_move_damage(const incoming_move &move, const vector3d &direction);
    void receive_and_act_on_results(incoming_move &move, bool force);
    void disable_weapon_based_effect(const combo_system_move *move);
    void send_weapon_attack(const incoming_move &move);
    void try_set_forced_react_needed(string_hash reaction, string_hash attack, int type, vhandle_type<entity> source,
                                     vector3d direction, bool pending, bool force);
    void try_set_forced_avoid_needed(string_hash reaction, string_hash attack, int type, vhandle_type<entity> source,
                                     vector3d direction, bool pending, bool force);
    bool consider_forced_responses(string_hash reaction, string_hash attack, string_hash avoid, int type,
                                   vhandle_type<entity> source, const vector3d &direction, bool pending, bool force);
    void possible_non_combo_system_attack(string_hash reaction, string_hash attack, string_hash avoid, int type,
                                          vhandle_type<entity> source, const vector3d &direction, bool force);
    vector3d get_attack_direction();
    bool find_attack_wall(entity_base *target);
    bool select_satisfactory_move(vhandle_type<actor> target, const vector3d &direction, unsigned input,
                                  string_hash category, float eta, bool target_known,
                                  vhandle_type<actor> current_target);
    bool check_for_and_set_next_move();

    //0x00467440
    void _activate(ai_core *a2);

    //virtual
    combo_system_move *get_cur_move();
    combo_system_move *get_next_move();

    //virtual
    bool needs_hit_react(Float a2);
    bool is_ignored_hit_react_category(string_hash category) const;
    int get_react_index();
    int get_avoid_index();
    void consider_incoming_move_forced_responses();
    void clear_forced_react_needed();
    void clear_forced_avoid_needed();
    void clear_from_target(vhandle_type<actor> target);
    bool has_cur_move() const
    {
        return field_40 != -1;
    }
    bool can_air_attack() const
    {
        return field_68 < 2;
    }
    bool has_accum_air_attack() const
    {
        return field_68 > 0;
    }
    void _frame_advance(Float delta);
    void advance_tether(Float delta);

    //0x00454430
    void _clear_cur_move();

    //virtual
    void clear_cur_move();

    //virtual
    bool has_next_move();

    //0x00454490
    void _clear_next_move();

    //virtual
    void clear_next_move();

    //0x0043FC90
    //virtual
    void left_air();

    static inline string_hash default_id{to_hash("combat_inode")};

    static inline string_hash reject_all_hash{int(to_hash("reject_all_attacks"))};
    static inline string_hash always_keep_target_hash{int(to_hash("always_keep_target"))};
};

struct ped_combat_inode : combat_inode {
    ped_combat_inode();
    explicit ped_combat_inode(from_mash_in_place_constructor *tag);
    int _get_virtual_type_enum() const
    {
        return 158;
    }
    static void *native_vtable();
    void receive_and_act_on_results(incoming_move &move, bool force);
    bool consider_forced_responses(string_hash reaction, string_hash attack, string_hash avoid, int type,
                                   vhandle_type<entity> source, const vector3d &direction, bool pending, bool force);
};

}  // namespace ai
