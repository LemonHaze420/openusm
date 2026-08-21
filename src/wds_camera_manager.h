#pragma once

#include "float.hpp"

#include <vector.hpp>

#include <cstdint>

struct marky_camera;
struct motion_control_system;
struct camera;
struct controller;

struct wds_camera_manager {
    _std::vector<motion_control_system *> field_0;
    _std::vector<controller *> field_10;
    camera *field_20;
    motion_control_system *field_24;
    motion_control_system *field_28;
    controller *field_2C;
    camera *field_30;
    motion_control_system *field_34;
    motion_control_system *field_38;
    controller *field_3C;
    camera *field_40;
    marky_camera *field_44;
    bool field_48;

    //0x0053DB80
    wds_camera_manager();

    bool is_marky_cam_enabled() const
    {
        return this->field_48;
    }

    //0x00542630
    int add_controller(controller *a2);

    //0x005426A0
    int add_mcs(motion_control_system *a2);

    //0x00530A70
    void advance_controllers(Float a2);

    //0x0054B8A0
    void setup_cameras();

    //0x00530A30
    void frame_advance(Float a2);

    //0x0050D480
    void usercam_frame_advance(Float a2);

    //0x0051EA10
    void scene_analyzer_frame_advance(Float a2);

    //0x0050D4C0
    void enable_marky_cam(bool a2, Float a3);
};

extern void wds_camera_manager_patch();
