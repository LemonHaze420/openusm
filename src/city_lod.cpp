#include "city_lod.h"

#include "camera.h"
#include "common.h"
#include "femanager.h"
#include "func_wrapper.h"
#include "game.h"
#include "geometry_manager.h"
#include "hull.h"
#include "igofrontend.h"
#include "igozoomoutmap.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "ngl_scene.h"
#include "oldmath_po.h"
#include "os_developer_options.h"
#include "parse_generic_mash.h"
#include "resource_key.h"
#include "resource_manager.h"
#include "utility.h"
#include "variables.h"

#include <cmath>
#include <functional>

VALIDATE_SIZE(city_lod, 8u);
VALIDATE_SIZE(strip_lod, 0x18);
VALIDATE_SIZE(lod_batch, 0x48);
VALIDATE_SIZE(lod_building, 0x18);
VALIDATE_OFFSET(lod_batch, corners, 0xC);
VALIDATE_OFFSET(lod_batch, height, 0x3C);

namespace {

struct lod_culling_settings {
    float air_height = 50.0f;
    float air_distance_squared = 9000000.0f;
    float ground_distance_squared = 2890000.0f;
};
Var<lod_culling_settings> lod_culling{0x00921F08};
struct lod_box_scale_settings {
    float x = 0.98f;
    float y = 0.98f;
    float z = 0.98f;
};
Var<lod_box_scale_settings> lod_box_scale{0x00922440};
Var<int> suppress_city_lod{0x00960B90};
constexpr float near_building_distance_squared = 21609.0f;

bool zoom_map_active()
{
    return g_femanager.IGO->m_igo_zoom_out_map->sub_55F320();
}

template <typename T>
void un_mash_array(mashable_vector<T> &array, generic_mash_data_ptrs &ptrs)
{
    if (array.is_shared()) {
        ptrs.rebase_shared(4);
        array.m_data = ptrs.get_from_shared<T>(array.m_size);
        ptrs.rebase_shared(4);
    } else {
        ptrs.rebase(4);
        array.m_data = ptrs.get<T>(array.m_size);
        ptrs.rebase(4);
    }
}
}  // namespace

city_lod::city_lod(const char *name) : field_0(nullptr), field_4(false)
{
    field_4 = os_developer_options::instance->get_flag(static_cast<os_developer_options::flags_t>(14));
    if (!field_4)
        return;

    resource_key resource_id{string_hash{name}, RESOURCE_KEY_TYPE_LOD};
    int size;
    auto *resource = resource_manager::get_resource(resource_id, &size, nullptr);
    if (resource != nullptr)
        parse_generic_object_mash(field_0, resource, nullptr, nullptr, nullptr, 0, 0, nullptr);

    map_mesh() = nglGetMesh(tlFixedString{"lod_map000"}, true);
    sides_mesh() = nglGetMesh(tlFixedString{"lod_sides000"}, true);
    top_mesh() = nglGetMesh(tlFixedString{"lod_top000"}, true);
    top_texture() = nglGetTexture(tlFixedString{"SM2_CityLODroofs"});
    mString ground_name{name};
    ground_name.append("_ground000");
    ground_mesh() = nglGetMesh(tlFixedString{ground_name.c_str()}, true);
    box_mesh() = nglGetFirstMeshInFile(tlFixedString{"bldg_box"});
    assert(box_mesh() != nullptr);
    for (unsigned i = 0; i < box_mesh()->NSections; ++i) {
        auto *material = box_mesh()->Sections[i].Section->Material;
        if (*material->Name == tlFixedString{"BoxMatTop"})
            top_material() = material;
    }
}


bool lod_batch::outside_frustum(const hull &frustum) const
{
    const vector3d edge_x = corners[2] - corners[1];
    const vector3d edge_z = corners[1] - corners[0];
    const vector3d center = (corners[0] + corners[2] + vector3d{0.0f, height, 0.0f}) * 0.5f;
    float extent_x;
    float extent_z;
    vector3d axis;
    if (std::equal_to<float>{}(edge_x.z, 0.0f)) {
        extent_x = std::fabs(edge_x.x) * 0.5f;
        extent_z = std::fabs(edge_z.z) * 0.5f;
        axis = vector3d{1.0f, 0.0f, 0.0f};
    } else {
        const float length = std::sqrt(edge_x.x * edge_x.x + edge_x.y * edge_x.y + edge_x.z * edge_x.z);
        extent_x = length * 0.5f;
        extent_z = std::sqrt(edge_z.x * edge_z.x + edge_z.y * edge_z.y + edge_z.z * edge_z.z) * 0.5f;
        axis = edge_x * (1.0f / length);
    }
    for (uint32_t i = 0; i < frustum.field_0.m_size; ++i) {
        const auto &p = frustum.field_0.m_data[i].arr;
        const float radius = std::fabs(p[0] * axis.x + p[2] * axis.z) * extent_x + std::fabs(p[1]) * (height * 0.5f) +
                             std::fabs(-p[0] * axis.z + p[2] * axis.x) * extent_z;
        if (p[0] * center.x + p[1] * center.y + p[2] * center.z + p[3] + radius < 0.0f)
            return true;
    }
    return false;
}

void lod_batch::render(const vector3d &camera_position, bool zoom_map) const
{
    for (int i = static_cast<int>(buildings.m_size) - 1; i >= 0; --i) {
        const auto &building = buildings[i];
        const float dx = camera_position.x - building.position.x;
        const float dz = camera_position.z - building.position.z;
        if (!zoom_map && dx * dx + dz * dz <= near_building_distance_squared)
            continue;

        po transform;
        const int rotation = building.rotation + ((building.flags & 0x10) ? 256 : 0);
        transform.set_rotate_y(static_cast<float>(rotation) * 0.017453292f);
        transform.m[3] =
            vector4d{building.position.x, building.position.y + building.height * 0.5f, building.position.z, 1.0f};

        nglParamSet<nglShaderParamSet_Pool> params{
            static_cast<nglParamSet<nglShaderParamSet_Pool>::nglParamSetType>(1)};

        const auto color = building.color;
        auto *tint =
            new (nglListAlloc(sizeof(vector4d), 16)) vector4d{static_cast<float>(8 * (color & 31)) * (2.0f / 255.0f),
                                                              static_cast<float>((color >> 2) & 248) * (2.0f / 255.0f),
                                                              static_cast<float>((color >> 7) & 248) * (2.0f / 255.0f),
                                                              static_cast<float>((color >> 8) & 128) * (2.0f / 255.0f)};
        params.SetParam(nglTintParam{tint});
        nglMeshParams mesh_params{};
        mesh_params.Flags = NGLP_SCALE;
        mesh_params.Scale = {vector3d{building.width * 0.5f * lod_box_scale().x,
                                      building.height * 0.5f * lod_box_scale().y,
                                      building.depth * 0.5f * lod_box_scale().z}};
        nglListAddMesh(
            city_lod::box_mesh(), *bit_cast<const math::MatClass<4, 3> *>(&transform.m), &mesh_params, &params);
    }
}

void strip_lod::un_mash_start(generic_mash_header *, void *, generic_mash_data_ptrs *ptrs, void *)
{
    uint32_t *shared_header = nullptr;
    if (batches.is_shared()) {
        ptrs->rebase_shared(4);
        shared_header = ptrs->get_from_shared<uint32_t>(3);
        ptrs->rebase_shared(4);
    }
    un_mash_array(batches, *ptrs);
    if (shared_header != nullptr && shared_header[2] != 0) {
        ptrs->field_0 += shared_header[0];
        ptrs->field_4 += shared_header[1] - sizeof(lod_batch) * batches.m_size;
    } else {
        for (auto &batch : batches)
            un_mash_array(batch.buildings, *ptrs);
    }
    if (shared_header != nullptr) {
        ++shared_header[2];
        ptrs->rebase_shared(4);
    } else {
        ptrs->rebase(4);
    }
    un_mash_array(meshes, *ptrs);
    for (auto &mesh : meshes) {
        const auto *key = ptrs->get_from_shared<resource_key>();
        mesh = nglGetMesh(key->m_hash.source_hash_code, true);
    }
}

void strip_lod::render_meshes()
{
    const bool zoom_map = zoom_map_active();
    for (auto *mesh : meshes) {
        const vector3d center{mesh->SphereCenter.x, mesh->SphereCenter.y, mesh->SphereCenter.z};
        const vector3d eye{nglCurScene->ViewPos.x, nglCurScene->ViewPos.y, nglCurScene->ViewPos.z};
        const vector3d delta = center - eye;
        if (zoom_map || delta.x * delta.x + delta.y * delta.y + delta.z * delta.z >= near_building_distance_squared)
            nglListAddMesh(mesh, *bit_cast<const math::MatClass<4, 3> *>(&identity_matrix), nullptr, nullptr);
    }
}

void city_lod::render()
{
    if (suppress_city_lod() || field_0 == nullptr || !field_4 || g_indoors)
        return;
    auto *camera = g_game_ptr->get_current_view_camera(0);
    if (geometry_manager::PROJ_FAR_PLANE_D <= 147.0f)
        return;
    const bool zoom_map = zoom_map_active();
    nglListBeginScene(static_cast<nglSceneParamType>(0));
    nglSetClearFlags(0);
    nglCurScene->AnimTime = 0.0f;
    nglCurScene->FBWriteMask = 7;
    geometry_manager::rebuild_view_frame();
    nglSetWorldToViewMatrix(*bit_cast<const math::MatClass<4, 3> *>(&geometry_manager::xforms[4]));
    float fov, nearz, farz;
    nglGetProjectionParams(&fov, &nearz, &farz);
    const float far_plane = g_distance_clipping_enabled
                                ? std::fmax(static_cast<float>(g_distance_clipping) * 0.01f * 1900.0f + 100.0f, 100.0f)
                                : 6000.0f;
    nglSetPerspectiveMatrix(fov, 0.1f, far_plane);
    const auto &eye = camera->get_abs_position();
    const float distance_squared =
        eye.y > lod_culling().air_height ? lod_culling().air_distance_squared : lod_culling().ground_distance_squared;
    for (const auto &batch : field_0->batches) {
        const vector3d center = (batch.corners[0] + batch.corners[1] + batch.corners[2] + batch.corners[3]) * 0.25f;
        const float dx = eye.x - center.x;
        const float dz = eye.z - center.z;
        if ((zoom_map || dx * dx + dz * dz < distance_squared) &&
            !batch.outside_frustum(geometry_manager::world_space_frustum))
            batch.render(eye, zoom_map);
    }
    field_0->render_meshes();
    if (ground_mesh() != nullptr) {
        nglListBeginScene(static_cast<nglSceneParamType>(1));
        nglSetClearFlags(0);
        nglSetZTestEnable(false);
        nglSetZWriteEnable(false);
        matrix4x4 transform = identity_matrix;
        transform[3] = vector4d{0.0f, -0.5f, 0.0f, 1.0f};
        nglListAddMesh(ground_mesh(), *bit_cast<const math::MatClass<4, 3> *>(&transform), nullptr, nullptr);
        nglListEndScene();
    }
    nglListEndScene();
}

void city_lod_patch()
{
    FUNC_ADDRESS(address, &city_lod::render);
    REDIRECT(0x0054B2FB, address);
}
