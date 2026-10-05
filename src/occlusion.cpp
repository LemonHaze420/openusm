#include "occlusion.h"

#include "app.h"
#include "beam.h"
#include "camera.h"
#include "color32.h"
#include "common.h"
#include "debug_render.h"
#include "game.h"
#include "oldmath_po.h"
#include "vector3d.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

VALIDATE_SIZE(occlusion::quad, 0x30);
VALIDATE_SIZE(occlusion::quad_shadow_volume, 0x90);

Var<occlusion::quad *> occlusion::quad_database = (0x0095C880);

Var<int> occlusion::quad_database_count = (0x0095C884);

Var<bool> occlusion::initialized = (0x0095C87C);
Var<int> occlusion::num_active_shadow_volumes = (0x0095C88C);
Var<int> occlusion::minimum_occluder_score{0x00921D80};
Var<int> occlusion::quad_database_update_index{0x00960B14};

void occlusion::term()
{

    operator delete[](quad_database());
    quad_database() = nullptr;
    quad_database_count() = 0;
}

void occlusion::init()
{
    quad_database() = new quad[400u];
    quad_database_count() = 0;
    initialized() = true;
    num_active_shadow_volumes() = 0;
#if STANDALONE_SYSTEM
    minimum_occluder_score() = 1;
#endif
}

void occlusion::add_quad_to_database(const occlusion::quad &a1)
{
    if (quad_database_count() < 399) {
        quad_database()[quad_database_count()++] = a1;
    }
}

void occlusion::reset_active_occluders()
{
    num_active_shadow_volumes() = 0;
}

void occlusion::empty_quad_database()
{
    assert(initialized());

    occlusion::quad_database_count() = 0;
}

void occlusion::debug_render_occluders()
{
    const color32 blue{255, 0, 0, 255};
    const color32 green{0, 255, 0, 128};
    for (auto i = 0; i < num_active_shadow_volumes(); ++i) {
        auto *v12 = &active_shadow_volumes()[i];
        auto *v1 = app::instance;
        auto *v2 = v1->m_game;
        auto *v4 = v2->get_current_view_camera(0);
        auto &v5 = v4->get_abs_po();
        [[maybe_unused]] auto v11 = v5;

        vector3d v10;
        vector3d v9;
        vector3d v8;
        vector3d v7;
        v10 = v12->field_0[0];
        v9 = v12->field_0[1];
        v8 = v12->field_0[2];
        v7 = v12->field_0[3];

        render_beam(v10, v9, blue, 1.0, false);
        render_beam(v9, v8, blue, 1.0, false);
        render_beam(v8, v7, blue, 1.0, false);
        render_beam(v7, v10, blue, 1.0, false);
        render_beam(v10, v8, blue, 1.0, false);
        render_beam(v9, v7, blue, 1.0, false);

        render_quad(v12->field_0[0], v12->field_0[1], v12->field_0[2], v12->field_0[3], green, true);
    }
}

void occlusion::init_frame(const vector3d &a1)
{
    update_based_on_scores(a1);
    active_shadow_volumes_scratchpad_mirror() = active_shadow_volumes();
}

void occlusion::term_frame()
{

}

void occlusion::update_based_on_scores(const vector3d &a1)
{
    std::qsort(active_shadow_volumes(), num_active_shadow_volumes(), sizeof(quad_shadow_volume),
               [](const void *left, const void *right) {
                   const int a = static_cast<const quad_shadow_volume *>(left)->field_80;
                   const int b = static_cast<const quad_shadow_volume *>(right)->field_80;
                   return a > b ? -1 : (a < b ? 1 : 0);
               });



    const int last = num_active_shadow_volumes() - 1;
    for (int retained = 0; retained < last; ++retained) {
        auto &volume = active_shadow_volumes()[retained];
        if (volume.field_80 < minimum_occluder_score()) {
            num_active_shadow_volumes() = retained;
            break;
        }
        volume.field_80 = 0;
    }

    if (quad_database_count() == 0) {
        return;
    }
    const int retained = std::min(num_active_shadow_volumes(), 11);
    const int count = std::min(retained + quad_database_count(), retained + 3);
    num_active_shadow_volumes() = retained;
    for (int index = retained; index < count; ++index) {
        const auto &candidate = quad_database()[quad_database_update_index() % quad_database_count()];
        add_active_occluder(candidate, a1);
        quad_database_update_index() = static_cast<int>(
            static_cast<unsigned int>(quad_database_update_index()) + 1u);
    }
    for (int index = 0; index < retained; ++index) {
        build_shadow_volume(active_shadow_volumes()[index], a1);
    }
}

namespace {
plane plane_through_points(const vector3d &origin, const vector3d &a, const vector3d &b)
{
    const double first_x = double(a.x) - origin.x;
    const double first_y = double(a.y) - origin.y;
    const double first_z = double(a.z) - origin.z;
    const float second_x = double(b.x) - origin.x;
    const float second_y = double(b.y) - origin.y;
    const double second_z = double(b.z) - origin.z;

    vector3d normal{static_cast<float>(second_y * first_z - second_z * first_y),
                    static_cast<float>(second_z * first_x - second_x * first_z),
                    static_cast<float>(second_x * first_y - second_y * first_x)};
    const double length_squared = double(normal.x) * normal.x + double(normal.y) * normal.y
                                  + double(normal.z) * normal.z;
    if (length_squared > 9.999999439624929e-11) {
        const double inverse_length = 1.0 / std::sqrt(length_squared);
        normal.x = normal.x * inverse_length;
        normal.y = normal.y * inverse_length;
        normal.z = normal.z * inverse_length;
    }
    plane result;
    result.arr[0] = normal.x;
    result.arr[1] = normal.y;
    result.arr[2] = normal.z;
    result.arr[3] = -(double(origin.x) * normal.x + double(normal.y) * origin.y
                      + double(normal.z) * origin.z);
    return result;
}

double signed_distance(const plane &face, const vector3d &point)
{
    return double(face.arr[2]) * point.z + double(face.arr[1]) * point.y
           + double(face.arr[0]) * point.x + face.arr[3];
}

void reverse_plane(plane &face)
{
    for (float &component : face.arr) {
        component = -component;
    }
}
}

void occlusion::build_shadow_volume(quad_shadow_volume &volume, const vector3d &camera_position)
{
    volume.field_84 = camera_position;
    for (int index = 0; index < 4; ++index) {
        auto &face = volume.field_30[index];
        face = plane_through_points(camera_position, volume.field_0[index], volume.field_0[(index + 1) & 3]);
        if (signed_distance(face, volume.field_0[(index + 2) & 3]) > 0.0) {
            reverse_plane(face);
        }
    }
    auto &face = volume.field_30[4];
    face = plane_through_points(volume.field_0[0], volume.field_0[1], volume.field_0[2]);
    if (signed_distance(face, camera_position) < float(EPSILON)) {
        reverse_plane(face);
    }
}

void occlusion::add_active_occluder(const quad &occluder, const vector3d &camera_position)
{
    assert(num_active_shadow_volumes() < 14);
    auto &volume = active_shadow_volumes()[num_active_shadow_volumes()++];
    for (int index = 0; index < 4; ++index) {
        volume.field_0[index] = occluder.points[index];
    }
    volume.field_80 = 0;
    build_shadow_volume(volume, camera_position);
}

bool occlusion::find_occluding_volume(const vector3d &center, float radius, int &volume_index)
{
    for (int index = 0; index < num_active_shadow_volumes(); ++index) {
        const auto &volume = active_shadow_volumes_scratchpad_mirror()[index];
        bool inside = true;
        for (const auto &face : volume.field_30) {

            const double distance = double(center.x) * face.arr[0] + double(center.z) * face.arr[2]
                                    + double(center.y) * face.arr[1] + face.arr[3] + radius;
            if (!(distance <= 0.0)) {
                inside = false;
                break;
            }
        }
        if (inside) {
            volume_index = index;
            return true;
        }
    }
    return false;
}

bool occlusion::sphere_occluded(const vector3d &center, float radius, int score)
{
    int index;
    if (!find_occluding_volume(center, radius, index)) {
        return false;
    }
    active_shadow_volumes_scratchpad_mirror()[index].field_80 += score;
    return true;
}
