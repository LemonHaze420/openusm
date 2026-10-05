#include "motion_effect_struct.h"
#include "actor.h"
#include "advanced_entity_ptrs.h"
#include "camera.h"
#include "common.h"
#include "entity.h"
#include "fixedstring.h"
#include "game.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "ngl_vertexdef.h"
#include "oldmath_po.h"
#include "spline.h"
#include "time_interface.h"
#include "us_pcuv_shader.h"
#include "variable.h"
#include "vector4d.h"
#include "wds.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>

VALIDATE_SIZE(motion_effect_struct, 0x30u);
VALIDATE_OFFSET(motion_effect_struct, trail, 0x1C);
VALIDATE_OFFSET(motion_effect_struct, distorted_trail_active, 0x2D);
VALIDATE_SIZE(motion_pose_sample, 0x1C);
VALIDATE_SIZE(motion_trail_info, 0x3C);
VALIDATE_SIZE(motion_distorted_trail_info, 0x48);

namespace {
Var<motion_effect_struct *> active{0x0095A700};
Var<motion_effect_struct *> inactive{0x0095A6FC};
Var<int> spline_references{0x0095A704};
Var<int> effect_count{0x0095A71C};
Var<spline *> trail_spline{0x0095A708}, trail_spline2{0x0095A70C};
Var<spline *> distorted_spline{0x0095A710}, distorted_spline2{0x0095A714}, distorted_spline3{0x0095A718};
Var<PCUV_ShaderMaterial *> blend_material{0x0095A720}, add_material{0x0095A724};
Var<PCUV_ShaderMaterial *> blend_fb_material{0x0095A728}, add_fb_material{0x0095A72C};

#if STANDALONE_SYSTEM

[[maybe_unused]] const bool motion_tuning_initialized = [] {
    var<int>(0x00921B1C) = 4;
    var<int>(0x00921B20) = 3;
    var<float>(0x00921B4C) = 0.7f;
    var<float>(0x00921B50) = 12.0f;
    var<float>(0x00921B54) = 2.0f;
    var<float>(0x00921B58) = 2.0f;
    return true;
}();
#endif

void unlink_effect(motion_effect_struct *effect)
{
    if (effect->previous != nullptr)
        effect->previous->next = effect->next;
    else if (inactive() == effect)
        inactive() = effect->next;
    else if (active() == effect)
        active() = effect->next;
    if (effect->next != nullptr)
        effect->next->previous = effect->previous;
    effect->next = effect->previous = nullptr;
}

template <class T>
void delete_history(T *&info)
{
    if (info != nullptr) {
        delete[] info->samples;
        delete info;
        info = nullptr;
    }
}

vector3d axis_offset(entity_base *entity, int axis, float width)
{
    return vector3d{entity->get_abs_po().m[axis][0], entity->get_abs_po().m[axis][1], entity->get_abs_po().m[axis][2]} *
           width;
}

void scale_motion_points(entity_base *source, std::initializer_list<vector3d *> points)
{
    if ((source->field_4 & 0x8000) == 0)
        return;
    auto *root = source->get_conglom_owner();
    if (root == nullptr)
        return;
    auto *actor_root = static_cast<actor *>(root);
    if (actor_root->adv_ptrs == nullptr || actor_root->adv_ptrs->field_8 == nullptr)
        return;
    const auto &scale = actor_root->adv_ptrs->field_8->m_scale;
    const auto &transform = root->get_abs_po();
    for (auto *destination : points) {
        auto point = transform.inverse_xform(*destination);
        for (int axis = 0; axis != 3; ++axis)
            point[axis] *= scale[axis];
        *destination = transform.slow_xform(point);
    }
}

motion_trail_sample current_trail_sample(motion_trail_info *info)
{
    motion_trail_sample sample;
    if (info->single_entity) {
        const auto offset =
            info->axis == 0
                ? vector3d{0.0f, 0.0f, 0.0f}
                : axis_offset(info->first, info->axis >= 1 && info->axis <= 3 ? info->axis - 1 : 0, info->width);
        const auto position = info->first->get_abs_position();
        sample = {position + offset, position - offset};
    } else {
        sample = {info->first->get_abs_position(), info->second->get_abs_position()};
        scale_motion_points(info->first, {&sample.first, &sample.second});
    }
    return sample;
}

motion_distorted_sample current_distorted_sample(motion_distorted_trail_info *info, bool rendering)
{
    motion_distorted_sample sample;
    if (info->single_entity) {
        const auto offset =
            axis_offset(info->owner, info->axis >= 1 && info->axis <= 3 ? info->axis - 1 : 0, info->width);
        const auto position = info->owner->get_abs_position();
        sample.first = position + offset;
        sample.second = position - offset;
        if (rendering)
            sample.third = sample.second;
    } else {
        const auto first = info->first->get_abs_position();
        const auto second = info->second->get_abs_position();
        const auto delta = first - second;
        const float divisor = var<float>(rendering ? 0x00921B54 : 0x00921B4C);
        sample.second = first - delta / divisor;
        sample.third = second + delta / divisor;
        if (rendering) {
            const auto position = info->owner->get_abs_position();
            sample.first =
                position - (position - first) / var<float>(0x00921B58) - (position - second) / var<float>(0x00921B58);
        } else {
            sample.first = info->samples[info->next_sample].first;
        }
        scale_motion_points(info->owner, {&sample.first, &sample.second, &sample.third});
    }
    return sample;
}

void build_trail_spline(spline *curve)
{
    curve->build(8, static_cast<spline::eSplineType>(2));
}

uint32_t packed_color(color32 color)
{
    return static_cast<uint32_t>(color32::to_int(color));
}

float motion_distance(const vector3d &first, const vector3d &second)
{
    const double x = static_cast<double>(first.x) - second.x;
    const double y = static_cast<double>(first.y) - second.y;
    const double z = static_cast<double>(first.z) - second.z;
    return static_cast<float>(std::sqrt(x * x + y * y + z * z));
}

void submit_triangle(PCUV_ShaderMaterial *material, const vector3d (&positions)[3], const vector2d (&uv)[3],
                     const uint32_t (&colors)[3])
{
    auto *base = material != nullptr ? reinterpret_cast<nglMaterialBase *>(&material->field_4) : nullptr;
    nglAddPCUVTriangle(base, positions, uv, colors);
}

void close_motion_mesh(const vector3d &center, float radius)
{
    nglMeshSetSphere({center}, radius);
    nglListAddMesh(nglCloseMesh(), {identity_matrix}, nullptr, nullptr);
}
}  // namespace

motion_effect_struct::motion_effect_struct(entity_base_vhandle handle, const mString &texture)
    : next(nullptr), previous(nullptr), owner(handle), pose_history(nullptr), trail(nullptr), distorted_trail(nullptr),
      afterimage(nullptr), draining_trail(false), trail_active(false), field_2A(false), pose_recording(false),
      draining_distorted_trail(false), distorted_trail_active(false), field_2E(false), field_2F(false)
{
    if (spline_references()++ == 0) {
        trail_spline() = new spline;
        trail_spline2() = new spline;
        distorted_spline() = new spline;
        distorted_spline2() = new spline;
        distorted_spline3() = new spline;
    }
    remove_from_list();
    if (effect_count()++ == 0) {
        blend_material() = new PCUV_ShaderMaterial(
            nglLoadTexture(tlFixedString{texture.c_str()}), static_cast<nglBlendModeType>(2), 0, 2);
        add_material() = new PCUV_ShaderMaterial(
            nglLoadTexture(tlFixedString{texture.c_str()}), static_cast<nglBlendModeType>(3), 0, 2);
        blend_fb_material() = new PCUV_ShaderMaterial(nglGetBackBufferTex(), static_cast<nglBlendModeType>(2), 0, 194);
        add_fb_material() = new PCUV_ShaderMaterial(nglGetBackBufferTex(), static_cast<nglBlendModeType>(3), 0, 194);
    }
}

motion_effect_struct::~motion_effect_struct()
{
    delete afterimage;
    delete_history(pose_history);
    delete_history(trail);
    delete_history(distorted_trail);
    if (--spline_references() == 0) {
        delete trail_spline();
        trail_spline() = nullptr;
        delete trail_spline2();
        trail_spline2() = nullptr;
        delete distorted_spline();
        distorted_spline() = nullptr;
        delete distorted_spline2();
        distorted_spline2() = nullptr;
        delete distorted_spline3();
        distorted_spline3() = nullptr;
    }
    unlink_effect(this);
    if (--effect_count() == 0) {
        for (auto *material : {blend_material(), add_material(), blend_fb_material(), add_fb_material()}) {
            if (material != nullptr) {
                if (material->m_texture != nullptr)
                    nglReleaseTexture(material->m_texture);
                delete material;
            }
        }
        blend_material() = add_material() = blend_fb_material() = add_fb_material() = nullptr;
    }
}

void motion_effect_struct::activate_trail(entity_base *source, int axis, float width, color32 color, int alpha,
                                          float interval, int samples, bool additive)
{
    trail_active = true;
    draining_trail = false;
    if (!trail) {
        trail = new motion_trail_info{};
        trail->field_0 = samples;
        trail->samples = new motion_trail_sample[samples];
    }
    trail->first = source;
    trail->second = nullptr;
    const auto position = source->get_abs_position();
    trail->samples[0] = {position, position};
    trail->axis = axis;
    trail->additive = additive;
    trail->width = width;
    trail->first_color = color;
    trail->second_color = color;
    trail->alpha = static_cast<uint8_t>(alpha);
    trail->interval = interval;
    trail->remaining = interval;
    trail->capacity = samples;
    trail->sample_count = 0;
    trail->next_sample = 0;
    trail->single_entity = true;
    remove_from_list();
}

void motion_effect_struct::remove_from_list()
{
    unlink_effect(this);
    auto &head =
        pose_recording || trail_active || distorted_trail_active || (afterimage != nullptr && afterimage->active)
            ? active()
            : inactive();
    next = head;
    head = this;
    if (next != nullptr)
        next->previous = this;
}

void motion_effect_struct::render_all_motion_fx(camera &, hull &)
{
    for (auto *effect = active(); effect != nullptr;) {
        auto *current = effect;
        effect = current->next;
        if (current->trail_active)
            current->render_trail();
        if (current->distorted_trail_active)
            current->render_distorted_trail();
    }
}

void motion_effect_struct::render_trail(vector3d a, vector3d b, vector3d c, vector2d ua, vector2d ub, vector2d uc,
                                        color32 ca, color32 cb, color32 cc, bool additive, vector3d, vector3d)
{
    const vector3d positions[3]{a, b, c};
    const vector2d uv[3]{ua, ub, uc};
    const uint32_t colors[3]{packed_color(ca), packed_color(cc), packed_color(cb)};
    submit_triangle(additive ? add_material() : blend_material(), positions, uv, colors);
}

void motion_effect_struct::render_distorted_trail(const vector3d &a, const vector3d &b, const vector3d &c,
                                                  const vector4d &ua, const vector4d &ub, const vector4d &uc,
                                                  color32 ca, color32 cb, color32 cc, bool additive, vector3d &minimum,
                                                  vector3d &maximum)
{
    const auto jitter = [](const vector4d &source) {
        const int y_range = var<int>(0x00921B20);
        const int x_range = var<int>(0x00921B1C);
        const float y = source.y + static_cast<float>(std::rand() % y_range - y_range / 2);
        const float x = source.x + static_cast<float>(std::rand() % x_range - x_range / 2);
        return vector2d{x, y};
    };
    const vector3d positions[3]{a, b, c};
    const vector2d uv[3]{jitter(ua), jitter(ub), jitter(uc)};
    const uint32_t colors[3]{packed_color(ca), packed_color(cc), packed_color(cb)};
    submit_triangle(additive ? add_fb_material() : blend_fb_material(), positions, uv, colors);
    const double center[3]{(static_cast<double>(minimum.x) + maximum.x) * 0.5,
                           (static_cast<double>(minimum.y) + maximum.y) * 0.5,
                           (static_cast<double>(minimum.z) + maximum.z) * 0.5};
    const float radius = motion_distance(minimum, maximum);
    for (int axis = 0; axis != 3; ++axis) {
        if (center[axis] - radius < minimum[axis])
            minimum[axis] = static_cast<float>(center[axis] - radius);
        if (center[axis] + radius > maximum[axis])
            maximum[axis] = static_cast<float>(center[axis] + radius);
    }
}

void motion_effect_struct::render_trail()
{
    if (draining_trail && --trail->sample_count <= 0) {
        trail_active = draining_trail = false;
        delete_history(trail);
        remove_from_list();
        return;
    }
    if (trail->sample_count <= 1)
        return;
    auto *first = trail_spline();
    auto *second = trail_spline2();
    first->reserve_control_pts(trail->sample_count + 1);
    second->reserve_control_pts(trail->sample_count + 1);
    const auto current = current_trail_sample(trail);
    first->add_control_pt(current.first);
    second->add_control_pt(current.second);
    int index = trail->next_sample;
    for (int i = 0; i != trail->sample_count; ++i) {
        if (index <= 0)
            index = trail->capacity;
        const auto &sample = trail->samples[--index];
        first->add_control_pt(sample.first);
        second->add_control_pt(sample.second);
    }
    build_trail_spline(first);
    build_trail_spline(second);
    const int count = static_cast<int>(first->curve_pts.size());
    nglCreateMesh(0x40000, 2 * count - 2, 0, nullptr);
    color32 ca = trail->first_color, cb = trail->second_color;
    ca.set_alpha(trail->alpha);
    cb.set_alpha(trail->alpha);
    const float first_decay = trail->alpha / (static_cast<float>(count) * 0.5f - 2.0f);
    const float second_decay = trail->alpha / (static_cast<float>(second->curve_pts.size()) * 0.5f - 2.0f);
    const auto decay = [](color32 &color, float amount) {
        const int alpha = static_cast<int>(color.get_alpha() - amount);
        color.set_alpha(static_cast<uint8_t>(std::max(0, alpha)));
    };
    vector3d minimum{1.0e32f, 1.0e32f, 1.0e32f}, maximum{-1.0e32f, -1.0e32f, -1.0e32f};
    if (!trail->single_entity || trail->axis != 0) {
        for (int i = 0; i < count - 1; ++i) {
            const auto previous_a = ca, previous_b = cb;
            decay(ca, first_decay);
            decay(cb, second_decay);
            render_trail(first->curve_pts[i],
                         second->curve_pts[i],
                         first->curve_pts[i + 1],
                         {},
                         {},
                         {},
                         previous_a,
                         ca,
                         previous_b,
                         trail->additive,
                         minimum,
                         maximum);
            render_trail(second->curve_pts[i + 1],
                         first->curve_pts[i + 1],
                         second->curve_pts[i],
                         {},
                         {},
                         {},
                         cb,
                         previous_b,
                         ca,
                         trail->additive,
                         minimum,
                         maximum);
        }
    } else {
        auto points = first->curve_pts;
        auto left = std::make_unique<vector3d[]>(count);
        auto right = std::make_unique<vector3d[]>(count);
        for (int i = 0; i < count - 2; ++i) {
            const auto camera_position = g_game_ptr->get_current_view_camera(0)->get_abs_position();
            auto tangent = points[i + 1] - points[i];
            const float length = tangent.length();
            if (length > 0.0001f) {
                tangent = tangent / length;
                auto normal = vector3d::cross(tangent, points[i] - camera_position);
                const float normal_length_squared = normal.length2();
                if (normal_length_squared > 9.9999994e-11f)
                    normal = normal * (1.0f / std::sqrt(normal_length_squared));
                const float extension = trail->width * 0.05f;
                points[i] = points[i] - tangent * extension;
                points[i + 1] = points[i] + tangent * (length + 2.0f * extension);
                normal = normal * trail->width;
                left[i] = points[i] + normal;
                right[i] = points[i] - normal;
                left[i + 1] = points[i + 1] + normal;
                right[i + 1] = points[i + 1] - normal;
            }
        }
        for (int i = 1; i < count - 3; ++i) {
            const auto previous_a = ca, previous_b = cb;
            decay(ca, first_decay);
            decay(cb, second_decay);
            const float u = static_cast<float>(i) / var<float>(0x00921B50);
            const float next_u = static_cast<float>(i + 1) / var<float>(0x00921B50);
            render_trail(left[i],
                         right[i],
                         right[i + 1],
                         {u, 0},
                         {u, 1},
                         {next_u, 1},
                         previous_a,
                         ca,
                         previous_b,
                         trail->additive,
                         minimum,
                         maximum);
            render_trail(right[i + 1],
                         left[i + 1],
                         left[i],
                         {next_u, 1},
                         {next_u, 0},
                         {u, 0},
                         cb,
                         previous_b,
                         ca,
                         trail->additive,
                         minimum,
                         maximum);
        }
    }
    close_motion_mesh(vector3d{0.0f, 0.0f, 0.0f}, static_cast<float>(std::sqrt(3.0) * 1.0e32));
}

void motion_effect_struct::render_distorted_trail()
{
    if (draining_distorted_trail && --distorted_trail->sample_count <= 0) {
        trail_active = draining_trail = false;
        delete_history(trail);
        remove_from_list();
        return;
    }
    if (distorted_trail->sample_count <= 1)
        return;
    spline *curves[3]{distorted_spline(), distorted_spline2(), distorted_spline3()};
    for (auto *curve : curves)
        curve->reserve_control_pts(distorted_trail->sample_count + 1);
    const auto current = current_distorted_sample(distorted_trail, true);
    curves[0]->add_control_pt(current.first);
    curves[1]->add_control_pt(current.second);
    curves[2]->add_control_pt(current.third);
    int index = distorted_trail->next_sample;
    for (int i = 0; i != distorted_trail->sample_count; ++i) {
        if (index <= 0)
            index = distorted_trail->capacity;
        const auto &sample = distorted_trail->samples[--index];
        curves[0]->add_control_pt(sample.first);
        curves[1]->add_control_pt(sample.second);
        curves[2]->add_control_pt(sample.third);
    }
    for (auto *curve : curves)
        build_trail_spline(curve);
    const int count = static_cast<int>(curves[0]->curve_pts.size());
    nglCreateMesh(0x40000, 6 * count - 6, 0, nullptr);
    vector3d minimum{1.0e32f, 1.0e32f, 1.0e32f}, maximum{-1.0e32f, -1.0e32f, -1.0e32f};
    float factor = 1.0f;
    const float step = 1.0f / static_cast<float>(count - 1);
    const auto color_at = [](float value) {
        color32 color{0, 0, 0, 255};
        const color32 first{16742400u}, second{16773013u};
        for (int channel = 0; channel != 3; ++channel)
            color[channel] = static_cast<uint8_t>(first[channel] * (1.0f - value) + second[channel] * value);
        return color;
    };
    for (int i = 0; i < count - 1; ++i) {
        const auto ca = color_at(factor), cb = color_at(factor - step);
        const vector4d uv[4]{{static_cast<float>(i), 0, 0, 0},
                             {static_cast<float>(i), 1, 0, 0},
                             {static_cast<float>(i + 1), 0, 0, 0},
                             {static_cast<float>(i + 1), 1, 0, 0}};
        const auto &a = curves[0]->curve_pts, &b = curves[1]->curve_pts, &c = curves[2]->curve_pts;
        render_distorted_trail(
            a[i], a[i + 1], b[i], uv[0], uv[2], uv[1], ca, cb, ca, distorted_trail->additive, minimum, maximum);
        render_distorted_trail(
            b[i + 1], b[i], a[i + 1], uv[3], uv[1], uv[2], cb, ca, cb, distorted_trail->additive, minimum, maximum);
        render_distorted_trail(
            a[i], c[i], a[i + 1], uv[1], uv[0], uv[3], ca, ca, cb, distorted_trail->additive, minimum, maximum);
        render_distorted_trail(
            c[i + 1], a[i + 1], c[i], uv[2], uv[3], uv[0], cb, cb, ca, distorted_trail->additive, minimum, maximum);
        render_distorted_trail(
            b[i], b[i + 1], c[i], uv[0], uv[2], uv[1], ca, cb, ca, distorted_trail->additive, minimum, maximum);
        render_distorted_trail(
            c[i + 1], c[i], b[i + 1], uv[3], uv[1], uv[2], cb, ca, cb, distorted_trail->additive, minimum, maximum);
        factor -= step;
    }
    const vector3d center{static_cast<float>((static_cast<double>(minimum.x) + maximum.x) * 0.5),
                          static_cast<float>((static_cast<double>(minimum.y) + maximum.y) * 0.5),
                          static_cast<float>((static_cast<double>(minimum.z) + maximum.z) * 0.5)};
    close_motion_mesh(center, motion_distance(minimum, center));
}

void motion_effect_struct::record(Float elapsed)
{
    if (pose_recording) {
        pose_history->remaining -= elapsed.value;
        if (pose_history->remaining <= 0.0f) {
            if (auto *entity = owner.get_volatile_ptr()) {
                const auto &transform = entity->get_abs_po();
                pose_history->samples[pose_history->next_sample] = {quaternion{transform.m},
                                                                    entity->get_abs_position()};
                ++pose_history->next_sample;
                if (pose_history->sample_count < pose_history->capacity)
                    ++pose_history->sample_count;
                if (pose_history->next_sample >= pose_history->capacity)
                    pose_history->next_sample = 0;
            }
            pose_history->remaining = pose_history->interval;
        }
    }
    if (distorted_trail_active) {
        distorted_trail->remaining -= elapsed.value;
        if (distorted_trail->remaining <= 0.0f) {
            if (!draining_distorted_trail) {
                auto &destination = distorted_trail->samples[distorted_trail->next_sample];
                if (distorted_trail->single_entity) {
                    if (distorted_trail->axis >= 1 && distorted_trail->axis <= 3) {
                        const auto sample = current_distorted_sample(distorted_trail, false);
                        destination.first = sample.first;
                        destination.second = sample.second;
                    }
                } else {
                    destination = current_distorted_sample(distorted_trail, false);
                }
                ++distorted_trail->next_sample;
                if (distorted_trail->sample_count < distorted_trail->capacity)
                    ++distorted_trail->sample_count;
                if (distorted_trail->next_sample >= distorted_trail->capacity)
                    distorted_trail->next_sample = 0;
            }
            distorted_trail->remaining = distorted_trail->interval;
        }
    }
    if (trail_active) {
        trail->remaining -= elapsed.value;
        if (trail->remaining <= 0.0f) {
            if (!draining_trail) {
                if (!trail->single_entity || (trail->axis >= 0 && trail->axis <= 3))
                    trail->samples[trail->next_sample] = current_trail_sample(trail);
                ++trail->next_sample;
                if (trail->sample_count < trail->capacity)
                    ++trail->sample_count;
                if (trail->next_sample >= trail->capacity)
                    trail->next_sample = 0;
            }
            trail->remaining = trail->interval;
        }
    }
}

void motion_effect_struct::record_all_motion_fx(Float elapsed)
{
    for (auto *effect = active(); effect != nullptr;) {
        auto *next_effect = effect->next;
        auto *entity = effect->owner.get_volatile_ptr();
        float scale = g_world_ptr->time_manager.field_0;
        if (entity != nullptr) {
            if ((entity->field_4 & 0x8000) != 0) {
                entity = entity->get_conglom_owner();
                if (entity->has_time_ifc())
                    scale = static_cast<float>(entity->time_ifc()->sub_4ADE50());
            } else if (entity->is_an_entity() && entity->has_time_ifc()) {
                scale = static_cast<float>(entity->time_ifc()->sub_4ADE50());
            }
        }
        effect->record(scale * elapsed.value);
        effect = next_effect;
    }
}

void motion_effect_struct_patch() {}
