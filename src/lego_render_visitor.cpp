#include "lego_render_visitor.h"

#include "bitvector.h"
#include "common.h"
#include "distance_fader.h"
#include "func_wrapper.h"
#include "gen_building.h"
#include "geometry_manager.h"
#include "hull.h"
#include "lego_map.h"
#include "ngl.h"
#include "ngl_support.h"
#include "occlusion.h"
#include "oldmath_po.h"
#include "region.h"
#include "scratchpad_stack.h"
#include "stack_allocator.h"
#include "subdivision_static_region_list.h"
#include "us_pcuv_shader.h"
#include "variables.h"
#include <algorithm>
#include <cmath>
#include <cstring>

VALIDATE_SIZE(lego_render_visitor, 0xA8);
VALIDATE_SIZE(lego_render_visitor::buffered_lego, 0x20);

namespace {
int lego_count;
scene_entity *lego_array;
uint64_t *visited_legos;
fixed_bitvector<uint32_t, 2048> *previous_legos;
fixed_bitvector<uint32_t, 2048> *new_legos;
matrix4x4 *lego_matrix;
static Var<int> lego_visit_counter{0x0095C89C};
static Var<int> legos_rendered_last_frame{0x0095C898};
static Var<bool> fade_legos{0x009391EC};

int visit_lego_index(subdivision_visitor &visitor, int index)
{
    const uint64_t bit = uint64_t{1} << (index & 63);
    auto &word = visited_legos[index >> 6];
    if ((word & bit) == 0) {
        word |= bit;
        static_cast<lego_render_visitor &>(visitor).render_lego(index);
    }
    return 0;
}

int visit_lego_node(subdivision_visitor &, const subdivision_node &)
{
    return 0;
}
const subdivision_visitor::native_vtable lego_vtable{visit_lego_node, visit_lego_index};


void make_lego_matrix(const scene_entity &lego)
{
    const auto &indices = var<uint8_t[449]>(0x0095A0D8);
    const auto &sine = var<float[181]>(0x0095A310);
    const float s = sine[indices[lego.quantized_yaw]];
    const float c = sine[indices[lego.quantized_yaw + 90]];
    lego_matrix->arr[0][0] = c;
    lego_matrix->arr[0][2] = s;
    lego_matrix->arr[2][0] = -s;
    lego_matrix->arr[2][2] = c;
    lego_matrix->w = vector4d{lego.x, lego.y, lego.z, 1.0f};
}


void render_scene_entity(const scene_entity &lego, float fade, region &reg)
{
    using shader_params = nglParamSet<nglShaderParamSet_Pool>;
    shader_params params{static_cast<shader_params::nglParamSetType>(1)};
    if (fade < 1.0f) {
        auto *tint = new (nglListAlloc(sizeof(vector4d), 16)) vector4d{1.0f, 1.0f, 1.0f, fade};
        params.SetParam(nglTintParam{tint});
    }
    if (lego.material_indices != 0) {
        params.SetParam(USMMaterialListParam{reg.field_9C->field_4});
        params.SetParam(
            USMMaterialIndicesParam{reinterpret_cast<uint8_t *>(const_cast<uint32_t *>(&lego.material_indices))});
    }
    static nglMeshParams mesh_params{0x80000040u};
    FastListAddMesh(lego.mesh, *bit_cast<const math::MatClass<4, 3> *>(lego_matrix), &mesh_params, &params);
}
}  // namespace

lego_render_visitor::lego_render_visitor(region *region, const hull *hull, const vector3d &position, Float scale)
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(&lego_vtable);
    frustum = hull;
    packed_frustum.source = hull;
    for (uint32_t index = 0; index < 8; ++index) {
        const auto &plane = hull->field_0.m_data[index < hull->field_0.m_size ? index : 0];
        packed_frustum.x[index] = plane.arr[0];
        packed_frustum.y[index] = plane.arr[1];
        packed_frustum.z[index] = plane.arr[2];
        packed_frustum.w[index] = plane.arr[3];
    }
    camera_position = position;
    field_98 = scale;
    reg = region;
    current = nullptr;
    buffered_count = 0;
#else
    THISCALL(0x0052DEC0, this, region, hull, &position, scale);
#endif
}

void static_lego_list_methods::prepare_for_scene_traversal()
{
    ++lego_visit_counter();
    legos_rendered_last_frame() = 0;
}

int static_lego_list_methods::traverse_all(const subdivision_node &node, subdivision_visitor &visitor)
{
    if (lego_array == nullptr)
        return 0;
    const auto &list = static_cast<const static_region_list_node &>(node);
    for (uint8_t index = 0; index < list.count; ++index)
        visit_lego_index(visitor, static_cast<int16_t>(list.region_indices()[index]));
    return 0;
}

void static_lego_list_methods::init_region_traversal(int count, scene_entity *legos,
                                                     const fixed_bitvector<uint32_t, 2048> *previous)
{
    lego_count = count;
    lego_array = legos;
    visited_legos = static_cast<uint64_t *>(scratchpad_stack::alloc(8 * (count / 64 + 1)));
    std::memset(visited_legos, 0, 8 * (count / 64 + 1));
    lego_matrix = static_cast<matrix4x4 *>(scratchpad_stack::alloc(64));
    *lego_matrix = identity_matrix;
    previous_legos = static_cast<fixed_bitvector<uint32_t, 2048> *>(scratchpad_stack::alloc(0x108));
    std::memcpy(previous_legos, previous, 0x108);
    new_legos = new (scratchpad_stack::alloc(0x108)) fixed_bitvector<uint32_t, 2048>{};
}

void static_lego_list_methods::term_region_traversal(fixed_bitvector<uint32_t, 2048> *previous)
{
    std::memcpy(previous, new_legos, 0x108);
    scratchpad_stack::pop(new_legos, 0x108);
    scratchpad_stack::pop(previous_legos, 0x108);
    scratchpad_stack::pop(lego_matrix, 64);
    scratchpad_stack::pop(visited_legos, 8 * (lego_count / 64 + 1));
    new_legos = nullptr;
    previous_legos = nullptr;
    lego_matrix = nullptr;
    visited_legos = nullptr;
}

void lego_render_visitor::render_lego(int index)
{
    auto &lego = lego_array[index];
    const bool generated = (lego.flags & 0x10) != 0;
    const uint8_t group = lego.render.fade_group;
    const float far_squared = geometry_manager::PROJ_FAR_PLANE_D * geometry_manager::PROJ_FAR_PLANE_D;
    const float fade_distance = group ? reinterpret_cast<const float *>(reg->field_44)[group]
                                      : distance_fader::fade_distances2()[lego.flags & 0xF];
    const float radius = generated ? lego.building.building_height * 5.0f * 0.5f : lego.mesh->SphereRadius + 0.5f;
    const float min_y = lego.y - radius;
    const float max_y = lego.y + radius + (generated && (lego.flags & 0x40000000u) != 0 ? 1.0f : 0.0f);
    float dy = std::max({min_y - camera_position.y, camera_position.y - max_y, 0.0f});
    if (generated)
        dy *= 0.125f;
    float dx, dz;
    if (group) {
        const auto &center = reinterpret_cast<const vector4d *>(reg->field_48)[group];
        dx = center.x - camera_position.x;
        dz = center.z - camera_position.z;
        dy = 0.0f;
    } else {
        dx = lego.x - camera_position.x;
        dz = lego.z - camera_position.z;
    }
    const float distance = dx * dx + dz * dz + dy * dy;
    const uint32_t bit = uint32_t{1} << (index & 31);
    const bool previous = (previous_legos->field_4[index >> 5] & bit) != 0;
    int fade = lego.fade;
    if (distance < std::min(far_squared, fade_distance))
        fade = !fade_legos() || !previous ? 255 : std::min(fade + 10, 255);
    else
        fade = !previous ? 0 : std::max(fade - 10, 0);
    new_legos->field_4[index >> 5] |= bit;
    lego.fade = static_cast<uint8_t>(fade);
    if (fade == 0)
        return;
    buffered_lego record{&lego, distance, min_y, max_y, vector3d{lego.x, lego.y, lego.z}, fade * 0.0039215689f};
    const auto &stack = scratchpad_stack::stk;
    const int allocation = (stack.alignment + 31) & ~(stack.alignment - 1);
    if (stack.current + allocation < stack.segment + stack.segment_size_bytes) {
        current = new (scratchpad_stack::alloc(sizeof(record))) buffered_lego{record};
        ++buffered_count;
    } else {
        render_epilog(record);
    }
}

void lego_render_visitor::render_epilog(const buffered_lego &record)
{
    auto &lego = *record.lego;
    const bool generated = (lego.flags & 0x10) != 0;
    if (generated) {
        const auto &indices = var<uint8_t[449]>(0x0095A0D8);
        const auto &sine = var<float[181]>(0x0095A310);
        const float s = sine[indices[lego.quantized_yaw]];
        const float c = sine[indices[lego.quantized_yaw + 90]];
        const float half_width = lego.building.building_width * 2.5f;
        const float half_depth = lego.building.building_depth * 2.5f;
        const float half_height = (record.max_y - record.min_y) * 0.5f;
        const float center_y = (record.max_y + record.min_y) * 0.5f;
        for (uint32_t index = 0; index < 8; ++index) {
            const float nx = packed_frustum.x[index];
            const float ny = packed_frustum.y[index];
            const float nz = packed_frustum.z[index];
            const float support = std::abs(nx * c + nz * s) * half_width + std::abs(ny) * half_height +
                                  std::abs(-nx * s + nz * c) * half_depth;
            if (nx * lego.x + ny * center_y + nz * lego.z + packed_frustum.w[index] + support < 0.0f)
                return;
        }
    } else {
        const float radius = lego.mesh->SphereRadius + 0.5f;
        if (!const_cast<hull *>(frustum)->sub_5CC030(
                lego.render.sphere_x, lego.sphere.sphere_y, lego.sphere.sphere_z, radius) ||
            (g_disable_occlusion_culling == 0 &&
             occlusion::sphere_occluded(vector3d{static_cast<float>(lego.render.sphere_x),
                                                 static_cast<float>(lego.sphere.sphere_y),
                                                 static_cast<float>(lego.sphere.sphere_z)},
                                        radius,
                                        1)))
            return;
    }
    ++legos_rendered_last_frame();
    make_lego_matrix(lego);
    if (generated)
        render_generated_building(lego, record.fade, record.distance_squared, *reg, *lego_matrix);
    else
        render_scene_entity(lego, record.fade, *reg);
}

void lego_render_visitor::render_buffered_legos()
{
    auto *record = current;
    for (int index = 0; index < buffered_count; ++index) {
        render_epilog(*record);
        scratchpad_stack::pop(record, sizeof(buffered_lego));
        --record;
    }
}
