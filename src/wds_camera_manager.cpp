#include "wds_camera_manager.h"

#include "camera.h"
#include "common.h"
#include "controller.h"
#include "dolly_and_strafe_mcs.h"
#include "femanager.h"
#include "func_wrapper.h"
#include "frontendmenusystem.h"
#include "game.h"
#include "lookat_target_controller.h"
#include "marky_camera.h"
#include "mic.h"
#include "motion_control_system.h"
#include "mouselook_controller.h"
#include "sniper_camera.h"
#include "theta_and_psi_mcs.h"
#include "trace.h"
#include "wds.h"

#include <cassert>

VALIDATE_SIZE(wds_camera_manager, 0x4C);

wds_camera_manager::wds_camera_manager()
{
    this->field_0 = {};
    this->field_10 = {};
    this->field_20 = nullptr;
    this->field_30 = nullptr;
    this->field_24 = nullptr;
    this->field_28 = nullptr;
    this->field_2C = nullptr;
    this->field_34 = nullptr;
    this->field_38 = nullptr;
    this->field_3C = nullptr;
    this->field_40 = nullptr;
    this->field_44 = nullptr;
    this->field_48 = false;
}

void wds_camera_manager::frame_advance(Float a2)
{
    if constexpr (1) {
        struct Vtbl {
            int field_0;
            bool(__fastcall *is_active)(void *);
            int field_8;
            void(__fastcall *frame_advance)(void *, void *, Float);
        };

        for (auto &mcs : this->field_0) {
            Vtbl *vtbl = CAST(vtbl, mcs->m_vtbl);

            if (vtbl->is_active(mcs)) {
                vtbl->frame_advance(mcs, nullptr, a2);
            }
        }
    } else {
        THISCALL(0x00530A30, this, a2);
    }
}

int wds_camera_manager::add_controller(controller *a2)
{
    TRACE("wds_camera_manager::add_controller");

    if constexpr (0) {
    } else {
        return THISCALL(0x00542630, this, a2);
    }
}

int wds_camera_manager::add_mcs(motion_control_system *a2)
{
    if constexpr (0) {
    } else {
        return THISCALL(0x005426A0, this, a2);
    }
}

void wds_camera_manager::advance_controllers(Float a2)
{
    TRACE("wds_camera_manager::advance_controllers");

    if constexpr (0) {
        for (auto &cntrl : this->field_10) {
            if (cntrl->field_4) {
                struct Vtbl {
                    int field_0;
                    void(__fastcall *frame_advance)(void *, void *, Float);
                };

                Vtbl *vtbl = CAST(vtbl, cntrl->m_vtbl);
                vtbl->frame_advance(cntrl, nullptr, a2);
            }
        }
    } else {
        THISCALL(0x00530A70, this, a2);
    }
}

static Var<theta_and_psi_mcs *> g_theta_and_psi_mcs{0x0095C73C};

void wds_camera_manager::setup_cameras()
{
    TRACE("wds_camera_manager::setup_cameras");

    if constexpr (0) {
        vector3d v46{ZEROVEC};

        auto *v2 = g_world_ptr->get_hero_ptr(0);
        if (v2 != nullptr) {
            v46 = v2->get_abs_position();
        }

        auto *user_cam = new camera{nullptr, string_hash{"USER_CAM"}};

        user_cam->set_abs_position(v46);
        g_world_ptr->ent_mgr.add_camera(nullptr, user_cam);

        auto *v14 = new theta_and_psi_mcs{user_cam, 0.0, 0.0};
        g_theta_and_psi_mcs() = v14;
        this->add_mcs(v14);

        auto *v16 = new dolly_and_strafe_mcs{user_cam};
        this->add_mcs(v16);

        auto *v19 = new mouselook_controller{v16, v14, nullptr};
        g_mouselook_controller() = v19;
        this->add_controller(v19);

        auto *v21 = new lookat_target_controller{user_cam};
        g_lookat_controller() = v21;
        this->add_controller(v21);

        this->field_28 = v14;
        this->field_24 = v16;
        this->field_2C = v19;
        this->field_30 = user_cam;

        if (g_femanager.m_fe_menu_system != nullptr) {
            g_femanager.m_fe_menu_system->RenderLoadMeter(0);
        }

        system_idle();

        auto *v25 = new mic{g_world_ptr->get_hero_ptr(0), string_hash{"BOOM_MIC"}};
        g_world_ptr->ent_mgr.add_mic(nullptr, v25);

        this->field_20 = new sniper_camera{string_hash{"SNIPER_CAM"}, g_world_ptr->get_hero_ptr(0)};
        g_world_ptr->ent_mgr.add_camera(nullptr, this->field_20);

        this->field_44 = new marky_camera{string_hash{"MARKY_CAM"}};
        g_world_ptr->ent_mgr.add_camera(nullptr, this->field_44);
        g_world_ptr->set_chase_cam_ptr(0, this->field_44);

        g_femanager.RenderLoadMeter(false);

        auto *scene_analyzer_cam = new camera{nullptr, string_hash{"SCENE_ANALYZER_CAM"}};
        scene_analyzer_cam->set_abs_position(v46);

        g_world_ptr->ent_mgr.add_camera(nullptr, scene_analyzer_cam);

        auto *a2 = new theta_and_psi_mcs{scene_analyzer_cam, 0.0, 0.0};
        this->add_mcs(a2);

        auto *v39 = new dolly_and_strafe_mcs{scene_analyzer_cam};
        this->add_mcs(v39);

        auto *v1 = new mouselook_controller{v39, a2, nullptr};
        this->add_controller(v1);

        this->field_40 = scene_analyzer_cam;
        this->field_38 = v39;
        this->field_3C = v1;
        this->field_34 = a2;
    } else {
        THISCALL(0x0054B8A0, this);
    }
}

void wds_camera_manager::usercam_frame_advance(Float a2)
{
    TRACE("wds_camera_manager::usercam_frame_advance");

    THISCALL(0x0050D480, this, a2);
}

void wds_camera_manager::scene_analyzer_frame_advance(Float a2)
{
    TRACE("wds_camera_manager::scene_analyzer_frame_advance");

    THISCALL(0x0051EA10, this, a2);
}

void wds_camera_manager::enable_marky_cam(bool a2, Float a3)
{
    assert(this->field_44 != nullptr);

    THISCALL(0x0050D4C0, this, a2, a3);
}

void wds_camera_manager_patch()
{
    {
        FUNC_ADDRESS(address, &wds_camera_manager::add_controller);
        REDIRECT(0x0054BAA8, address);
        REDIRECT(0x0054BAE3, address);
        REDIRECT(0x0054BDEA, address);
    }

    {
        FUNC_ADDRESS(address, &wds_camera_manager::advance_controllers);
        REDIRECT(0x0055841B, address);
    }
}
