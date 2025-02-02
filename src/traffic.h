#pragma once

#include "spawnable.h"

#include "actor.h"
#include "entity_base_vhandle.h"
#include "float.hpp"
#include "traffic_path.h"
#include "variable.h"
#include "vector3d.h"
#include "vehicle.h"

#include <vector.hpp>

struct parking_marker;
struct traffic_path_lane;
struct traffic_route;

namespace ai {
struct ai_car_inode;
struct traffic_inode;
}

struct traffic : spawnable {
    int field_8;
    vehicle field_C;
    traffic_path_lane *previous_lane;
    traffic_path_lane *field_140;
    traffic_path_lane *field_144;
    traffic_path_lane *field_148;
    vector3d field_14C;

    bool field_158;
    int field_15C;
    int field_160;
    float field_164;
    int field_168;
    int field_16C;
    vhandle_type<entity> field_170;
    int field_174;
    float field_178;
    traffic_path_intersection::eDirection field_17C;
    bool field_180;
    bool field_181;
    bool field_182;
    bool field_183;
    int field_184;
    traffic_path_intersection *field_188;
    int field_18C;
    traffic_path_lane *field_190;
    float field_194;
    float field_198;
    float field_19C;
    float field_1A0;
    float field_1A4;
    float field_1A8;
    float field_1AC;
    float field_1B0;
    float field_1B4;
    float field_1B8;
    bool field_1BC;
    bool field_1BD;
    bool field_1BE;
    int field_1C0;
    int field_1C4;
    char field_1C8;
    char field_1C9;
    char field_1CA;
    char field_1CB;
    int field_1CC;
    float field_1D0;
    vector3d field_1D4;
    float field_1E0;
    vhandle_type<actor> field_1E4;
    traffic_route *field_1E8;
    _std::vector<vector3d> field_1EC;
    float field_1FC;
    bool field_200;
    bool field_201;
    bool field_202;
    bool field_203;
    int field_204;
    int field_208;
    uint8_t field_20C;
    int field_210;
    int field_214;
    int field_218;
    ai::traffic_inode *field_21C;
    ai::ai_car_inode *field_220;
    int ai_potential_car_counter;
    int field_228;

    ~traffic();

    traffic_path_lane * get_current_lane() {
        return field_140;
    }

    bool is_ai_potential_car() const {
        return this->ai_potential_car_counter > 0;
    }

    bool more_dests() const {
        return !this->field_1EC.empty();
    }

    //0x006C3900
    bool is_fully_pulled_over() const {
        return this->field_1EC.size() < 3;
    }

    //0x006CB1F0
    bool is_not_chase_lane(traffic_path_lane *a1);

    //0x006D07A0
    vector3d pop_dest();

    bool is_ai_car_occupied() const;

    void update_destination(Float a2);

    void sub_6CD2D0(traffic *a2);

    void sub_6DA3B0(Float a2, Float a3, Float a4);

    void sub_6DACB0(entity_base_vhandle a2);

    //0x006C3650
    traffic * car_behind();

    //0x006C7400
    bool start_lane(traffic_path_lane *a2, bool a3);

    //0x006D24A0
    bool start_turn();

    //0x006C37B0
    void clear_previous_lane();

    //0x006C3DD0
    void release_turn();

    //0x006C7680
    void check_lane_change();

    void set_current_lane(traffic_path_lane *a2, int a3, bool a4);

    //0x006C3650
    traffic * car_ahead();

    //0x006BCF60
    void set_ai_controller(ai::ai_car_inode *a2);

    //0x006D0AC0
    void set_ai_car_occupied(bool a2);

    //0x006BC850
    void set_driver_type(int a2);

    //virtual
    //0x006D9070
    void _do_spawn(
            vector3d a4,
            vector3d a2,
            traffic_path_lane *lane,
            bool a10,
            bool a11);

    //virtual
    //0x006D9740
    void _critical_processing(Float a2);

    //virtual
    //0x006CB300
    bool _is_viable_pos(const vector3d &a2);

    void spawn_at_marker(parking_marker *a2);

    static void initialize_traffic();

    static void terminate_traffic();

    //0x006D6350
    static void test_script();

    static void set_traffic_density(Float density);

    static void enable_traffic(bool a1, bool a2);

    //0x006D0910
    static void get_closest_point_on_lane_with_facing(vector3d *a1, vector3d *a2, bool a3);

    //0x004ADE20
    static bool is_unanimated_car(actor *a1);

    //0x006C3510
    static traffic *get_traffic_from_entity(vhandle_type<entity> a1);

    //0x006CCE80
    static void create_new_traffic(int a1);

    //0x006BC150
    static parking_marker * find_open_parking_marker();

    //0x006CB1A0
    static traffic * get_traffic_from_entity_slow(vhandle_type<entity> a1);

    static void set_traffic_model_usage(int a1, Float a2);

    static bool add_traffic_model(int a1, mString &a2);

    static void advance_traffic(Float a1);

    static bool & traffic_enabled;

    static bool & traffic_initialized;

    static float & traffic_density;

    static float & parking_density;

    static int & lane_changes_this_frame;
    static int & spawned_this_frame;
    static int & unspawned_this_frame;

    static _std::vector<traffic *> & traffic_list;

    static std::array<int, 5> &old_drivers;

    static std::array<int, 5> &new_drivers;

    static traffic *&manual_car;

    static traffic *&getaway_car;

    static traffic *&emergency_car;

    static _std::vector<traffic *> *& ai_occupied_cars;

    static int &living_cars;
    static int &parked_cars;
    static int &visible_cars;

    static int & last_traffic_id;;
};

extern void traffic_patch();
