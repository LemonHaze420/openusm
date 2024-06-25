#pragma once

#include "singleton.h"

#include "float.hpp"
#include "game_control.h"
#include "variable.h"

#include <map.hpp>

struct rumble_manager;
struct input_device;
struct device_axis;

const float AXIS_MAX  = 1.0f;
const float AXIS_MID  = 0.0f;
const float AXIS_MIN  = -1.0f;

enum device_id_t {
    INVALID_DEVICE_ID = -1,

    MOUSE1_DEVICE = 11,
};

inline constexpr auto MAX_MOUSE_DEVICES = 1;

inline constexpr auto MAX_KEYBOARD_DEVICES = 1;

struct input_mgr : singleton {
    rumble_manager *rumble_ptr;

    _std::map<device_id_t, input_device *> device_map;
    _std::map<int, game_control> control_map;

    int field_20;

    bool field_24;
    bool field_25;
    bool field_26;
    float (*m_state_callback)(int);
    float (*m_delta_callback)(int);
    void *field_30[1];
    int empty1[7];
    input_device *keyboard_devices[MAX_KEYBOARD_DEVICES];
    input_device *mouse_devices[MAX_MOUSE_DEVICES];
    device_id_t field_58;

    //0x005E0EA0
    input_mgr();

    void * operator new(size_t size);

    void operator delete(void *ptr, size_t size);

    //0x005E0870
    //virtual
    ~input_mgr();

    //0x005DC140
    void insert_device(input_device *a2);

    //0x005D5A60
    void poll_devices();

    //0x005D87C0
    float get_control_delta(int a2, device_id_t a3) const;

    //0x005E0920
    void register_control(const game_control &control);

    //0x00599090
    void scan_devices();

    //0x005DAB20
    void frame_advance(Float a2);

    //0x005D86D0
    float get_control_state(int a2, device_id_t a3) const;

    //0x005BCB40
    void set_control_state_monkey_callback(float (*a2)(int));

    //0x005BCB50
    void set_control_delta_monkey_callback(float (*a2)(int));

    //0x0055E850
    input_device *get_device_from_map(device_id_t id) const;

    //0x005D59B0
    input_device *get_device_from_map_internal(device_id_t a2) const;

    //0x005DC2A0
    void clear_mapping();

    //0x005D8660
    void map_control(int a2, device_id_t a3, int a4);

    //0x005D8610
    void map_control(int a2, const device_axis &a3);

    //0x005EB840
    static void create_inst();

    //0x009685DC
    static inline input_mgr *& instance = var<input_mgr *>(0x009685DC);
};

extern bool & pc_inserted_devices;

extern void input_mgr_patch();
