#include "debug_render.h"
#include "app.h"
#include "camera.h"
#include "color.h"
#include "common.h"
#include "debug_string.h"
#include "femanager.h"
#include "fixedstring.h"
#include "func_wrapper.h"
#include "game.h"
#include "geometry_manager.h"
#include "line_info.h"
#include "local_collision.h"
#include "ngl.h"
#include "oldmath_po.h"
#include "os_developer_options.h"
#include "pcuv_shadermaterial.h"
#include "render_text.h"
#include "trace.h"
#include "variables.h"
#include "vector2di.h"
#include "vector3d.h"
#include "vtbl.h"
#include "wds_render_manager.h"

#include <ngl_dx_scene.h>

#include <ngl_mesh.h>

#include <cassert>
#include <cmath>

VALIDATE_SIZE(debug_line, 0x20);
VALIDATE_SIZE(debug_lines_t, 0x2808);

debug_lines_t debug_lines{};

debug_line::debug_line(const vector3d &a2, const vector3d &a3, color32 a4, Float a5)
    : field_0(a2), field_C(a3), field_18(a4), field_1C(a5 * 1.0) {}

void sub_CB4800(const vector3d &a1,
                const vector3d &arg4,
                int a3,
                float a4,
                nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator *a6) {
    auto v44 = arg4 - a1;

    auto a3a = v44.length2();
    if (a3a >= 9.9999997e-10) {
        a3a = std::sqrt(a3a);
        auto v42 = v44 / a3a;

        auto sub_CB4BE0 = []() -> vector3d {
            auto *v1 = g_game_ptr()->get_current_view_camera(0);
            auto v2 = v1->get_abs_position();

            return v2;
        };

        auto camera_pos = sub_CB4BE0();
        auto v40 = camera_pos - a1;
        auto v39 = camera_pos - arg4;

        auto sub_68FD4B = [](float a1) -> double { return 1.0 / std::sqrt(a1); };

        auto v38 = v40.length2();
        auto v37 = v39.length2();
        if (v38 != 0.0) {
            auto v17 = 0.5 * a4;
            auto a2 = sub_68FD4B(v38) * v17;

            v40 *= a2;
        }
        if (v37 != 0.0) {
            auto v18 = 0.5 * a4;
            auto a2a = sub_68FD4B(v37) * v18;

            v39 *= a2a;
        }

        v40 += a1;
        v39 += arg4;

        auto sub_67C462 = [](float a1) -> double { return 1.0 - 0.75 / ((0.25 * 0.25) * a1 + 1.0); };

        auto v36 = sub_67C462(v38);
        auto v35 = sub_67C462(v37);
        auto v5 = a1 + arg4;
        auto v6 = v5 * 0.5;

        auto v34 = v6 - camera_pos;
        auto v33 = vector3d::cross(v34, v42);

        if (v33.length2() >= 9.9999997e-10 || geometry_manager::is_scene_analyzer_enabled()) {
            auto sub_665974 = [&sub_68FD4B](vector3d &self, float a2) -> void {
                auto v4 = self.length2();
                if (v4 > (0.0000099999997 * 0.0000099999997)) {
                    auto v3 = sub_68FD4B(v4) * a2;
                    self *= v3;
                }
            };

            sub_665974(v33, a4 * 0.5);

            vector3d v32{};
            vector3d v31{};
            vector3d v30{};
            vector3d a4a{};

            auto v7 = v33 * v36;
            auto v8 = v40 - v7;
            v32 = v8;

            auto v9 = v33 * v36;
            auto v10 = v40 + v9;
            v31 = v10;

            auto v11 = v33 * v35;
            auto v12 = v39 + v11;
            v30 = v12;

            auto v13 = v33 * v35;
            auto v14 = v39 - v13;
            a4a = v14;

            sub_CB3F80(v32, v31, v30, a4a, a3, a6);
        }
    }
}

void debug_line::render(nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator *a2) {
    auto v3 = this->field_1C;
    auto v2 = color32::to_int(this->field_18);
    sub_CB4800(this->field_0, this->field_C, v2, v3, a2);
}

void add_debug_line(vector3d a1, vector3d a2, color32 a7, Float a8) {
    auto v4 = debug_line{a1, a2, a7, a8};
    debug_lines.add_line(v4);
}

void render_debug_lines() {
    if (debug_lines.m_size) {
        debug_lines.render();
    }
}

void clear_debug_lines() {
    debug_lines.clear();
}

nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator sub_CBFF50(int a3)
{
    sub_578420(1u);
    nglMaterialBase *material = nullptr;
    if ( debug_material != nullptr )
    {
        material = (nglMaterialBase *)&debug_material->field_4;
    }

    auto v2 = sub_507920(material, 4 * a3, a3, 0, 0, 5, true);
    auto result = v2->CreateIterator();
    return result;
}

void debug_lines_t::render() {
    nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator iter{};

    int a2 = this->m_size;
    while (a2 > 20) {
        a2 -= 20;

        nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator a1 = sub_CBFF50(20);

        iter = a1;

        for (int i = 20; --i >= 0; this->field_0[i + a2].render(&iter)) {
            ;
        }

        sub_57F3C0();
    }

    nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator v2 = sub_CBFF50(a2);
    iter = v2;

    while (--a2 >= 0) {
        this->field_0[a2].render(&iter);
    }

    sub_57F3C0();

    if (this->field_2804) {
        this->clear();
    }
}

void debug_lines_t::clear() {
    this->m_size = 0;
}

void debug_lines_t::add_line(const debug_line &a2) {
    if (this->m_size >= 320) {
        for (int i = 1; i < 320; ++i) {
            this->field_0[i - 1] = this->field_0[i];
        }

        --this->m_size;
    }

    this->field_0[this->m_size] = a2;
    ++this->m_size;
}

VALIDATE_SIZE(debug_lines_t, 0x2808);

debug_spheres_t debug_spheres{};

void debug_sphere::render() {
    auto v3 = this->field_10;
    auto radius = this->field_0.radius;
    auto center = this->field_0.center;
    render_debug_hemisphere(center, radius, v3);
}

void debug_spheres_t::add_sphere(const debug_sphere &a2) {
    if (this->m_size >= 512) {
        for (int i = 1; i < 512; ++i) {
            this->field_0[i - 1] = this->field_0[i];
        }

        --this->m_size;
    }

    this->field_0[this->m_size] = a2;
    ++this->m_size;
}

void debug_spheres_t::render() {
    for (auto i = 0u; i < this->m_size; ++i) {
        this->field_0[i].render();
    }

    if (this->field_2804) {
        this->clear();
    }
}

void debug_spheres_t::clear() {
    this->m_size = 0;
}

void add_debug_sphere(vector3d a1, Float a4, color32 a5) {
    auto v5 = a5;
    auto v3 = sphere{a1, a4};
    auto v4 = debug_sphere{v3, v5};
    debug_spheres.add_sphere(v4);
}

void render_debug_spheres() {
    debug_spheres.render();
}

void sub_CB3F80(const vector3d &a1,
                const vector3d &a2,
                const vector3d &a3,
                const vector3d &a4,
                int a5,
                void *a6)
{

    auto *iter = static_cast<nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator *>(a6);


    iter->BeginStrip(4);
    iter->Write(a1, a5, 0.0, 0.0);
    ++iter->field_8;
    iter->Write(a2, a5, 0.0, 0.0);
    ++iter->field_8;
    iter->Write(a4, a5, 0.0, 0.0);
    ++iter->field_8;
    iter->Write(a3, a5, 0.0, 0.0);
    ++iter->field_8;

    {
        struct {
            int field_0;
            struct {
                int field_0;
                nglMeshSection *field_4;
            } * field_4;

        } *local_iter = static_cast<decltype(local_iter)>(a6);

        auto *v40 = local_iter->field_4->field_4;

        if ((v40->Flags & 0x40000) == 0)
        {
            v40->m_vertexBuffer->lpVtbl->Unlock(v40->m_vertexBuffer);
        }
    }
}

void render_beam(const vector3d &a1, const vector3d &a2, color32 a3, Float a4, bool )
{
    TRACE("render_beam");

    auto a1a = a2 - a1;
    if (a1a.length2() >= 9.9999997e-10 && sub_578420(1u)) {
        nglMaterialBase *v7 = nullptr;
        if (debug_material != nullptr) {
            v7 = CAST(v7, &debug_material->field_4);
        }

        auto *v4 = sub_507920(v7, 4, 1, 0, nullptr, 5, true);

        nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator iter;
        nglVertexDef__GetIterator(v4, 0, &iter);

        auto v5 = color32::to_int(a3);
        sub_CB4800(a1, a2, v5, a4, &iter);
        sub_57F3C0();
    }
}

void render_quad(const vector3d &pt1, const vector3d &pt2, const vector3d &pt3, const vector3d &pt4, color32 col, bool double_sided)
{
    if ( sub_578420(1u) )
    {
        nglMaterialBase *a1a = nullptr;
        if ( debug_material != nullptr )
        {
            a1a = (nglMaterialBase *)&debug_material->field_4;
        }

        auto *v6 = sub_507920(a1a, 4, 1, 0, nullptr, 5, true);
        auto a6 = v6->CreateIterator();
        auto v7 = color32::to_int(col);
        sub_CB3F80(pt1, pt2, pt3, pt4, v7, &a6);
        sub_57F3C0();
    }
}

Var<int[51]> min_values{0x009617A0};

Var<int[51]> max_values{0x009227E8};

Var<int[34]> debug_render_items = {0x00960D30};

int debug_render_get_ival(debug_render_items_e item) {
    assert(item < DEBUG_RENDER_ITEMS_COUNT);

    return debug_render_items()[item];
}

void debug_render_set_ival(debug_render_items_e a1, int a2)
{
    auto max = a2;
    if ( a2 > debug_render_get_max(a1) )
    {
        max = debug_render_get_max(a1);
    }

    auto min = debug_render_get_min(a1);
    if ( max >= min )
    {
        debug_render_items()[a1] = max;
    }
    else
    {
        debug_render_items()[a1] = min;
    }
}

bool debug_render_get_bval(debug_render_items_e item) {
    assert(item < DEBUG_RENDER_ITEMS_COUNT);

    return debug_render_items()[item] != 0;
}


int debug_render_get_min(debug_render_items_e item)
{
    assert(item < DEBUG_RENDER_ITEMS_COUNT);

    return min_values()[item];
}

int debug_render_get_max(debug_render_items_e item)
{
    assert(item < DEBUG_RENDER_ITEMS_COUNT);
    return max_values()[item];
}

//std::vector<vector3d> s_debug_hemisphere_pos{};

nglMesh *s_debug_hemisphere{nullptr};
nglMesh *s_debug_cylinder{nullptr};
nglMesh *s_debug_box{nullptr};
nglMesh *s_debug_disc{nullptr};

PCUV_ShaderMaterial *debug_material = nullptr;
PCUV_ShaderMaterial *dword_15BCE44 = nullptr;

void debug_render_init()
{
    TRACE("debug_render_init");

    CDECL_CALL(0x005784F0);

    int a4 = 194;

    debug_material = new PCUV_ShaderMaterial{nglWhiteTex(), nglBlendModeType{2}, 0, a4};

    if (!g_is_the_packer()) {
        auto *mesh_file = nglLoadMeshFile(tlFixedString{"debugobj"});
        assert(mesh_file != nullptr);

        s_debug_hemisphere = nglGetMesh(tlFixedString{"debug_hemisphere"}, true);
        assert(s_debug_hemisphere != nullptr);

        s_debug_cylinder = nglGetMesh(tlFixedString{"debug_cylinder"}, true);
        assert(s_debug_cylinder != nullptr);

        s_debug_box = nglGetMesh(tlFixedString("debug_box"), true);
        assert(s_debug_box != nullptr);

        s_debug_disc = nglGetMesh(tlFixedString{"debug_disc"}, true);
        assert(s_debug_disc != nullptr);

        dword_15BCE44 = new PCUV_ShaderMaterial{nglWhiteTex(), nglBlendModeType{2}, 0, a4};
        assert(debug_strings == nullptr);

        debug_strings = new fixed_vector<debug_string_t, 25>{};
        assert(debug_strings != nullptr);
    }
}

void debug_render_done() {
    if (!g_is_the_packer()) {
        if (s_debug_box != nullptr) {
            nglReleaseMeshFile(tlFixedString{"debugobj"});
            s_debug_disc = nullptr;
            s_debug_box = nullptr;
            s_debug_cylinder = nullptr;
            s_debug_hemisphere = nullptr;
        }

        if (debug_material != nullptr) {
            auto &finalize = get_vfunc(debug_material->m_vtbl, 0x0);

            finalize(debug_material, 1);
        }

        debug_material = nullptr;
    }
}

void render_debug_box(const vector3d &a2, const vector3d &a3, color32 a4) {
    vector3d v16 = (a2 + a3) * 0.5;

    vector3d v15 = (a3 - a2) * 0.5;

    static constexpr vector3d LEFT{1.0, 0, 0};
    static constexpr vector3d UP{0.0, 1.0, 0};
    static constexpr vector3d FORWARD{0.0, 0, 1.0};

    matrix4x4 v14;
    v14[0] = {LEFT, 0};
    v14[1] = {UP, 0};
    v14[2] = {FORWARD, 0};
    v14[3] = {v16, 1};

    nglMeshParams v13{};

    nglParamSet<nglShaderParamSet_Pool> v12{1};

    v13.Flags |= 2u;

    v13.Scale[0] = v15[0];
    v13.Scale[1] = v15[1];
    v13.Scale[2] = v15[2];
    v13.Scale[3] = 1.0;

    auto set_color = [](nglParamSet<nglShaderParamSet_Pool> *a1, color32 a2) -> void {
        color32 v2{255, 255, 255, 255};
        if (a2 != v2) {
            color v9 = a2.to_color();
            auto *mem = nglListAlloc(16, 16);
            auto *v4 = new (mem) vector4d{v9.r, v9.g, v9.b, 0};

            int v6[2];
            v6[1] = (int) v4;

            nglTintParam param{v4};

            auto sub_66058C = [](nglParamSet<nglShaderParamSet_Pool> *self,
                                 nglTintParam &param) -> void {
                self->Set(nglTintParam::ID(), param.field_0);
            };

            sub_66058C(a1, param);
        }
    };

    set_color(&v12, a4);
    math::MatClass<4, 3> v9 = math::MatClass<4, 3>{v14};
    nglListAddMesh(s_debug_box, v9, &v13, &v12);
}

static color32 stru_1589F90[] = {color32{0, 255, 0, 128},
                                 color32{255, 255, 0, 128},
                                 color32{0, 255, 255, 128},
                                 color32{255, 255, 255, 128},
                                 color32{255, 0, 255, 128},
                                 color32{255, 128, 128, 128},
                                 color32{80, 80, 255, 128},
                                 color32{255, 80, 80, 128}};


void sub_CB99A0(const vector3d &arg0, color32 a2, float a5, const char *a4)
{
    if ( os_developer_options::instance()->get_flag(mString {"SHOW_DEBUG_TEXT"}) )
    {
        auto *v4 = g_femanager().GetFont(0);

        uint32_t v39, v38;
        nglGetStringDimensions(v4, &v39, &v38, "M");
        auto v37 = (float)v38;
        auto *v5 = app::instance();
        auto *v6 = v5->m_game;
        auto *v7 = v6->get_current_view_camera(0);
        auto &v8 = v7->get_abs_po();
        auto &v9 = v8.get_z_facing();
        auto v35 = v9;
        auto &v11 = v7->get_abs_position();
        vector3d a3a = v11;
        auto v33 = arg0 - a3a;
        if ( v33 != ZEROVEC )
        {
            v33.normalize();
            bool v32 = false;
            auto v12 = arg0 - a3a;
            auto v31 = v12.length2();
            if ( v32
                || debug_render_get_ival(OBBS)
                || dot(v35, v33) > 0.0
                && v31 < 10000.0
                && sub_50D220(a3a, arg0, nullptr))
            {
                auto &v14 = geometry_manager::get_xform((geometry_manager::xform_t) 4);
                auto v30 = v14 * arg0;
                if ( v30[2] > 0.0 )
                {
                    auto &v15 = geometry_manager::get_xform((geometry_manager::xform_t)5);
                    auto v16 = sub_501B20(v15, v30);
                    v30 = v16;

                    char *v29 = nullptr;
                    auto *a1 = a4;
                    for (;; a1 = v29 + 1 )
                    {
                        v29 = strchr(a1, 10);
                        if ( v29 == nullptr )
                        {
                            break;
                        }

                        v29[0] = '\0';
                        mString v25 {a1};
                        auto v17 = vector2di(v30[0], v30[1]);
                        render_text(v25, v17, a2, 0.0, a5);
                        v30[1] += v37;
                    }

                    mString v27 {a1};
                    auto v18 = vector2di(v30[0], v30[1]);
                    render_text(v27, v18, a2, 0.0, a5);
                }
            }
        }
    }
}

void sub_6A2635(const vector3d &arg0, color32 arg4, float a3, const char *a2, ...)
{
    va_list va;
    va_start(va, a2);

    char a1[2048] {};
    vsprintf(a1, a2, va);
    sub_CB99A0(arg0, arg4, a3, a1);
}


void print_3d_text(const vector3d &arg0, color32 arg4, const char *a2, ...)
{
    char a1[2048] {};
    va_list va;

    va_start(va, a2);
    vsprintf(a1, a2, va);
    sub_CB99A0(arg0, arg4, 1.0, a1);
}

void debug_render_line_info(line_info *a1, [[maybe_unused]] char a2, int a3)
{
    for (int i = 0; i <= a3; ++i) {
        color32 blue {255, 0, 0, 128};
        render_debug_hemisphere(a1[i].field_0, 0.1, blue);

        color32 red {0, 0, 255, 128};
        render_debug_hemisphere(a1[i].field_C, 0.1, red);

        render_beam(a1[i].field_0, a1[i].field_C, stru_1589F90[i % 8], 0.050000001, false);

        if (a1[i].field_0 != ZEROVEC ||
            a1[i].field_C != ZEROVEC)
        {
            auto v11 = i;
            auto v8 = stru_1589F90[i % 8];
            auto v5 = a1[i].field_0 + a1[i].field_C;
            auto v6 = v5 * 0.5;
            print_3d_text(v6, v8, "%c%d", a2, v11);
        }

        if ( a1[i].collision ) {
            const color32 black {255, 255, 255, 128};
            render_debug_hemisphere(a1[i].hit_pos, 0.1, black);
            const color32 white {0, 0, 0, 128};
            auto v9 = a1[i].hit_pos + a1[i].hit_norm;
            render_beam(a1[i].hit_pos, v9, white, 0.050000001, false);
        }
    }
}

void debug_render_line_info() {
    //sp_log("%d", num_debug_line_info[1]);

    debug_render_line_info(debug_line_info[1], 'c', num_debug_line_info[1]);
    debug_render_line_info(debug_line_info[0], 's', num_debug_line_info[0]);
}

