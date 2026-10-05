#pragma once

#include "info_node.h"

struct traffic;

namespace ai {

struct traffic_inode : info_node {
    struct CarCombatInfo {
        int section;
        int hit_points;
        float fire_interval;
        int burst_size;
        float attack_interval;
        float damage;
        float sense_lead_time;
        float counter_window;
        float shot_timer;
        float attack_timer;
        float sense_timer;
        int shots_remaining;
        bool field_30;
        bool dodged;
        bool attack_left;
        bool attack_announced;

        CarCombatInfo();
        explicit CarCombatInfo(from_mash_in_place_constructor *);
        void reset_timers();
        void frame_advance(actor *owner, Float time);
    };

    CarCombatInfo hood;
    CarCombatInfo roof;
    CarCombatInfo trunk;
    unsigned flags;
    int animation_vehicle_type;
    traffic *traffic_ptr;
    bool field_C4;
    bool field_C5;
    bool field_C6;
    bool field_C7;
    bool field_C8;

    traffic_inode();
    explicit traffic_inode(from_mash_in_place_constructor *);
    ~traffic_inode();
    static void *native_vtable();
    void _destruct_mashed_class();
    void attach_to_traffic();
    bool remove_from_traffic_system(bool play_voice);
    void _activate(ai_core *core);
    void _frame_advance(Float time);

    void initialize(bool car_combat_enabled);
    void _reset();
    bool car_combat_enabled() const;
    void determine_vehicle_type_for_anims();
    void init(int section, int hit_points, float fire_interval, int burst_size, float attack_interval, float damage,
              float sense_lead_time, float counter_duration);

    static inline string_hash default_id{int(to_hash("TRAFFIC"))};
};

}  // namespace ai
