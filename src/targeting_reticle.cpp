#include "targeting_reticle.h"

#include "common.h"
#include "panelfile.h"
#include "panelquad.h"
#include "trace.h"
#include "camera.h"
#include "conglom.h"
#include "entity_base_vhandle.h"
#include "geometry_manager.h"
#include "oldmath_po.h"
#include "os_developer_options.h"
#include "variable.h"
#include "wds.h"
#include <algorithm>

VALIDATE_SIZE(targeting_reticle, 0x1Cu);

targeting_reticle::targeting_reticle()
{
    this->field_4 = 0;
    this->field_8 = nullptr;
    this->field_0 = false;
}

void targeting_reticle::init()
{
    if (this->field_8 == nullptr) {
        this->field_8 = PanelFile::UnmashPanelFile("targeting_reticle", static_cast<panel_layer>(7));
        this->field_C = this->field_8->GetPQ("TR_arrow_top_left");
        this->field_10 = this->field_8->GetPQ("TR_arrow_top_right");
        this->field_14 = this->field_8->GetPQ("TR_arrow_bottom_left");
        this->field_18 = this->field_8->GetPQ("TR_arrow_bottom_right");
        this->field_4 = 0;
        this->field_0 = false;
    }
}

void targeting_reticle::draw()
{
    if (!field_0 || field_8 == nullptr)
        return;
    PanelQuad *quads[]{field_C, field_10, field_14, field_18};
    for (auto *quad : quads)
        quad->TurnOn(true);
    auto *camera = g_world_ptr->get_chase_cam_ptr(0);
    const auto camera_position = camera->get_abs_position();
    vector3d forward = camera->get_abs_po().get_z_facing();
    forward.y = 0.0f;
    if (forward.length2() > 0.0001f)
        forward.normalize();
    float near_distance = os_developer_options::instance->get_int(static_cast<os_developer_options::ints_t>(61));
    float far_distance = os_developer_options::instance->get_int(static_cast<os_developer_options::ints_t>(62));
    float minimum_scale =
        os_developer_options::instance->get_int(static_cast<os_developer_options::ints_t>(63)) * 0.01f;
    if (near_distance < 1.0f || far_distance <= near_distance || minimum_scale <= 0.0f || minimum_scale > 1.0f) {
        near_distance = 1.0f;
        far_distance = 11.0f;
        minimum_scale = 0.5f;
    }
    auto position = var<vector3d>(0x0096B450);
    float radius = -1.0f;
    entity_base_vhandle handle{static_cast<uint32_t>(field_4)};
    auto *owner = handle.get_volatile_ptr();
    if (owner != nullptr) {
        position = owner->get_abs_position();
        entity_base *neck = nullptr;
        if (owner->field_4 & 4) {
            static const string_hash neck_id{"BIP01 NECK"};
            neck = static_cast<conglomerate *>(owner)->get_bone(neck_id, true);
        }
        if (neck != nullptr) {
            position = neck->get_abs_position();
            radius = owner->get_visual_radius() * 0.5f;
        } else if (owner->is_an_actor()) {
            radius = owner->get_visual_radius() * 0.5f;
            position.y += radius * 0.5f;
            minimum_scale = std::min(minimum_scale * 3.0f, 1.0f);
        }
    }
    const float distance = std::clamp((position - camera_position).length(), near_distance, far_distance);
    const float scale = (1.0f - (distance - near_distance) / far_distance) * (1.0f - minimum_scale) + minimum_scale;
    const vector3d horizontal{-forward.z * radius * 0.5f, 0.0f, forward.x * radius * 0.5f};
    vector3d corners[]{position + horizontal, position - horizontal, position + horizontal, position - horizontal};
    corners[0].y += 0.5f;
    corners[1].y += 0.5f;
    corners[2].y -= radius;
    corners[3].y -= radius;
    const auto &projection = geometry_manager::get_xform(static_cast<geometry_manager::xform_t>(4));
    const auto &screen_transform = geometry_manager::get_xform(static_cast<geometry_manager::xform_t>(5));
    for (auto &corner : corners) {
        const auto projected = projection * corner;
        field_0 = projected.z >= 0.0f;
        corner = sub_501B20(screen_transform, projected);
    }
    const float inset = std::min((corners[1].x - corners[0].x) * 0.5f, 30.0f) * scale;
    auto &frame = var<float>(0x0096B44C);
    float fraction = 0.0f;
    if (frame >= 0.0f && frame < 2.0f)
        fraction = frame * 0.5f;
    else if (frame >= 2.0f && frame < 4.0f)
        fraction = 1.0f - (frame - 2.0f) * 0.5f;
    for (int i = 0; i < 4; ++i) {
        corners[i].x += (i & 1 ? -inset : inset) * fraction;
        corners[i].y += (i < 2 ? inset : -inset) * fraction;
        quads[i]->SetCenterPos(corners[i].x, corners[i].y);
        quads[i]->Scale(scale, true);
    }
    if (corners[0].z <= 0.0f || corners[3].z <= 0.0f)
        for (auto *quad : quads)
            quad->TurnOn(false);
    if (frame < 4.0f)
        frame += 1.0f;
    if (field_0)
        field_8->Draw();
    else
        field_0 = true;
}
