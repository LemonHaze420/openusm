#include "wds_render_manager.h"

#include "GL/gl.h"

#include "aeps.h"
#include "beam.h"
#include "bitvector.h"
#include "camera.h"
#include "camera_teleport_update_visitor.h"
#include "city_lod.h"
#include "comic_panels.h"
#include "common.h"
#include "culling_params.h"
#include "cut_scene_player.h"
#include "debug_render.h"
#include "femanager.h"
#include "filespec.h"
#include "func_wrapper.h"
#include "game.h"
#include "geometry_manager.h"
#include "glass_house_manager.h"
#include "hierarchical_entity_proximity_map.h"
#include "igofrontend.h"
#include "line_info.h"
#include "loaded_regions_cache.h"
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
#include "sector2d.h"
#include "shadow.h"
#include "subdivision_obb.h"
#include "trace.h"
#include "terrain.h"
#include "us_colorvol.h"
#include "us_pcuv_shader.h"
#include "utility.h"
#include "variables.h"
#include "vector2di.h"
#include "vector2d.h"
#include "wds.h"

#include <cassert>
#include <cmath>

VALIDATE_SIZE(wds_render_manager, 156u);

struct traversed_entity {
    entity_base_vhandle m_handle;
    int field_4;
};
static Var<fixed_vector<traversed_entity, 750> *> traversed_entities_last_frame{0x0095C7B4};

wds_render_manager::wds_render_manager()
{
    this->field_30.sub_56FCB0();

    this->field_0 = new RenderOptimizations();
    this->field_94 = nullptr;
    this->field_98 = 0;

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

void wds_render_manager::render_region_mesh(nglMesh *a2, Float fade)
{
    TRACE("wds_render_manager::render_region_mesh");

    sp_log("fade = %f", fade);

    THISCALL(0x00537390, this, a2, fade);
}

int wds_render_manager::add_far_away_entity(vhandle_type<entity> a2)
{
    return THISCALL(0x0052A470, this, a2);
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
    THISCALL(0x0053DA50, this);
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

    if constexpr (0) {
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

    if constexpr (0) {
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
    CDECL_CALL(0x00520E60);
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
                sp_log("no camera region!!!!");
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
    THISCALL(0x0056FCB0, this);
}

void wds_render_manager::frame_advance(Float a2)
{
    TRACE("wds_render_manager::frame_advance");

    THISCALL(0x0054ADE0, this, a2);
}

void wds_render_manager::render_stencil_shadows(const camera &a2)
{
    TRACE("wds_render_manager::render_stencil_shadows");

    THISCALL(0x0053D5E0, this, &a2);
}

void wds_render_manager::build_render_data_regions(render_data &a2, camera &a3)
{
    TRACE("wds_render_manager::build_render_data_regions");

    THISCALL(0x00547000, this, &a2, &a3);
}

void wds_render_manager::build_render_data_ents(render_data &a2, camera &a3, int a4)
{
    TRACE("wds_render_manager::build_render_data_ents");

    THISCALL(0x00547250, this, &a2, &a3, a4);
}

void wds_render_manager::clear_colorvol_scene()
{
    USColorVolShaderSpace::gUSColorVolScene() = nullptr;
}

void wds_render_manager::render_meshes(camera &a2)
{
    TRACE("wds_render_manager::render_meshes");

    THISCALL(0x0053CED0, this, &a2);
}

void wds_render_manager::render_legos(camera &a2)
{
    TRACE("wds_render_manager::render_legos");

    THISCALL(0x0053D270, this, &a2);
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
