#pragma once

#include "fixedstring.h"
#include "float.hpp"
#include "variable.h"

struct mString;
struct mission_manager_script_data;
struct resource_key;
struct mission_table_container;
struct region;

struct mission_manager {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    mission_table_container *m_global_table_container;
    mission_table_container *m_district_table_containers[1];
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;
    int field_30;
    int field_34;
    int m_district_table_count;
    mission_manager_script_data *m_script_to_load;
    mission_manager_script_data *m_script;
    int field_44[3];
    bool m_unload_script;
    int field_54;
    int field_58;
    int field_5C;
    float *field_60;
    float field_64;
    int field_68;
    float *field_6C;
    int field_70;
    int field_74;
    float *field_78;
    float *field_7C;
    bool field_80;
    int field_84;
    fixedstring<8> field_88;
    fixedstring<8> field_A8;
    int field_C8;
    bool field_CC;
    fixedstring<8> field_D0;
    int hero_switch_frame;
    float field_F4;
    float field_F8;
    int field_FC;

    //0x005DA010
    mission_manager();

    int get_district_table_count() const
    {
        return this->m_district_table_count;
    }

    mission_table_container *get_district_table(int a2)
    {
        return this->m_district_table_containers[a2];
    }

    mission_table_container *get_global_table()
    {
        return this->m_global_table_container;
    }

    void prepare_unload_script();

    //0x005C5A00
    void force_mission(int a2, const char *a3, int a4, const char *a5);

    //0x005E13D0
    bool get_script(mission_manager_script_data *return_script_data);

    //0x005D7EF0
    void kill_braindead_script();

    //0x005DBCD0
    void sort_district_priorities();

    void sub_5BACA0(Float a2);

    void sub_5BB220(Float a2);

    //0x005E16B0
    void frame_advance(Float a2);

    //0x005DEE40
    void load_script(const mission_manager_script_data &a2);

    //0x005DEFA0
    int run_script(const mission_manager_script_data &arg0);

    //0x005BB410
    void set_real_time();

    //0x005BAE20
    void render_fade();

    //0x005DA4B0
    void show_mission_loading_panel(const mString &a1);

    //0x005C5790
    void update_hero_switch();

    //0x005D1EA0
    int add_global_table(const resource_key &a2);

    //0x005D1EE0
    void add_district_table(void *a2, region *a3);

    void unlock();

    //0x005BB5B0
    void lock();

    void unload_script_now();

    //0x005DBD00
    void unload_script_if_requested();

    static Var<mission_manager *> s_inst;

    static Var<mString> current_mission_debug_title;
};

extern void mission_manager_patch();
