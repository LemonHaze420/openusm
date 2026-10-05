#include "wds_render_manager.h"

#include <windows.h>

#include "GL/gl.h"

#include "aeps.h"
#include "beam.h"
#include "bitvector.h"
#include "camera.h"
#include "camera_teleport_update_visitor.h"
#include "city_lod.h"
#include "comic_panels.h"
#include "common.h"
#include "custom_math.h"
#include <functional>
#include "conglom.h"
#include "fixed_pool.h"
#include "culling_params.h"
#include "cut_scene_player.h"
#include "base_ai_core.h"
#include "debug_render.h"
#include "distance_fader.h"
#include "femanager.h"
#include "filespec.h"
#include "func_wrapper.h"
#include "game.h"
#include "geometry_manager.h"
#include "glass_house_manager.h"
#include "hierarchical_entity_proximity_map.h"
#include "igofrontend.h"
#include "line_info.h"
#include "igozoomoutmap.h"
#include "lego_map.h"
#include "lego_render_visitor.h"
#include "loaded_regions_cache.h"
#include "light_source.h"
#include "motion_effect_struct.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "ngl_support.h"
#include "render_text.h"
#include "occlusion.h"
#include "occlusion_visitor.h"
#include "oldmath_po.h"
#include "oriented_bounding_box_root_node.h"
#include "os_developer_options.h"
#include "physical_interface.h"
#include "proximity_map.h"
#include "region.h"
#include "renderoptimizations.h"
#include "scratchpad_stack.h"
#include "stack_allocator.h"
#include "sector2d.h"
#include "shadow.h"
#include "subdivision_obb.h"
#include "subdivision_static_region_list.h"
#include "subdivision_visitor.h"
#include "trace.h"
#include "terrain.h"
#include "us_colorvol.h"
#include "us_lighting.h"
#include "us_pcuv_shader.h"
#include "utility.h"
#include "variables.h"
#include "vector2di.h"
#include "vector2d.h"
#include "vector4d.h"
#include "vtbl.h"
#include "ngl_scene.h"
#include "wds.h"

#include <cassert>
#include <algorithm>
#include <limits>
#include <cmath>

VALIDATE_SIZE(wds_render_manager, 156u);

struct traversed_entity {
    entity *ent{};
    region *reg{};
};
static Var<fixed_vector<traversed_entity, 750> *> traversed_entities_last_frame{0x0095C7B4};

wds_render_manager::wds_render_manager()
{
    this->field_30.sub_56FCB0();

    this->field_0 = new RenderOptimizations();
    this->field_94 = nullptr;
    this->field_98 = nullptr;

    this->field_10[0] = 0.30000001f;
    this->field_10[1] = -1.0f;
    this->field_10[2] = 0.2f;
    this->field_10[3] = 0;

    this->field_20[0] = 0.80000001f;
    this->field_20[1] = 0.80000001f;
    this->field_20[2] = 0.85000002f;
    this->field_20[3] = 1.0f;

    this->field_60 = {0, 0, 0, 1};

    this->field_90 = 1.0f;
    this->field_5C = nullptr;
    this->field_8C = 0;
    this->field_84 = 175.0f;
    this->field_88 = 250.0f;
}

void show_terrain_info()
{
    if (g_world_ptr != nullptr) {
        auto *v0 = g_world_ptr->get_hero_ptr(0);
        if (v0 != nullptr) {
            auto *v8 = g_world_ptr->get_hero_ptr(0);
            if (v8->has_physical_ifc()) {
                auto *v3 = g_world_ptr->get_hero_ptr(0);
                auto *v4 = v3->physical_ifc();

                string_hash v17;
                v4->get_parent_terrain_type(&v17);

                vector2d v5{512.0, 32.0};
                vector2di v14{v5};

                auto *v6 = v17.to_string();
                mString v16{v6};
                color32 v7{255, 255, 255, 255};
                render_text(v16, v14, v7, 1.0, 1.0);
            }
        }
    }
}

void sub_6A9863()
{
    if (debug_render_get_bval(SPHERES)) {
        render_debug_spheres();
    }

    if (debug_render_get_bval(LINES)) {
        render_debug_lines();
    }

    if (debug_render_get_ival(LINE_INFO)) {
        debug_render_line_info();
    }

    render_debug_lines();
    render_debug_spheres();
}

void wds_render_manager::debug_render()
{
    TRACE("wds_render_manager::debug_render");

    if constexpr (0) {
        if (os_developer_options::instance->get_flag(mString{"SHOW_TERRAIN_INFO"})) {
            show_terrain_info();
        }

        if (debug_render_get_ival((debug_render_items_e)20) ||
            os_developer_options::instance->get_flag(mString{"SHOW_GLASS_HOUSE"})) {
            //glass_house_manager::show_glass_houses();
        }

        //if ( debug_render_get_ival((debug_render_items_e)21) || SHOW_OBBS || SHOW_DISTRICTS )
        {
            auto *ter = g_world_ptr->get_the_terrain();
            ter->show_obbs();
        }

        render_debug_spheres();

        debug_render_line_info();
    }

    sub_6A9863();
}

void wds_render_manager::render_region_mesh(nglMesh *mesh, Float fade)
{
#if STANDALONE_SYSTEM
    if (mesh == nullptr)
        return;
    using shader_params = nglParamSet<nglShaderParamSet_Pool>;
    const bool tinted = std::not_equal_to<float>{}(fade.value, 1.0f);
    shader_params params{static_cast<shader_params::nglParamSetType>(tinted)};
    if (tinted) {
        auto *tint = new (nglListAlloc(sizeof(vector4d), 16)) vector4d{1.0f, 1.0f, 1.0f, fade};
        params.SetParam(nglTintParam{tint});
    }
    static nglMeshParams mesh_params{0x80000041u};
    FastListAddMesh(mesh, *bit_cast<const math::MatClass<4, 3> *>(&identity_matrix), &mesh_params, &params);
#else
    THISCALL(0x00537390, this, mesh, fade);
#endif
}

fixed_pool &far_away_render_list_pool()
{
    static Var<fixed_pool> pool{0x009220A4};
    if (!pool().m_initialized)
        pool().init(8, 64, 4, 1, 0, nullptr);
    return pool();
}

int wds_render_manager::add_far_away_entity(vhandle_type<entity> handle)
{
#if STANDALONE_SYSTEM
    for (auto *entry = field_98; entry != nullptr; entry = entry->next) {
        if (entry->handle.field_0 == handle.field_0)
            return 0;
    }
    auto *entry = static_cast<far_away_render_list_entry *>(far_away_render_list_pool().allocate_new_block());
    entry->handle = handle;
    entry->next = field_98;
    field_98 = entry;
    return 0;
#else
    return THISCALL(0x0052A470, this, handle);
#endif
}

void wds_render_manager::init_level(const char *a2)
{
    TRACE("wds_render_manager::init_level", a2);
    if constexpr (1) {
        if (this->field_5C == nullptr) {
            tlFixedString a1{"obb_shadow000"};
            this->field_5C = nglGetMesh(a1, true);
        }

        if (this->field_94 == nullptr) {
            filespec v7{mString{a2}};

            this->field_94 = new city_lod{v7.m_name.c_str()};
        }
    } else {
        THISCALL(0x00550930, this, a2);
    }
}

void wds_render_manager::create_colorvol_scene()
{
#if STANDALONE_SYSTEM
    if (os_developer_options::instance->get_flag(mString{"DISABLE_COLORVOLS"})) {
        USColorVolShaderSpace::g_enable_colorvols() = false;
        return;
    }
    if (!USColorVolShaderSpace::color_volumes_enabled())
        return;
    auto *saved_scene = nglListSelectScene(nglCurScene->field_30C);
    nglListBeginScene(static_cast<nglSceneParamType>(0));
    USColorVolShaderSpace::gUSColorVolScene() = nglCurScene;
    nglSetZWriteEnable(false);
    nglSetClearFlags(4);
    nglSetSceneCallBack(static_cast<nglSceneCallbackType>(0), USColorVolShaderSpace::USColorVolPreSceneCallback, nullptr);
    nglSetSceneCallBack(static_cast<nglSceneCallbackType>(2), USColorVolShaderSpace::USColorVolPostSceneCallback, nullptr);
    geometry_manager::rebuild_view_frame();
    nglSetWorldToViewMatrix(*bit_cast<const math::MatClass<4, 3> *>(&geometry_manager::xforms[4]));
    float fov, near_plane, far_plane;
    nglGetProjectionParams(&fov, &near_plane, &far_plane);
    const float clipping_far = g_distance_clipping_enabled
        ? std::max(g_distance_clipping * LARGE_EPSILON * 1900.0f + 100.0, 100.0) : 10000.0;
    nglSetPerspectiveMatrix(fov, 0.1f, clipping_far);
    nglCalculateMatrices(false);
    nglListEndScene();
    nglListSelectScene(saved_scene);
#else
    THISCALL(0x0053DA50, this);
#endif
}

void wds_render_manager::render_lowlods(camera &)
{
    if (os_developer_options::instance->get_flag(mString{"RENDER_LOWLODS"})) {
        this->field_94->render();
    }
}

static constexpr auto g_projected_fov_multiplier = 0.80000001f;

void wds_render_manager::update_occluders(camera &a2)
{
    TRACE("wds_render_manager::update_occluders");

    if constexpr (STANDALONE_SYSTEM) {
        occlusion::empty_quad_database();

        for (auto &i : this->field_30.field_0) {
            auto *reg = i.field_0;

            float v18 = reg->get_ground_level();

            auto &v5 = a2.get_abs_po();

            auto &v6 = a2.get_abs_position();

            occlusion_visitor visitor{v6, v5.get_z_facing(), v18, reg};

            ++subdivision_node_obb_base::visit_key();

            auto a4 = a2.compute_xz_projected_fov() * g_projected_fov_multiplier;

            auto &v11 = a2.get_abs_po();

            auto &v12 = a2.get_abs_po();

            vector3d a2a = a2.get_abs_position() - v12.get_z_facing() * 20.f;

            sector2d v26{a2a, v11.get_z_facing(), a4};
            ++region::visit_key2;
            auto *v16 = reg->field_98;
            if (v16 != nullptr) {
                v16->field_5C->traverse_sector_raster(v26, 100.0f, visitor);
            }
        }
    } else {
        THISCALL(0x00530500, this, &a2);
    }
}

void update_camera_teleport(camera &cam)
{
    TRACE("update_camera_teleport");

    if constexpr (STANDALONE_SYSTEM) {
        auto *v1 = g_cut_scene_player();
        auto v17 = (v1->is_playing() ? 1.0 : 25.0);

        static Var<vector3d> last_camera_position{0x00960B48};
        static Var<bool> last_camera_position_valid{0x00960B54};

        auto &abs_pos = cam.get_abs_position();
        if (!last_camera_position_valid()) {
            last_camera_position() = abs_pos;
        }

        ++entity::visit_key;

        auto len2 = (last_camera_position() - abs_pos).length2();
        if (len2 > v17) {
            fixed_vector<region *, 15> a2{};

            camera_teleport_update_visitor_t visitor{};
            loaded_regions_cache::get_regions_intersecting_sphere(
                abs_pos, culling_params::entity_traversal_distance, &a2);
            for (auto i = 0u; i < a2.size(); ++i) {
                region *reg = a2.at(i);
                assert(reg != nullptr);

                reg->visibility_map->traverse_sphere(abs_pos, culling_params::entity_traversal_distance, &visitor);
                auto *bitvector_of_legos_rendered_last_frame = reg->bitvector_of_legos_rendered_last_frame;
                if (bitvector_of_legos_rendered_last_frame != nullptr) {
                    bitvector_of_legos_rendered_last_frame->clear();
                }
            }

            if (traversed_entities_last_frame() != nullptr) {
                traversed_entities_last_frame()->m_size = 0;
            }
        }

        last_camera_position() = cam.get_abs_position();
        last_camera_position_valid() = true;
    } else {
        CDECL_CALL(0x00530760, &cam);
    }
}

void sub_520E60()
{
#if STANDALONE_SYSTEM
    static Var<conglomerate_light_cache_entry *> active{0x0095C874};
    static Var<conglomerate_light_cache_entry *> free{0x00960AE8};
    const uint32_t ticks = g_world_ptr->time_manager.field_C;
    auto **entry = &active();
    while (*entry != nullptr) {
        auto *current = *entry;
        if (ticks - current->last_used_tick <= 10) {
            entry = &current->next;
        } else {
            *entry = current->next;
            current->next = free();
            free() = current;
        }
    }
#else
    CDECL_CALL(0x00520E60);
#endif
}

namespace {
bool light_affects_position(light_source &light, const vector3d &position)
{
    const auto &properties = *light.properties;
    switch (properties.m_type) {
    case influence_type::POINT: {
        const auto delta = light.get_abs_position() - position;
        return delta.length2() < properties.cutoff_range * properties.cutoff_range;
    }
    case influence_type::DIRECTIONAL: {
        const auto delta = light.get_abs_position() - position;
        const float axial = dot(delta, light.get_abs_po().get_y_facing());
        return axial >= 0.0f && axial <= properties.cutoff_range
            && properties.cutoff_hot * properties.cutoff_hot >= delta.length2() - axial * axial;
    }
    case influence_type::SPOT: {
        const auto delta = position - light.get_abs_position();
        const float distance_squared = delta.length2();
        return distance_squared <= properties.cutoff_range * properties.cutoff_range
            && std::sqrt(distance_squared) * properties.cutoff_hot
                >= dot(delta, light.get_abs_po().get_y_facing());
    }
    default:
        return false;
    }
}

struct affecting_light_visitor : subdivision_visitor {
    const vector3d *position;
    int count{};
    light_source *light{};

    explicit affecting_light_visitor(const vector3d &point) : position(&point)
    {
        static const native_vtable callbacks{visit_light, nullptr};
        m_vtbl = reinterpret_cast<std::intptr_t>(&callbacks);
    }

    static int visit_light(subdivision_visitor &base, const subdivision_node &node)
    {
        auto &visitor = static_cast<affecting_light_visitor &>(base);
        auto &light = *reinterpret_cast<light_source *>(const_cast<subdivision_node *>(&node));
        if (light.field_5C != entity::visit_key && light_affects_position(light, *visitor.position)) {
            if (visitor.count >= 1)
                return 3;
            visitor.light = &light;
            ++visitor.count;
        }
        return 0;
    }
};
}

_std::vector<light_source *> *wds_render_manager::find_lights(const vector3d &position)
{
#if STANDALONE_SYSTEM
    static Var<_std::vector<light_source *>> affecting_lights{0x00960B94};
    auto &lights = affecting_lights();
    lights.clear();
    auto *sun = usl_suns()[g_TOD];
    if (!g_indoors && sun != nullptr) {
        lights.push_back(sun);
    } else {
        fixed_vector<region *, 15> regions;
        loaded_regions_cache::get_regions_intersecting_sphere_platform_independent(
            vector4d{position.x, position.y, position.z, 0.0f}, &regions);
        for (unsigned i = 0; i < regions.size(); ++i) {
            affecting_light_visitor visitor{position};
            regions.m_data[i]->light_proximity_map->traverse_point(position, visitor);
            if (visitor.count > 0)
                lights.push_back(visitor.light);
        }
    }
    return &lights;
#else
    return reinterpret_cast<_std::vector<light_source *> *(__fastcall *)(
        wds_render_manager *, void *, const vector3d *)>(0x00542440)(this, nullptr, &position);
#endif
}

void update_spidey_interface()
{
    if (g_world_ptr != nullptr) {
        if (g_world_ptr->get_hero_ptr(0) != nullptr) {
            g_femanager.IGO->UpdateInScene();
        }
    }
}

#include "debug_menu.h"

void wds_render_manager::render(camera &a2, int a3)
{
    TRACE("wds_render_manager::render");
    assert(this->field_94 != nullptr);
    if constexpr (1) {
        sub_520E60();
        update_camera_teleport(a2);
        if (g_disable_occlusion_culling == 3) {
            occlusion::reset_active_occluders();
        } else {
            this->update_occluders(a2);
            occlusion::init_frame(a2.get_abs_position());
        }

        auto *panel_params = comic_panels::get_panel_params();
        if (panel_params == nullptr || (panel_params->field_0 & 0x20) != 0) {
            this->create_colorvol_scene();

            if (debug_render_get_bval(LOW_LODS)) {
                this->render_lowlods(a2);
            }

            g_camera_link() = &a2;

            this->field_30.field_0.clear();
            this->field_30.field_10.clear();

            a2.compute_sector(g_world_ptr->the_terrain, false, nullptr);
            auto *prim_reg = a2.get_primary_region();

            auto *reg = g_world_ptr->the_terrain->find_region(a2.get_abs_position(), nullptr);
            if (reg != prim_reg) {
                auto *v10 = g_world_ptr->get_hero_ptr(a3);
                if (v10 != nullptr) {
                    if (v10->get_primary_region() == nullptr) {
                        prim_reg = reg;
                    }
                }
            }

            if (prim_reg == nullptr) {
                if (g_disable_occlusion_culling != 3) {
                    occlusion::term_frame();
                }

                return;
            }

            geometry_manager::rebuild_view_frame();
            ++region::visit_key;
            this->field_30.field_0.reserve(g_world_ptr->the_terrain->get_num_regions() + 1);

            a2.get_abs_position();

            this->build_render_data_regions(this->field_30, a2);
            this->sub_53D560(a2);
        }

        if (debug_render_get_bval(ENTITIES)) {
            this->build_render_data_ents(this->field_30, a2, a3);
            aeps::FrameSetupRenderAndThenRender();
            if (panel_params == nullptr || (panel_params->field_0 & 0x20) != 0) {
                motion_effect_struct::render_all_motion_fx(a2, geometry_manager::world_space_frustum);
                update_spidey_interface();
                ++entity::visit_key;
            }
        }

        if (panel_params == nullptr || (panel_params->field_0 & 0x20) != 0) {
            send_shadow_projectors();

            if (debug_render_get_bval(OCCLUSION)) {
                occlusion::debug_render_occluders();
            }

            this->debug_render();
            this->clear_colorvol_scene();
        }

        if (g_disable_occlusion_culling != 3) {
            occlusion::term_frame();
        }

    } else {
        THISCALL(0x0054B250, this, &a2, a3);
    }

    //_populate_missions();

    if (debug_render_get_bval(OCCLUSION)) {
        occlusion::debug_render_occluders();
    }

    this->debug_render();
}

void render_data::sub_56FCB0()
{
    if constexpr (STANDALONE_SYSTEM) {
        field_0.reserve(8);
        field_10.reserve(256);
        field_20 = {};
    } else {
        THISCALL(0x0056FCB0, this);
    }
}

void wds_render_manager::frame_advance([[maybe_unused]] Float a2)
{
    TRACE("wds_render_manager::frame_advance");

#if STANDALONE_SYSTEM
    static auto &curve = var<float[55]>(0x00921BA4);
    static auto &clock_scale = var<float>(0x00889840);
    const float clock = g_game_ptr->get_script_game_clock_timer() * clock_scale;

    int record = 0;
    while (record < 10 && clock >= curve[(record + 1) * 5]) {
        ++record;
    }
    if (record == 0 || record == 10) {
        for (int component = 0; component < 4; ++component) {
            field_10[component] = curve[record * 5 + component + 1];
        }
    } else {
        const float start = curve[record * 5];
        const float end = curve[(record + 1) * 5];
        const float alpha = (clock - start) / (end - start);
        for (int component = 0; component < 4; ++component) {
            const float from = curve[record * 5 + component + 1];
            const float to = curve[(record + 1) * 5 + component + 1];
            field_10[component] = from + (to - from) * alpha;
        }
    }
#else
    THISCALL(0x0054ADE0, this, a2);
#endif
}

void wds_render_manager::render_stencil_shadows(const camera &a2)
{
    TRACE("wds_render_manager::render_stencil_shadows");

    THISCALL(0x0053D5E0, this, &a2);
}

namespace {
using projected_frustum = fixed_vector<vector2d, 14>;


bool compute_projected_hull(projected_frustum &points)
{
    struct scan_point {
        vector2d position;
        float angle;
        int previous;
        int next;
    } scan[14];
    vector2d center{0.0f, 0.0f};
    for (uint32_t i = 0; i < points.m_size; ++i) {
        center.x += points.m_data[i].x;
        center.y += points.m_data[i].y;
    }
    center.x /= points.m_size;
    center.y /= points.m_size;
    int count = 0;
    for (uint32_t i = 0; i < points.m_size; ++i) {
        const float dx = points.m_data[i].x - center.x;
        const float dy = points.m_data[i].y - center.y;
        const float length = std::sqrt(dx * dx + dy * dy);
        if (length < EPSILON)
            continue;
        float angle = std::acos(std::clamp(dx / length, -1.0f, 1.0f));
        if (dy < 0.0f)
            angle = 2.0f * PI - angle;
        scan[count++] = scan_point{vector2d{dx, dy}, angle, 0, 0};
    }
    if (count < 3)
        return false;
    std::sort(scan, scan + count, [](const scan_point &a, const scan_point &b) { return a.angle > b.angle; });
    int first = 0;
    for (int i = 0; i < count; ++i) {
        scan[i].previous = (i + count - 1) % count;
        scan[i].next = (i + 1) % count;
        if (scan[i].position.y < scan[first].position.y
            || (std::equal_to<float>{}(scan[i].position.y, scan[first].position.y) && scan[i].position.x < scan[first].position.x))
            first = i;
    }
    int current = scan[first].next;
    do {
        const auto &p = scan[scan[current].previous].position;
        const auto &v = scan[current].position;
        const auto &n = scan[scan[current].next].position;
        if ((p.y - v.y) * (v.x - n.x) - (p.x - v.x) * (v.y - n.y) < 0.0f) {
            if (current == first) {
                vector2d min{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
                vector2d max{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
                for (uint32_t i = 0; i < points.m_size; ++i) {
                    min.x = std::min(min.x, points.m_data[i].x);
                    min.y = std::min(min.y, points.m_data[i].y);
                    max.x = std::max(max.x, points.m_data[i].x);
                    max.y = std::max(max.y, points.m_data[i].y);
                }
                points.m_data[0] = min;
                points.m_data[1] = vector2d{min.x, max.y};
                points.m_data[2] = max;
                points.m_data[3] = vector2d{max.x, min.y};
                points.m_size = 4;
                return false;
            }
            scan[scan[current].next].previous = scan[current].previous;
            scan[scan[current].previous].next = scan[current].next;
            current = scan[current].previous;
        } else {
            current = scan[current].next;
            if (current == first)
                break;
        }
    } while (true);
    points.m_size = 0;
    do {
        points.m_data[points.m_size++] = vector2d{
            center.x + scan[current].position.x, center.y + scan[current].position.y};
        current = scan[current].next;
    } while (current != first);
    return true;
}


void project_view_frustum(const vector3d &forward, projected_frustum &points, float ground, float distance)
{
    const auto &vertices = geometry_manager::frustum_verts;
    const auto &origin = vertices.m_data[0];
    points.m_data[points.m_size++] = vector2d{origin.x - forward.x * 20.0f, origin.z - forward.z * 20.0f};
    vector3d center{};
    for (uint32_t i = 1; i < 5; ++i) {
        center.x += vertices.m_data[i].x * 0.25f;
        center.y += vertices.m_data[i].y * 0.25f;
        center.z += vertices.m_data[i].z * 0.25f;
    }
    if (ground + 0.1f > origin.y)
        ground = origin.y - 5.0f;
    for (uint32_t i = 1; i <= vertices.m_size; ++i) {
        const auto &target = i < vertices.m_size ? vertices.m_data[i] : center;
        const vector3d direction = target - origin;
        const float t = std::not_equal_to<float>{}(direction.y, 0.0f) ? (ground - origin.y) / direction.y : -1.0f;
        const float x = direction.x * t;
        const float z = direction.z * t;
        if (t > 0.0f && std::sqrt(x * x + z * z) < distance) {
            points.m_data[points.m_size++] = vector2d{origin.x + x, origin.z + z};
        } else {
            const float length = std::sqrt(direction.x * direction.x + direction.z * direction.z);
            if (length > LARGE_EPSILON)
                points.m_data[points.m_size++] = vector2d{
                    origin.x + direction.x / length * distance, origin.z + direction.z / length * distance};
        }
    }
    compute_projected_hull(points);
}


bool square_intersects_projected_hull(const projected_frustum &points, const vector2d &min, const vector2d &max)
{
    vector2d hull_min{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    vector2d hull_max{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
    for (uint32_t i = 0; i < points.m_size; ++i) {
        hull_min.x = std::min(hull_min.x, points.m_data[i].x);
        hull_min.y = std::min(hull_min.y, points.m_data[i].y);
        hull_max.x = std::max(hull_max.x, points.m_data[i].x);
        hull_max.y = std::max(hull_max.y, points.m_data[i].y);
    }
    if (hull_max.x < min.x || hull_max.y < min.y || hull_min.x > max.x || hull_min.y > max.y)
        return false;
    const vector2d corners[]{min, vector2d{min.x, max.y}, max, vector2d{max.x, min.y}};
    for (uint32_t i = 0; i < points.m_size; ++i) {
        const auto &from = points.m_data[i];
        const auto &to = points.m_data[(i + 1) % points.m_size];
        int outside = 0;
        for (const auto &corner : corners)
            outside += (corner.y - from.y) * (to.x - from.x) - (corner.x - from.x) * (to.y - from.y) > 0.0f;
        if (outside == 4)
            return false;
    }
    return true;
}

struct region_visibility_visitor : subdivision_visitor {
    int count{};
    region *regions[20]{};

    static int visit_region(subdivision_visitor &base, const subdivision_node &node)
    {
        auto &visitor = static_cast<region_visibility_visitor &>(base);
        auto &reg = *reinterpret_cast<region *>(const_cast<subdivision_node *>(&node));
        if (reg.field_5C != region::visit_key2 && reg.is_loaded()
            && (!g_indoors || (reg.flags & (0x100u | 0x40000u)) != 0)) {
            reg.field_5C = region::visit_key2;
            if (visitor.count < 20)
                visitor.regions[visitor.count++] = &reg;
        }
        return 0;
    }
    region_visibility_visitor()
    {
        static const native_vtable table{visit_region, nullptr};
        m_vtbl = reinterpret_cast<std::intptr_t>(&table);
    }
};


void update_entity_render_budget()
{
    static Var<fixed_vector<ai::ai_core *, 32>> active_cores{0x0095CB50};
    struct render_budget {
        float distance_scale{1.0f};
        float distance_squared{10000.0f};
    };
    static Var<render_budget> budget{0x00921C88};
    static Var<float> hold_time{0x0095C7B0};
    active_cores().m_size = 0;
    auto *high = ai::ai_core::the_ai_core_list_high;
    if (high != nullptr) {
        for (auto *core : *high) {
            if ((core->field_64->field_4 & 0x800u) == 0)
                active_cores().push_back(core);
        }
    }
    auto *hero = g_world_ptr->get_hero_ptr(0);
    int nearby_count = 0;
    if (hero != nullptr) {
        const auto &position = hero->get_abs_position();
        for (auto *core : active_cores()) {
            auto *actor = core->field_64;
            const float distance = (actor->get_abs_position() - position).length2();
            if (distance < 2500.0f || (!actor->get_occluded_last_frame() && distance < 10000.0f))
                ++nearby_count;
        }
    }
    if (nearby_count >= 6) {
        budget().distance_scale = 0.25f;
        hold_time() = 10.0f;
        budget().distance_squared = 3600.0f;
    } else if (nearby_count >= 4) {
        budget().distance_scale = 0.5625f;
        hold_time() = 10.0f;
        budget().distance_squared = 6400.0f;
    } else {
        budget().distance_scale = 1.0f;
        hold_time() = std::max(hold_time() - g_world_ptr->time_manager.field_18, 0.0f);
    }
}

float render_ground_level()
{
    auto *hero = g_world_ptr->get_hero_ptr(0);
    auto *reg = hero != nullptr ? hero->get_primary_region() : nullptr;
    return reg != nullptr ? reg->get_ground_level() : 0.0f;
}
}

void wds_render_manager::build_render_data_regions(render_data &data, camera &cam)
{
#if STANDALONE_SYSTEM
    update_entity_render_budget();
    const float ground = render_ground_level();
    const float height = std::max(g_world_ptr->get_hero_or_marky_cam_ptr()->get_abs_position().y - ground, 0.0f);
    const float distance = std::clamp(std::sqrt(height * height + 27889.0f), 75.0f, 800.0f);
    projected_frustum points{};
    project_view_frustum(cam.get_abs_po().get_z_facing(), points, ground, distance);
    ++region::visit_key2;
    region_visibility_visitor visitor;
    static_region_list_methods::init();
    g_world_ptr->the_terrain->region_map->traverse_convex_hull_raster(points, visitor);
    static_region_list_methods::term();
    for (int i = 0; i < visitor.count; ++i) {
        auto *reg = visitor.regions[i];
        vector3d min, max;
        reg->obb->get_extents(&min, &max);
        if (square_intersects_projected_hull(points, vector2d{min.x - 20.0f, min.z - 20.0f},
            vector2d{max.x + 20.0f, max.z + 20.0f}))
            data.field_0.push_back(render_data::region_info{reg});
    }
#else
    THISCALL(0x00547000, this, &data, &cam);
#endif
}

void wds_render_manager::build_render_data_ents(render_data &data, camera &cam, int)
{


    static Var<fixed_vector<traversed_entity, 750> *> this_frame{0x0095C7B8};
    static Var<int> occlusion_status_base{0x0095A6D0};
    static Var<int> fade_rate{0x00921C90};
    static Var<uint8_t> fade_enabled{0x009391EC};
#if STANDALONE_SYSTEM
    static const bool initialized_fade = [] {
        fade_enabled() = 1;
        return true;
    }();
    (void)initialized_fade;
#endif
    if (traversed_entities_last_frame() == nullptr)
        traversed_entities_last_frame() = new fixed_vector<traversed_entity, 750>{};
    if (this_frame() == nullptr)
        this_frame() = new fixed_vector<traversed_entity, 750>{};
    auto &traversed = *this_frame();
    data.field_20 = cam.get_abs_position();
    ++entity::visit_key;
    if (g_disable_occlusion_culling != 3)
        ++occlusion_status_base();
    const auto *map = g_femanager.IGO->field_44;
    fade_rate() = map->field_5C4 || map->field_5C3 ? 80 : 10;

    struct visibility_visitor : subdivision_visitor {
        region *reg;
        fixed_vector<traversed_entity, 750> *entities;

        static int visit_entity(subdivision_visitor &base, const subdivision_node &node)
        {
            auto &visitor = static_cast<visibility_visitor &>(base);
            auto &ent = *reinterpret_cast<entity *>(const_cast<subdivision_node *>(&node));
            if (ent.field_5C != entity::visit_key) {
                ent.field_5C = entity::visit_key;
                visitor.entities->push_back(traversed_entity{&ent, visitor.reg});
            }
            return 0;
        }
        visibility_visitor(region *r, fixed_vector<traversed_entity, 750> &list) : reg(r), entities(&list)
        {
            static const native_vtable table{visit_entity, nullptr};
            m_vtbl = reinterpret_cast<std::intptr_t>(&table);
        }
    };

    auto *panel = comic_panels::get_panel_params();
    if (panel != nullptr && panel->field_4 != nullptr && panel->field_4->field_4C != nullptr) {

        struct panel_entity_groups {
            uint32_t prefix[26];
            _std::vector<entity_base_vhandle> groups[5];
        };
        auto &groups = reinterpret_cast<panel_entity_groups *>(panel->field_4->field_4C)->groups;
        ++entity::visit_key;
        for (unsigned group = 0; group != 5; ++group) {
            for (auto &handle : groups[group]) {
                auto *ent = static_cast<entity *>(handle.get_volatile_ptr());
                if (ent == nullptr || ent->field_5C == entity::visit_key)
                    continue;
                auto renderable = reinterpret_cast<bool(__fastcall *)(entity *, void *)>(
                    get_vfunc(ent->m_vtbl, 0x18C));
                if (!renderable(ent, nullptr) || !ent->is_visible())
                    continue;
                ent->field_5C = entity::visit_key;
                if ((panel->field_0 & (1u << group)) != 0) {
                    traversed.push_back(traversed_entity{ent, nullptr});
                    if ((ent->field_4 & 4) != 0) {
                        auto &conglom = static_cast<conglomerate &>(*ent);
                        conglom.field_110 = (conglom.field_110 & 0xFFFFF1FFu) | (group << 9);
                    }
                }
            }
        }
    }
    if (panel == nullptr || panel->field_4 == nullptr || (panel->field_0 & 0x20) != 0) {
        const float distance = std::min(geometry_manager::PROJ_FAR_PLANE_D,
            culling_params::entity_traversal_distance);
        for (const auto &entry : data.field_0) {
            auto *reg = entry.field_0;
            if (reg == nullptr || (reg->flags & 0x10) == 0)
                continue;
            projected_frustum points{};
            project_view_frustum(cam.get_abs_po().get_z_facing(), points,
                reg->obb != nullptr ? reg->get_ground_level() : 0.0f, distance);
            visibility_visitor visitor{reg, traversed};
            reg->visibility_map->traverse_convex_hull_raster(points, visitor);
        }
        if (!g_indoors) {
            auto **link = &field_98;
            while (*link != nullptr) {
                auto *node = *link;
                auto *ent = node->handle.get_volatile_ptr();
                bool keep = ent != nullptr && ent->is_ext_flagged(0x200)
                    && !ent->is_flagged(0x200000);
                if (keep) {
                    auto renderable = reinterpret_cast<bool(__fastcall *)(entity *, void *)>(
                        get_vfunc(ent->m_vtbl, 0x18C));
                    keep = renderable(ent, nullptr);
                }
                if (!keep) {
                    *link = node->next;
                    far_away_render_list_pool().remove(node);
                } else {
                    if (ent->field_5C != entity::visit_key) {
                        ent->field_5C = entity::visit_key;
                        traversed.push_back(traversed_entity{ent, ent->get_primary_region()});
                    }
                    link = &node->next;
                }
            }
        }
    }
    std::sort(traversed.m_data, traversed.m_data + traversed.m_size,
        [](const traversed_entity &left, const traversed_entity &right) {
            return reinterpret_cast<uintptr_t>(left.ent) < reinterpret_cast<uintptr_t>(right.ent);
        });
    stack_allocator saved;
    scratchpad_stack::save_state(&saved);
    using render_list = fixed_vector<render_data::entity_info, 400>;
    auto &visible = *static_cast<render_list *>(scratchpad_stack::alloc(sizeof(render_list)));
    visible.m_size = 0;
    const float far_squared = geometry_manager::PROJ_FAR_PLANE_D * geometry_manager::PROJ_FAR_PLANE_D;
    const bool use_occlusion = (g_disable_occlusion_culling & 1) == 0;
    const auto &last = *traversed_entities_last_frame();
    uint32_t previous = 0;
    for (const auto &entry : traversed) {
        auto &ent = *entry.ent;
        bool seen = false;
        int safety_counter = 0;
        while (previous < last.m_size) {
            const auto &old = last.m_data[previous];
            if (entry.ent == old.ent && (entry.reg == old.reg || old.reg == nullptr || entry.reg == nullptr)) {
                ++previous;
                seen = true;
                break;
            }
            if (reinterpret_cast<uintptr_t>(entry.ent) < reinterpret_cast<uintptr_t>(old.ent))
                break;
            ++previous;
            if (++safety_counter >= 750)
                break;
        }
        if (ent.rendered_last_frame_override) {
            seen = true;
            ent.rendered_last_frame_override = 0;
        }
        vector3d center;
        auto get_center = reinterpret_cast<vector3d *(__fastcall *)(entity *, void *, vector3d *)>(
            get_vfunc(ent.m_vtbl, 0x2C));
        get_center(&ent, nullptr, &center);
        auto get_radius = reinterpret_cast<float(__fastcall *)(entity *, void *)>(
            get_vfunc(ent.m_vtbl, 0x28));
        const float radius = get_radius(&ent, nullptr);
        const auto group = static_cast<uint8_t>(ent.field_3E);
        auto *reg = ent.get_primary_region();
        const vector3d distance_center = group != 0 && reg != nullptr
            ? vector3d{reg->field_48[group].x, reg->field_48[group].y, reg->field_48[group].z}
            : center;
        const vector3d delta = distance_center - data.field_20;

        const float distance_squared = delta.x * delta.x + delta.z * delta.z
            + (group != 0 && reg != nullptr ? 0.0f : delta.y * delta.y);
        const float fade_distance = group != 0 && reg != nullptr
            ? reg->field_44[group] : distance_fader::fade_distances2()[ent.field_4 & 0xF];
        int timer;
        if (distance_squared < std::min(far_squared, fade_distance))
            timer = !fade_enabled() || !seen ? 255 : std::min(255, int(ent.m_timer) + fade_rate());
        else
            timer = !seen ? 0 : std::max(0, int(ent.m_timer) - fade_rate());
        ent.m_timer = static_cast<uint8_t>(timer);
        if (timer == 0)
            continue;
        bool hidden = !geometry_manager::world_space_frustum.sub_5CC030(center.x, center.y, center.z, radius);
        if (!hidden && use_occlusion)
            hidden = occlusion::sphere_occluded(center, radius, 3);
        ent.field_44 = occlusion_status_base() - int(hidden);
        if (!hidden)
            visible.push_back(render_data::entity_info{center, radius, distance_squared,
                0.0039215689f * float(timer), &ent});
    }
    for (const auto &entry : visible) {
        auto render = reinterpret_cast<void(__fastcall *)(entity *, void *, float)>(
            get_vfunc(entry.ent->m_vtbl, 0x1AC));
        render(entry.ent, nullptr, entry.fade);
    }
    std::swap(traversed_entities_last_frame(), this_frame());
    this_frame()->m_size = 0;
    scratchpad_stack::restore_state(saved);
}

void wds_render_manager::clear_colorvol_scene()
{
    USColorVolShaderSpace::gUSColorVolScene() = nullptr;
}

void wds_render_manager::render_meshes(camera &cam)
{
#if STANDALONE_SYSTEM
    const float far_plane = std::min(geometry_manager::PROJ_FAR_PLANE_D, 167.0f);
    const auto *map = g_femanager.IGO->field_44;
    const float altitude_allowance = map->field_5C4 || map->field_5C3 ? 0.0f : 250.0f;
    const auto &position = cam.get_abs_position();
    for (const auto &entry : field_30.field_0) {
        auto *reg = entry.field_0;
        if (g_indoors && (reg->flags & (0x100u | 0x40000u)) == 0)
            continue;
        for (int i = static_cast<int>(reg->meshes->size()) - 1; i >= 0; --i) {
            const uint8_t group = reg->field_3C[i];
            if (group != 0) {
                const auto &center = reg->field_48[group];
                const float dx = position.x - center.x;
                const float dz = position.z - center.z;
                if (dx * dx + dz * dz >= reg->field_44[group])
                    continue;
            }
            auto *mesh = (*reg->meshes)[i];
            const auto &center = mesh->SphereCenter;
            const float dx = center.x - position.x;
            const float dz = center.z - position.z;
            const float distance = std::max(std::sqrt(dx * dx + dz * dz) - mesh->SphereRadius, 0.0f)
                + std::max(position.y - (mesh->SphereRadius + altitude_allowance), 0.0f);
            if (distance >= far_plane || ((g_disable_occlusion_culling & 2) == 0
                && !geometry_manager::world_space_frustum.sub_5CC030(center.x, center.y, center.z, mesh->SphereRadius)))
                continue;
            const int fade_index = distance_fader::estimate_fade_index_for_bounding_sphere(mesh->SphereRadius);
            const float fade_start = std::min(distance_fader::fade_distances()[fade_index], far_plane - 10.0f);
            const float fade = std::clamp(1.0f - (distance - fade_start) * 0.1f, 0.0f, 1.0f);
            if (fade > 0.0f)
                render_region_mesh(mesh, fade);
        }
    }
#else
    THISCALL(0x0053CED0, this, &cam);
#endif
}

void wds_render_manager::render_legos(camera &cam)
{
#if STANDALONE_SYSTEM
    const auto *map = g_femanager.IGO->field_44;
    if (map->field_5C4 && !map->field_5C3)
        return;
    const float ground = render_ground_level();
    const float height = std::max(g_world_ptr->get_hero_or_marky_cam_ptr()->get_abs_position().y - ground, 0.0f);
    const float distance = std::abs(std::min(geometry_manager::PROJ_FAR_PLANE_D, 167.0f));
    const float scale = 1.0f - std::clamp(height * 0.0021739129f, 0.0f, 1.0f) * 0.84848487f;
    projected_frustum points{};
    project_view_frustum(cam.get_abs_po().get_z_facing(), points, ground, distance);
    static_lego_list_methods::prepare_for_scene_traversal();
    for (const auto &entry : field_30.field_0) {
        auto *reg = entry.field_0;
        if (g_indoors && (reg->flags & (0x100u | 0x40000u)) == 0)
            continue;
        auto *root = reg->field_9C;
        if (root == nullptr || root->field_C == nullptr)
            continue;
        ++subdivision_node_obb_base::visit_key();
        lego_render_visitor visitor{reg, &geometry_manager::world_space_frustum, cam.get_abs_position(), scale};
        static_lego_list_methods::init_region_traversal(
            static_cast<uint16_t>(root->field_14), root->field_8, reg->bitvector_of_legos_rendered_last_frame);
        root->field_C->traverse_convex_hull_raster(points, visitor);
        visitor.render_buffered_legos();
        static_lego_list_methods::term_region_traversal(reg->bitvector_of_legos_rendered_last_frame);
    }
#else
    THISCALL(0x0053D270, this, &cam);
#endif
}

static int g_region_meshes_occluded_this_frame;
static int g_region_meshes_rendered_this_frame;

void wds_render_manager::sub_53D560(camera &a2)
{
    TRACE("wds_render_manager::sub_53D560");

    if constexpr (1) {
        g_region_meshes_occluded_this_frame = 0;
        g_region_meshes_rendered_this_frame = 0;
        if (debug_render_get_bval(REGION_MESHES)) {
            this->render_meshes(a2);
        }

        if (debug_render_get_bval(LEGOS)) {
            this->render_legos(a2);
        }
    } else {
        THISCALL(0x0053D560, this, &a2);
    }
}

void wds_render_manager_patch()
{
    {
        FUNC_ADDRESS(address, &wds_render_manager::render_region_mesh);
        REDIRECT(0x0053D234, address);

        REDIRECT(0x00537465, FastListAddMesh);
    }

    REDIRECT(0x0054B410, debug_render_get_bval);

    {
        FUNC_ADDRESS(address, &wds_render_manager::render);
        REDIRECT(0x0054E52D, address);
    }

    REDIRECT(0x0054B265, update_camera_teleport);

    {
        FUNC_ADDRESS(address, &wds_render_manager::init_level);
        REDIRECT(0x0055B355, address);
    }

    {
        FUNC_ADDRESS(address, &wds_render_manager::render_stencil_shadows);
        REDIRECT(0x0054E585, address);
    }

    {
        FUNC_ADDRESS(address, &wds_render_manager::build_render_data_regions);
        REDIRECT(0x0054B3FB, address);
    }

    {
        FUNC_ADDRESS(address, &wds_render_manager::build_render_data_ents);
        REDIRECT(0x0054B428, address);
    }

    {
        FUNC_ADDRESS(address, &wds_render_manager::sub_53D560);
        REDIRECT(0x0054B403, address);
    }
}
