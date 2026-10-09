#include "threat_assessment_meters.h"

#include "common.h"
#include "fe_mini_map_dot.h"
#include "panelfile.h"
#include "panelquad.h"
#include "trace.h"
#include "custom_math.h"
#include "camera.h"
#include "conglom.h"
#include "entity_base_vhandle.h"
#include "game.h"
#include "geometry_manager.h"
#include "oldmath_po.h"
#include "ngl.h"
#include "os_developer_options.h"
#include "panelanimfile.h"
#include "panelquadsection.h"
#include "wds.h"
#include <algorithm>
#include <cmath>

VALIDATE_SIZE(threat_assessment_meters, 0x1B8u);
VALIDATE_SIZE(threat_assessment_meters::tam_instance, 0x2Cu);

threat_assessment_meters::threat_assessment_meters()
{
    this->field_0 = -1;
    this->field_4 = -1;
    this->field_8 = 1.0;
    this->field_C = 0;
    this->field_10 = 0.30000001;

    this->field_14 = nullptr;
    this->field_134 = 0.0;
    for (auto &instance : field_64) {
        instance.field_0 = false;
        instance.field_18 = 0;
        instance.field_28 = 0;
    }
}

void threat_assessment_meters::init()
{
    TRACE("threat_assessment_meters::init");

    if (!this->field_14) {
        this->field_14 = PanelFile::UnmashPanelFile("tam_poi", static_cast<panel_layer>(7));
        this->field_18[0] = this->field_14->GetPQ("TAM_meter_icon");
        this->field_18[1] = this->field_14->GetPQ("TAM_meter_arrow");
        this->field_18[2] = this->field_14->GetPQ("TAM_meter_gauge_01");
        this->field_18[3] = this->field_14->GetPQ("TAM_meter_gauge_02");
        this->field_18[4] = this->field_14->GetPQ("TAM_meter_gauge_03");
        this->field_18[5] = this->field_14->GetPQ("TAM_meter_gauge_04");

        this->field_30 = this->field_14->field_28.at(0);
        auto v31 = this->field_18[0]->GetCenterX();
        auto v28 = this->field_18[0]->GetCenterY();

        for (int i = 0; i < 6; ++i) {
            float v30;
            float v29;
            this->field_18[i]->GetCenterPos(v30, v29);

            this->field_34[i] = vector2d{v30 - v31, v29 - v28};
        }

        this->field_134 = 30.0f;

        for (int i = 0; i < 4; ++i) {
            auto *mem = mem_alloc(sizeof(fe_mini_map_dot));
            auto *icon = new (mem) fe_mini_map_dot{static_cast<mini_map_dot_type>(5), vector3d{0, 0, 0}};

            assert(icon != nullptr);

            icon->field_24 = false;
            this->field_64[i].field_1C = icon;
        }

        this->field_18[2]->sub_616690(this->field_138, this->field_178);
        this->field_18[3]->sub_616690(this->field_148, this->field_188);
        this->field_18[4]->sub_616690(this->field_158, this->field_198);
        this->field_18[5]->sub_616690(this->field_168, this->field_1A8);
    }
}

void threat_assessment_meters::draw()
{
    if (field_14 == nullptr)
        return;
    if (field_30->field_2D) {
        field_14->Draw();
        return;
    }
    for (auto &position : field_114)
        position[0] = -1.0f;
    const float *base_x[]{field_138, field_148, field_158, field_168};
    const float *base_y[]{field_178, field_188, field_198, field_1A8};
    constexpr float full_u[4][4]{{1, 0, 1, 0}, {1, 1, 0, 0}, {1, 0, 1, 0}, {1, 1, 0, 0}};
    constexpr float full_v[4][4]{{0, 0, 1, 1}, {1, 0, 1, 0}, {0, 0, 1, 1}, {1, 0, 1, 0}};
    for (auto &instance : field_64) {
        if (!instance.field_0)
            continue;
        for (auto *quad : field_18)
            quad->ResetToInitialXY();
        const float fraction = bit_cast<float>(instance.field_8);
        const int quadrant = fraction > 0.75f ? 0 : fraction > 0.5f ? 1 : fraction > 0.25f ? 2 : 3;
        float x[4], y[4], u[4], v[4];
        std::copy_n(base_x[quadrant], 4, x);
        std::copy_n(base_y[quadrant], 4, y);
        std::copy_n(full_u[quadrant], 4, u);
        std::copy_n(full_v[quadrant], 4, v);
        constexpr float midpoints[]{0.875f, 0.625f, 0.375f, 0.125f};
        constexpr float ends[]{0.75f, 0.5f, 0.25f, 0.0f};
        const bool first_half = fraction > midpoints[quadrant];
        const float angle = (fraction - (first_half ? midpoints[quadrant] : ends[quadrant])) * 6.2831855f;
        float sine, cosine;
        fast_sin_cos_approx(angle, &sine, &cosine);
        const float amount = 1.0f - sine / cosine;
        const auto interpolate = [amount](float from, float to) {
            return from + (to - from) * amount;
        };
        if (quadrant == 0) {
            if (first_half) {
                v[0] = interpolate(v[0], v[2]);
                y[0] = interpolate(y[0], y[2]);
            } else {
                v[0] = v[2];
                y[0] = y[2];
                u[2] = interpolate(u[2], u[3]);
                u[0] = u[2];
                x[2] = interpolate(x[2], x[3]);
                x[0] = x[2];
            }
        } else if (quadrant == 1) {
            if (first_half) {
                u[0] = interpolate(u[0], u[2]);
                x[0] = interpolate(x[0], x[2]);
            } else {
                u[0] = u[2];
                x[0] = x[2];
                v[2] = interpolate(v[2], v[3]);
                v[0] = v[2];
                y[2] = interpolate(y[2], y[3]);
                y[0] = y[2];
            }
        } else if (quadrant == 2) {
            if (first_half) {
                v[3] = interpolate(v[3], v[1]);
                y[3] = interpolate(y[3], y[1]);
            } else {
                v[3] = v[1];
                y[3] = y[1];
                u[1] = interpolate(u[1], u[0]);
                u[3] = u[1];
                x[1] = interpolate(x[1], x[0]);
                x[3] = x[1];
            }
        } else {
            if (first_half) {
                u[3] = interpolate(u[3], u[1]);
                x[3] = interpolate(x[3], x[1]);
            } else {
                u[3] = u[1];
                x[3] = x[1];
                v[1] = interpolate(v[1], v[0]);
                v[3] = v[1];
                y[1] = interpolate(y[1], y[0]);
                y[3] = y[1];
            }
        }
        auto *section = bit_cast<nglQuad *>(&field_18[quadrant + 2]->pqs.at(0)->field_14);
        for (int vertex = 0; vertex < 4; ++vertex) {
            section->field_0[vertex].uv.field_0 = u[vertex];
            section->field_0[vertex].uv.field_4 = v[vertex];
            nglSetQuadVPos(section, vertex, x[vertex], y[vertex]);
        }
        for (int i = 0; i < 4; ++i)
            field_18[i + 2]->TurnOn(i >= quadrant);
        instance.field_20 = false;
        float rotation = 1.5f;
        entity_base_vhandle handle{static_cast<uint32_t>(instance.field_18)};
        auto *owner = handle.get_volatile_ptr();
        if (owner != nullptr)
            instance.field_C = owner->get_abs_position();
        const auto camera_position = g_world_ptr->get_chase_cam_ptr(0)->get_abs_position();
        float near_distance = os_developer_options::instance->get_int(static_cast<os_developer_options::ints_t>(55));
        float far_distance = os_developer_options::instance->get_int(static_cast<os_developer_options::ints_t>(56));
        float minimum_scale =
            os_developer_options::instance->get_int(static_cast<os_developer_options::ints_t>(57)) * 0.01f;
        if (near_distance < 1.0f || far_distance <= near_distance || minimum_scale <= 0.0f || minimum_scale > 1.0f) {
            near_distance = 1.0f;
            far_distance = 11.0f;
            minimum_scale = 0.5f;
        }
        const float distance = std::clamp((instance.field_C - camera_position).length(), near_distance, far_distance);
        const float scale = (1.0f - (distance - near_distance) / far_distance) * (1.0f - minimum_scale) + minimum_scale;
        auto position = instance.field_C;
        if (owner != nullptr && owner->is_an_actor()) {
            static const string_hash marker_id{"TAM_MARKER"};
            auto *marker =
                (owner->field_4 & 4) ? static_cast<conglomerate *>(owner)->get_member(marker_id, true) : nullptr;
            if (marker != nullptr)
                position = marker->get_abs_position();
            else
                position.y += owner->get_visual_radius();
        }
        const auto projected = geometry_manager::get_xform(static_cast<geometry_manager::xform_t>(4)) * position;
        auto screen = sub_501B20(geometry_manager::get_xform(static_cast<geometry_manager::xform_t>(5)), projected);
        const float left = 50.0f + field_134 * 0.5f;
        const float right = 590.0f - field_134 * 0.5f;
        const float top = 40.0f + field_134 * 0.5f;
        const float bottom = 440.0f - field_134 * 0.5f;
        if (screen.x < left || screen.x > right || projected.z < 0.0f) {
            auto *camera = g_game_ptr->get_current_view_camera(0);
            const auto &pose = camera->get_abs_po();
            auto x_axis = pose.get_x_facing();
            auto z_axis = pose.get_z_facing();
            if (camera->field_41 != -1) {
                x_axis = -pose.get_y_facing();
                z_axis = -z_axis;
            }
            const auto direction = position - camera_position;
            const float bearing = -std::atan2(-dot(x_axis, direction), dot(z_axis, direction));
            const float half_fov = geometry_manager::get_field_of_view() * 0.5f * 0.84f;
            rotation = bearing >= 0.0f ? 3.1415927f : 0.0f;
            po turn{po_identity_matrix};
            turn.set_rot(pose.get_y_facing(), bearing >= 0.0f ? bearing - half_fov : bearing + half_fov);
            const auto rotated = turn.non_affine_slow_xform(direction) + camera_position;
            screen = sub_501B20(geometry_manager::get_xform(static_cast<geometry_manager::xform_t>(7)), rotated);
            instance.field_20 = true;
        }
        if (screen.y < top) {
            screen.y = top;
            rotation = 1.5707964f;
            instance.field_20 = true;
        } else if (screen.y > bottom) {
            screen.y = bottom;
            rotation = 4.712389f;
            instance.field_20 = true;
        }
        if (screen.x >= left && screen.x <= right) {
            const auto collides = [this, &screen](float x) {
                for (const auto &used : field_114)
                    if (bit_cast<uint32_t>(used[0]) != 0xBF800000u &&
                        field_134 * field_134 >
                            (used[0] - x) * (used[0] - x) + (used[1] - screen.y) * (used[1] - screen.y))
                        return true;
                return false;
            };
            float step = -field_134;
            float left_x = screen.x;
            while (collides(left_x)) {
                left_x += step;
                if (left_x < left) {
                    step = field_134;
                    left_x = screen.x + step;
                }
            }
            step = field_134;
            float right_x = screen.x;
            while (collides(right_x)) {
                right_x += step;
                if (right_x > right) {
                    step = -field_134;
                    right_x = screen.x + step;
                }
            }
            screen.x = std::abs(left_x - screen.x) < std::abs(right_x - screen.x) ? left_x : right_x;
        }
        for (auto &used : field_114) {
            if (bit_cast<uint32_t>(used[0]) == 0xBF800000u) {
                used = {screen.x, screen.y};
                break;
            }
        }
        instance.field_24 = bit_cast<int>(rotation);
        for (int i = 0; i < 6; ++i) {
            const float x = screen.x + scale * field_34[i][0];
            const float y = screen.y + scale * field_34[i][1];
            field_18[i]->sub_616710(x - field_18[i]->field_14[0], y - field_18[i]->field_14[1]);
            field_18[i]->Scale(scale, true);
        }
        instance.field_1C->field_14 = instance.field_C;
        field_18[1]->TurnOn(instance.field_20);
        field_18[1]->Rotate(field_18[0]->GetCenterX(), field_18[0]->GetCenterY(), rotation, true);
        field_14->Draw();
        for (auto *quad : field_18)
            quad->TurnOn(true);
        field_18[1]->TurnOn(false);
        for (int i = 0; i < 4; ++i) {
            auto *quad = bit_cast<nglQuad *>(&field_18[i + 2]->pqs.at(0)->field_14);
            for (int vertex = 0; vertex < 4; ++vertex) {
                quad->field_0[vertex].uv.field_0 = full_u[i][vertex];
                quad->field_0[vertex].uv.field_4 = full_v[i][vertex];
                nglSetQuadVPos(quad, vertex, base_x[i][vertex], base_y[i][vertex]);
            }
        }
    }
}

void threat_assessment_meters_patch()
{
    {
        FUNC_ADDRESS(address, &threat_assessment_meters::init);
        REDIRECT(0x00647E26, address);
    }
}
