#pragma once

#include "mashable_vector.h"
#include "variable.h"
#include "vector3d.h"

#include <vector.hpp>

struct mission_condition;
struct po;
struct region;

struct mission_marker_base;
struct mission_camera_marker;
struct mission_transform_marker;
struct mission_camera_transform_marker;

struct mission_manager_script_data;

struct mission_table_game_state_entry {
    float *field_0;
    union {
        float f;
        float *p;
    } field_4;
    uint32_t field_8;

    //0x005BBA00
    bool check() const;
};

struct mission_condition_instance {
    uint32_t sentinel;
    po *field_4;
    vector3d field_8;
    const char *key_name;
    const char *script_data_name;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;
    int field_30;
    int field_34;
    int field_38;
    float *nums;
    int num_nums;
    const char **strings;
    int num_strings;
    vector3d *positions;
    int num_positions;
    unsigned int field_54;
    mashable_vector<mission_table_game_state_entry> field_58;
    int field_60;
    int field_64;

    bool is_flag_set(uint32_t a2) const
    {
        return (a2 & this->field_54) != 0;
    }

    int get_patrol_num() const
    {
        return this->field_60;
    }

    bool get_key_po(const mission_condition *a2, po *p) const;

    bool check_game_state() const;

    const char *get_script_data_name() const;

    bool append_nums(const mission_condition *a2, _std::vector<float> *num_list) const;

    bool append_strings(const mission_condition *a2, _std::vector<mString> *str_list) const;
};

struct mission_condition {
    float field_0;
    float field_4;
    mashable_vector<mission_condition_instance> instances;
    mashable_vector<mission_table_game_state_entry> field_10;
    const char *field_18;
    const char *field_1C;
    unsigned char field_20;
    int field_24;
    unsigned int field_28;
    const char *field_2C;
    const char *field_30;

    bool is_flag_set(uint32_t a2) const
    {
        return (a2 & this->field_28) != 0;
    }

    bool get_key_po(int instance, po *p) const;

    mission_condition_instance *find_best_instance(mission_manager_script_data *data) const;

    bool applies_to_current_hero() const;

    bool check_condition(mission_manager_script_data *data) const;

    bool append_nums(int instance, _std::vector<float> *nums) const;
};


struct mission_table_container {
    mashable_vector<mission_marker_base> field_0;
    mashable_vector<mission_camera_marker> multi_array_camera_markers;
    mashable_vector<mission_transform_marker> field_10;
    mashable_vector<mission_camera_transform_marker> field_18;
    mashable_vector<float> multi_array_nums;
    mashable_vector<char *> multi_array_strings;
    mashable_vector<vector3d> multi_array_positions;
    mashable_vector<mission_condition> field_38;
    int field_40;
    region *field_44;

    struct script_info {
        const char *field_0;
        mission_condition_instance *field_4;
        int field_8;
    };

    region *get_region()
    {
        return this->field_44;
    }

    bool append_script_info(_std::vector<mission_table_container::script_info> *info);

    bool append_nums(const char *a2, int a3, _std::vector<float> *nums) const;

    //0x005C6010
    void un_mash(generic_mash_header *a2, void *a3, void *a4, generic_mash_data_ptrs *a5);
};

inline Var<mission_table_container *> s_current_un_mashing_mission_table_container{0x00968524};
