#include "comic_panels.h"

#include "comic_page_camera.h"
#include "common.h"
#include "entity_base.h"
#include "func_wrapper.h"
#include "game.h"
#include "geometry_manager.h"
#include "ngl.h"
#include "memory.h"
#include "ngl_params.h"
#include "ngl_scene.h"
#include "cut_scene_player.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"
#include "vector3d.h"
#include "vtbl.h"
#include "wds.h"
#include <cstring>
#include "scene_anim.h"
#include "panels/nalpanel/panel_anim_inst.h"
#include "panels/nalpanel/panel_pose_skel.h"
#include "ngl/shaders/us_panel.h"
#include "ngl_vertexdef.h"
#include "ngl_mesh.h"
#include <algorithm>
#include "glam_camera.h"
#include "marky_camera.h"
#include "quaternion.h"
#include "string_hash.h"
#include "component.h"
#include "shadow.h"
#include "potential_shadow.h"
#include <cmath>
#include "nglsortinfo.h"
#include "cut_scene.h"
#include "custom_math.h"
#include <functional>

VALIDATE_SIZE(comic_panels::panel_component_camera, 0x4C);

VALIDATE_OFFSET(comic_panels::panel, field_64, 0x64u);

VALIDATE_SIZE(comic_panels::panel_component_base, 0x10u);

VALIDATE_SIZE(comic_panels::panel_component::render_info, 0x148u);

VALIDATE_SIZE(comic_panels::panel_params_t, 0xD4);

namespace comic_panels {

glam_camera *(&glamour_cams)[4] = var<glam_camera *[4]>(0x0096F7A4);
Var<camera *> current_view_camera{0x0096F7B4};

Var<panel *> game_play_panel{0x0096F7D4};

Var<fixed_vector<panel *, 48>> panels{0x0096F9F8};

bool &world_has_been_rendered = var<bool>(0x0096F7A0);
void clear_color_rect(aarect<float, vector2d> &, color &, math::MatClass<4, 3> &);
namespace {
void nglAdjustViewForHiresScreenshot()
{
    const int tile = var<int>(0x00971F38);
    nglCurScene->vx1 = var<float *>(0x00971EFC)[tile];
    nglCurScene->vy1 = var<float *>(0x00971EF0)[tile];
    nglCurScene->vx2 = var<float *>(0x00971EF8)[tile];
    nglCurScene->vy2 = var<float *>(0x00971EF4)[tile];
    nglCurScene->field_3E4 = true;
}
int component_index(const nalComp::nalCompSkeleton &skeleton, unsigned kind)
{
    static const char *names[]{"PanelBase",
                               "PanelScissor",
                               "PanelGutter",
                               "PanelTexture.1",
                               "PanelTexture.2",
                               "PanelCamera.1",
                               "PanelCamera.2",
                               "PanelColor.1",
                               "PanelColor.2",
                               "PanelEffect",
                               "PanelCharacter.1",
                               "PanelCharacter.2"};
    static const char *types[]{"PanelBase",
                               "PanelScissor",
                               "PanelGutter",
                               "PanelTexture",
                               "PanelTexture",
                               "PanelCamera",
                               "PanelCamera",
                               "PanelColor",
                               "PanelColor",
                               "PanelEffect",
                               "PanelCharacter",
                               "PanelCharacter"};
    return skeleton.GetCompIxFromName({static_cast<int>(to_hash(names[kind])), to_hash(types[kind])});
}
void *component_pose(nalPanel::nalPanelPose &pose, unsigned kind)
{
    const int index = component_index(*pose.field_4, kind);
    return index == -1 || pose.field_4->ConvertCompIxToPoseIx(index) == -1 ? nullptr : pose.GetComponentPoseData(index);
}
void *component_animation(nalPanel::nalPanelAnim &anim, unsigned kind)
{
    return anim._GetPerAnimDataFromComponentIx(component_index(*anim.field_30, kind));
}
void __fastcall base_pose(panel_component_base *self, void *, nalPanel::nalPanelPose *pose)
{
    if (auto *data = static_cast<const char *>(component_pose(*pose, 0))) {
        std::memcpy(&self->field_8, data + 36, 4);
        self->field_C = (data[40] & 1) != 0;
    }
}
unsigned panel_texture_bytes = 0;
nglTexture *create_panel_texture(unsigned width, unsigned height)
{
    const unsigned bytes = 2 * width * height;
    if (panel_texture_bytes + bytes > 10000000)
        return nullptr;
    panel_texture_bytes += bytes;
    return nglCreateTexture(0x1201, width, height, 0, true);
}
void release_panel_texture(nglTexture *texture)
{
    panel_texture_bytes -= 2 * texture->m_width * texture->m_height;
    nglDestroyTexture(texture);
}
void __fastcall camera_resources(panel_component_camera *self, void *)
{
    if (self->field_44)
        release_panel_texture(self->field_44);
    self->field_44 = nullptr;
}
void __fastcall camera_destroy(panel_component_camera *self, void *, bool release)
{
    camera_resources(self, nullptr);
    if (release)
        mem_dealloc(self, sizeof(panel_component_camera));
}
void __fastcall camera_animation(panel_component_camera *self, void *, nalPanel::nalPanelAnim *anim)
{
    if (auto *data = static_cast<const uint32_t *>(component_animation(*anim, self->field_28 + 5))) {
        self->field_48 = (self->field_48 & ~1) | (data[1] & 1);
        const char *name = reinterpret_cast<const char *>(data + 2);
        unsigned slot = 0;
        for (; slot < std::min(data[0], 4u); ++slot) {
            self->register_camera(slot, name);
            name += std::strlen(name) + 1;
        }
        for (; slot < 4; ++slot)
            self->register_camera(slot, nullptr);
    }
}
void __fastcall camera_pose(panel_component_camera *self, void *, nalPanel::nalPanelPose *pose)
{
    if (self->field_20)
        return;
    if (auto *data = static_cast<const uint32_t *>(component_pose(*pose, self->field_28 + 5))) {
        std::memcpy(&self->field_24, data + 5, 4);
        const unsigned flags = data[6];
        if (std::equal_to<float>{}(self->field_24, 0.0f) || !(flags & 1)) {
            self->field_2C = -1;
            self->field_48 &= ~4;
        } else {
            self->field_2C = data[4];
            self->field_48 = (self->field_48 & ~(2 | 4 | 8 | 16)) | ((flags & 2) ? 4 : 0) | ((flags & 4) ? 8 : 0) |
                             ((flags & 8) ? 16 : 0) | ((flags & 512) ? 2 : 0);
            self->field_30 = (flags >> 4) & 31;
            std::memcpy(&self->field_8, data, 16);
        }
        if (!(self->field_48 & 4))
            camera_resources(self, nullptr);
    }
}
void __fastcall component_destroy(panel_component_base *self, void *, bool release)
{
    if (release)
        mem_dealloc(self, sizeof(panel_component_base));
}
void __fastcall component_unused(panel_component_base *, void *, void *) {}
void __fastcall component_unused_resources(panel_component_base *, void *) {}
void __fastcall base_render(panel_component_base *self, void *, panel_component::render_info *info)
{
    self->_render(*info);
}
void __fastcall base_capture(panel_component_base *self, void *, panel_component::render_info *info)
{
    info->field_138 *= self->field_8;
}
std::intptr_t base_table()
{
    static void *table[]{reinterpret_cast<void *>(component_destroy),
                         reinterpret_cast<void *>(component_unused),
                         reinterpret_cast<void *>(component_unused),
                         reinterpret_cast<void *>(base_pose),
                         reinterpret_cast<void *>(component_unused),
                         reinterpret_cast<void *>(base_render),
                         reinterpret_cast<void *>(base_capture),
                         reinterpret_cast<void *>(component_unused),
                         reinterpret_cast<void *>(component_unused_resources)};
    return reinterpret_cast<std::intptr_t>(table);
}
void __fastcall camera_render(panel_component_camera *self, void *, panel_component::render_info *info)
{
    self->_render(*info);
}
void __fastcall camera_capture(panel_component_camera *self, void *, panel_component::render_info *info)
{
    self->_capture(*info);
}
void __fastcall camera_geometry(panel_component_camera *self, void *, panel_component::render_info *info)
{
    self->setup_geomgr(*info);
}
std::intptr_t camera_table()
{
    static void *table[]{reinterpret_cast<void *>(camera_destroy),
                         reinterpret_cast<void *>(component_unused),
                         reinterpret_cast<void *>(camera_animation),
                         reinterpret_cast<void *>(camera_pose),
                         reinterpret_cast<void *>(component_unused),
                         reinterpret_cast<void *>(camera_render),
                         reinterpret_cast<void *>(camera_capture),
                         reinterpret_cast<void *>(camera_geometry),
                         reinterpret_cast<void *>(camera_resources)};
    return reinterpret_cast<std::intptr_t>(table);
}
void draw_panel_quad(math::MatClass<4, 3> &transform, const aarect<float, vector2d> &rect,
                     const aarect<float, vector2d> &uv, nglTexture *texture, color32 tint, bool opaque, int mode,
                     float depth = 0.0f, nglTexture *secondary = nullptr, unsigned flags = 0)
{
    auto *material = new (nglListAlloc(sizeof(USPanelShaderMaterial), 16)) USPanelShaderMaterial;
    material->texture = texture;
    material->secondary = secondary;
    material->blend = opaque ? NGLBM_OPAQUE : NGLBM_BLEND;
    material->flags = flags ? flags : mode == 1 ? 1 : 194;
    material->mode = mode;
    nglCreateMesh(NGLMESH_TEMP, 1, 0, nullptr);
    auto *definition = nglCreatePCUVVertexDef();
    nglVertexDef_MultipassMesh_Base::AddMeshSection(
        definition, material->material(), 4, 1, 0, nullptr, 24, D3DPT_TRIANGLESTRIP, true);
    auto vertices = definition->CreateIterator();
    vertices.BeginStrip(4);
    const uint32_t packed = color32::to_int(tint);
    vertices.Write({rect.field_0[0][0], rect.field_0[0][1], depth}, packed, {uv.field_0[0][0], uv.field_0[1][1]});
    ++vertices;
    vertices.Write({rect.field_0[0][0], rect.field_0[1][1], depth}, packed, uv.field_0[0]);
    ++vertices;
    vertices.Write({rect.field_0[1][0], rect.field_0[0][1], depth}, packed, uv.field_0[1]);
    ++vertices;
    vertices.Write({rect.field_0[1][0], rect.field_0[1][1], depth}, packed, {uv.field_0[1][0], uv.field_0[0][1]});
    nglListAddMesh(nglCloseMesh(), transform, nullptr, nullptr);
}
template <typename T>
T &component_field(void *self, unsigned offset)
{
    return *reinterpret_cast<T *>(static_cast<char *>(self) + offset);
}
using rectangle = aarect<float, vector2d>;
void render_gutter(void *, panel_component::render_info &);
rectangle intersect_rect(rectangle a, const rectangle &b)
{
    for (int axis = 0; axis < 2; ++axis) {
        a.field_0[0][axis] = std::max(a.field_0[0][axis], b.field_0[0][axis]);
        a.field_0[1][axis] = std::min(a.field_0[1][axis], b.field_0[1][axis]);
    }
    return a;
}
rectangle page_bounds{vector2d{0.0f, 0.0f}, vector2d{640.0f, 480.0f}};
rectangle page_rectangle()
{
    return page_bounds;
}
rectangle project_panel_rectangle(const matrix4x4 &transform, const rectangle &rect)
{
    rectangle result{};
    for (unsigned corner = 0; corner < 2; ++corner) {
        const auto point = transform * vector3d{rect.field_0[corner][0], rect.field_0[corner][1], 0.0f};
        if (nglCurScene) {
            const auto projected = nglProjectPoint(math::VecClass<3, 1>{vector4d{point.x, point.y, point.z, 1.0f}});
            result.field_0[corner] = {projected[0], 480.0f - projected[1]};
        } else {
            const auto projected = geometry_manager::get_xform(geometry_manager::XFORM_WORLD_TO_SCREEN) * point;
            result.field_0[corner] = {projected.x, projected.y};
        }
    }
    return result;
}
rectangle screen_rectangle(const rectangle &pixels)
{
    return {vector2d{pixels.field_0[0][0] / 320.0f - 1.0f, 1.0f - pixels.field_0[1][1] / 240.0f},
            vector2d{pixels.field_0[1][0] / 320.0f - 1.0f, 1.0f - pixels.field_0[0][1] / 240.0f}};
}
struct panel_capture_data {
    nglTexture *texture;
    RECT rect;
};
void capture_panel_callback(void *payload, void *)
{
    const auto &capture = *static_cast<const panel_capture_data *>(payload);
    if (capture.rect.right > capture.rect.left && capture.rect.bottom > capture.rect.top)
        getUSPanelShader().CopyTexture(nglGetBackBufferTex(), capture.texture, capture.rect);
}
rectangle normalize_rect(rectangle rect, const rectangle &bounds)
{
    for (int axis = 0; axis < 2; ++axis) {
        const float scale = 2.0f / (bounds.field_0[1][axis] - bounds.field_0[0][axis]);
        rect.field_0[0][axis] = (rect.field_0[0][axis] - bounds.field_0[0][axis]) * scale - 1.0f;
        rect.field_0[1][axis] = (rect.field_0[1][axis] - bounds.field_0[0][axis]) * scale - 1.0f;
    }
    return rect;
}
template <unsigned Kind>
struct animation_component {
    static constexpr unsigned sizes[]{0x10, 0x2C, 0x1C, 0x38, 0x38, 0x4C, 0x4C, 0x20, 0x20, 0x20, 0x3C, 0x3C};
    template <typename T>
    static T &field(void *self, unsigned offset)
    {
        return component_field<T>(self, offset);
    }
    static void __fastcall resources(void *self, void *)
    {
        if constexpr (Kind == 3 || Kind == 4) {
            for (unsigned slot = 0; slot < 4; ++slot) {
                auto &texture = field<nglTexture *>(self, 24 + slot * 4);
                if (texture)
                    nglReleaseTexture(texture);
                texture = nullptr;
            }
            field<nglTexture *>(self, 40) = nullptr;
        } else if constexpr (Kind == 9) {
            auto &texture = field<nglTexture *>(self, 8);
            if (texture)
                nglReleaseTexture(texture);
            texture = nullptr;
            field<nglTexture *>(self, 12) = nullptr;
            auto &capture = field<nglTexture *>(self, 28);
            if (capture)
                release_panel_texture(capture);
            capture = nullptr;
        }
    }
    static void __fastcall destroy(void *self, void *, bool release)
    {
        resources(self, nullptr);
        if (release)
            mem_dealloc(self, sizes[Kind]);
    }
    static void __fastcall animation(void *self, void *, nalPanel::nalPanelAnim *anim)
    {
        const auto *data = static_cast<const uint32_t *>(component_animation(*anim, Kind));
        if (!data)
            return;
        if constexpr (Kind == 1) {
            field<bool>(self, 41) = (*data & 1) != 0;
        } else if constexpr (Kind == 3 || Kind == 4) {
            const char *name = reinterpret_cast<const char *>(data + 1);
            for (unsigned slot = 0; slot < 4; ++slot) {
                auto &texture = field<nglTexture *>(self, 24 + slot * 4);
                if (texture)
                    nglReleaseTexture(texture);
                texture = nullptr;
                if (slot < data[0]) {
                    if (*name)
                        texture = nglLoadTexture(tlFixedString{name});
                    name += std::strlen(name) + 1;
                }
            }
        } else if constexpr (Kind == 9) {
            auto &texture = field<nglTexture *>(self, 8);
            if (texture)
                nglReleaseTexture(texture);
            const char *name = reinterpret_cast<const char *>(data + 2);
            texture = data[1] && *name ? nglLoadTexture(tlFixedString{name}) : nullptr;
            field<int>(self, 16) = data[0];
        }
    }
    static void __fastcall pose(void *self, void *, nalPanel::nalPanelPose *value)
    {
        const auto *data = static_cast<const uint32_t *>(component_pose(*value, Kind));
        if (!data)
            return;
        if constexpr (Kind == 1) {
            std::memcpy(static_cast<char *>(self) + 8, data, 32);
            field<bool>(self, 40) = (data[8] & 1) != 0;
        } else if constexpr (Kind == 2) {
            std::memcpy(static_cast<char *>(self) + 8, data, 16);
            field<bool>(self, 24) = (data[4] & 1) != 0;
            field<bool>(self, 25) = (data[4] & 2) != 0;
            field<bool>(self, 26) = (data[4] & 4) != 0;
        } else if constexpr (Kind == 3 || Kind == 4) {
            std::memcpy(static_cast<char *>(self) + 44, data + 5, 4);
            auto &texture = field<nglTexture *>(self, 40);
            if (std::equal_to<float>{}(field<float>(self, 44), 0.0f) || !(data[6] & 1))
                texture = nullptr;
            else {
                texture = data[4] < 4 ? field<nglTexture *>(self, 24 + 4 * data[4]) : nullptr;
                std::memcpy(static_cast<char *>(self) + 8, data, 16);
                field<bool>(self, 48) = (data[6] & 2) != 0;
            }
        } else if constexpr (Kind == 7 || Kind == 8) {
            field<bool>(self, 29) = (data[4] & 1) != 0;
            field<bool>(self, 28) = (data[4] & 2) != 0;
            if (data[4] & 2) {
                std::memcpy(static_cast<char *>(self) + 8, data, 16);
                auto &col = field<color>(self, 8);
                const auto clamp = [](float value) {
                    return value >= 0.0f ? (value > 1.0f ? 1.0f : value) : 0.0f;
                };
                col.r = clamp(col.r);
                col.g = clamp(col.g);
                col.b = clamp(col.b);
                col.a = clamp(col.a);
            } else
                field<color>(self, 8) = default_bgcol();
        } else if constexpr (Kind == 9) {
            std::memcpy(static_cast<char *>(self) + 20, data + 1, 4);
            if (std::equal_to<float>{}(field<float>(self, 20), 0.0f) || !(data[3] & 1))
                field<nglTexture *>(self, 12) = nullptr;
            else {
                field<nglTexture *>(self, 12) = data[0] < 1 ? field<nglTexture *>(self, 8) : nullptr;
                field<uint32_t>(self, 24) = data[2];
            }
        } else if constexpr (Kind == 10 || Kind == 11) {
            const unsigned flags = data[7];
            if (!(flags & 1)) {
                field<unsigned>(self, 44) = 0;
                return;
            }
            field<unsigned>(self, 48) = flags & 16 ? 2 : (flags >> 3) & 1;
            field<uint8_t>(self, 56) = ((flags & 2) ? 4 : 0) | ((flags & 4) ? 8 : 0) | ((flags & 32) ? 2 : 0);
            std::memcpy(static_cast<char *>(self) + 24, data + 3, 12);
            field<float>(self, 36) = 1.0f;
            std::memcpy(static_cast<char *>(self) + 8, data, 12);
            field<float>(self, 20) = 1.0f;
            field<uint32_t>(self, 40) = data[6];
            field<unsigned>(self, 44) = (flags >> 6) & 31;
        }
    }
    static void __fastcall render(void *self, void *, panel_component::render_info *info);
    static void __fastcall capture(void *self, void *, panel_component::render_info *info)
    {
        if constexpr (Kind == 1 || Kind == 10 || Kind == 11)
            render(self, nullptr, info);
    }
    static std::intptr_t table()
    {
        static void *value[]{reinterpret_cast<void *>(destroy),
                             reinterpret_cast<void *>(component_unused),
                             reinterpret_cast<void *>(animation),
                             reinterpret_cast<void *>(pose),
                             reinterpret_cast<void *>(component_unused),
                             reinterpret_cast<void *>(render),
                             reinterpret_cast<void *>(capture),
                             Kind == 1 ? reinterpret_cast<void *>(render) : reinterpret_cast<void *>(component_unused),
                             reinterpret_cast<void *>(resources)};
        return reinterpret_cast<std::intptr_t>(value);
    }
    static panel_component_base *create()
    {
        auto *value = static_cast<panel_component_base *>(mem_alloc(sizes[Kind]));
        std::memset(value, 0, sizes[Kind]);
        value->m_vtbl = table();
        if constexpr (Kind == 1 || Kind == 3 || Kind == 4) {
            field<rectangle>(value, 8) = {vector2d{0.0f, 0.0f}, vector2d{1.0f, 1.0f}};
            if constexpr (Kind == 1)
                field<rectangle>(value, 24) = field<rectangle>(value, 8);
            else {
                field<float>(value, 44) = 1.0f;
                field<unsigned>(value, 52) = Kind - 3;
            }
        } else if constexpr (Kind == 7 || Kind == 8) {
            field<color>(value, 8) = {0.0f, 0.0f, 0.0f, 1.0f};
            field<unsigned>(value, 24) = Kind - 7;
            field<bool>(value, 28) = true;
            field<bool>(value, 29) = true;
        } else if constexpr (Kind == 9)
            field<float>(value, 20) = 1.0f;
        else if constexpr (Kind == 10 || Kind == 11) {
            field<float>(value, 20) = 1.0f;
            field<float>(value, 36) = 1.0f;
            field<unsigned>(value, 52) = Kind - 10;
            field<uint8_t>(value, 56) = 4;
        }
        return value;
    }
};
panel_component_base *create_animation_component(unsigned kind)
{
    switch (kind) {
    case 0:
        return new (mem_alloc(sizeof(panel_component_base)))
            panel_component_base{static_cast<int>(base_table()), nullptr, 1.0f, true};
    case 1:
        return animation_component<1>::create();
    case 2:
        return animation_component<2>::create();
    case 3:
        return animation_component<3>::create();
    case 4:
        return animation_component<4>::create();
    case 5:
    case 6: {
        auto *value = new (mem_alloc(sizeof(panel_component_camera))) panel_component_camera{};
        value->m_vtbl = camera_table();
        value->field_8 = {vector2d{0.0f, 0.0f}, vector2d{1.0f, 1.0f}};
        value->field_24 = 1.0f;
        value->field_28 = kind - 5;
        value->field_48 = 24;
        return reinterpret_cast<panel_component_base *>(value);
    }
    case 7:
        return animation_component<7>::create();
    case 8:
        return animation_component<8>::create();
    case 9:
        return animation_component<9>::create();
    case 10:
        return animation_component<10>::create();
    case 11:
        return animation_component<11>::create();
    default:
        return nullptr;
    }
}
nalAnimClass<nalAnyPose>::nalInstanceClass *__fastcall panel_create(panel *self, void *, nalAnimClass<nalAnyPose> *anim)
{
    self->init_anim(reinterpret_cast<nalPanel::nalPanelAnim *>(anim));
    return anim->VirtualCreateInstance(anim->Skeleton);
}
void __fastcall panel_advance(panel *self, void *, nalAnimClass<nalAnyPose>::nalInstanceClass *instance, Float time,
                              Float previous, Float total, Float previous_total)
{
    if (std::equal_to<float>{}(total, previous_total))
        return;
    auto *anim = reinterpret_cast<nalPanel::nalPanelAnim *>(instance->field_10);
    const unsigned frame = static_cast<unsigned>(anim->frame_count * double(time.value));
    int last;
    std::memcpy(&last, &self->field_58, sizeof(last));
    if (frame > static_cast<unsigned>(last) && frame < anim->frame_count) {
        self->field_50 = 0.0f;
        if (!self->field_5C) {
            self->field_5C = true;
            for (auto *component = self->field_60; component; component = component->field_4) {
                auto release = reinterpret_cast<void(__fastcall *)(void *, void *)>(get_vfunc(component->m_vtbl, 32));
                release(component, nullptr);
            }
        }
        return;
    }
    if (frame < static_cast<unsigned>(self->field_54)) {
        self->field_50 = 0.0f;
        return;
    }
    auto *skeleton = static_cast<nalPanel::nalPanelSkeleton *>(instance->field_C);
    nalPanel::nalPanelPose pose{skeleton};
    instance->VirtualGetPose(time,
                             previous,
                             *reinterpret_cast<nalBasePose *>(&pose.field_4),
                             *reinterpret_cast<const nalBasePose *>(&skeleton->m_theDefaultPose->field_4));
    for (auto *component = self->field_60; component; component = component->field_4) {
        auto apply = reinterpret_cast<void(__fastcall *)(void *, void *, nalPanel::nalPanelPose *)>(
            get_vfunc(component->m_vtbl, 12));
        apply(component, nullptr, &pose);
    }
    auto *data = static_cast<const float *>(component_pose(pose, 0));
    quaternion rotation{-data[3], data[0], data[1], data[2]};
    rotation.to_matrix(self->field_4);
    self->field_4[3] = {data[4], data[5], data[6], 1.0f};
    self->m_size = {data[7], data[8]};
    self->field_50 = data[9];
    pose.FreePoseData();
}
void __fastcall panel_scene_render(panel *, void *, nalAnimClass<nalAnyPose>::nalInstanceClass *, Float) {}
void __fastcall panel_scene_release(panel *, void *) {}
std::intptr_t panel_table()
{
    static nalClientSceneAnim::vtable table{reinterpret_cast<decltype(table.CreateInstance)>(panel_create),
                                            reinterpret_cast<decltype(table.Advance)>(panel_advance),
                                            reinterpret_cast<decltype(table.Render)>(panel_scene_render),
                                            reinterpret_cast<decltype(table.Release)>(panel_scene_release)};
    return reinterpret_cast<std::intptr_t>(&table);
}
void clear_depth_rect(rectangle rect, math::MatClass<4, 3> &transform)
{
    auto *material = new (nglListAlloc(sizeof(USPanelShaderMaterial), 16)) USPanelShaderMaterial;
    material->texture = nglWhiteTex;
    material->blend = NGLBM_OPAQUE;
    material->flags = 1;
    nglCreateMesh(NGLMESH_TEMP, 1, 0, nullptr);
    auto *definition = nglCreatePCUVVertexDef();
    nglVertexDef_MultipassMesh_Base::AddMeshSection(
        definition, material->material(), 4, 1, 0, nullptr, 24, D3DPT_TRIANGLESTRIP, true);
    auto writer = definition->CreateIterator();
    writer.BeginStrip(4);
    const auto &matrix = reinterpret_cast<const matrix4x4 &>(transform);
    for (unsigned corner = 0; corner < 4; ++corner) {
        const auto point = matrix * vector3d{rect.field_0[corner & 1][0], rect.field_0[corner >> 1][1], 0.0f};
        math::VecClass<3, 1> projected;
        nglProjectPoint(projected, math::VecClass<3, 1>{point[0], point[1], point[2], 1.0f});
        writer.Write({projected[0], projected[1], 1.0f}, 0xFFFFFFFFu, {0.0f, 0.0f});
        ++writer;
    }
    auto *mesh = nglCloseMesh();
    nglListBeginScene(static_cast<nglSceneParamType>(1));
    nglSetZTestEnable(false);
    nglSetZWriteEnable(true);
    nglSetFBWriteMask(0);
    matrix4x4 view{identity_matrix};
    view[0][0] = 2.0f / 640.0f;
    view[1][1] = 2.0f / 480.0f;
    view[3] = {-1.0f, -1.0f, 0.0f, 1.0f};
    nglSetWorldToViewMatrix({view});
    nglSetAspectRatio(1.0f);
    nglSetOrthoMatrix(0.0f, 1.0f);
    nglSetView(-1.0f, -1.0f, 1.0f, 1.0f);
    nglSetScissor(-1.0f, -1.0f, 1.0f, 1.0f);
    if (g_game_ptr->get_cur_state() == game_state::TAKE_SCREENSHOT ||
        g_game_ptr->get_cur_state() == game_state::SAVE_SCREENSHOT)
        nglAdjustViewForHiresScreenshot();
    nglCalculateMatrices(false);
    math::MatClass<4, 3> identity{identity_matrix};
    nglListAddMesh(mesh, identity, nullptr, nullptr);
    nglListEndScene();
}

USPanelShaderMaterial gutter_materials[4];
float page_near = 0.0f;
float page_far = 10000.0f;
bool page_orthographic = true;
color page_background{0.57f, 0.57f, 0.57f, 1.0f};

nglMesh *gutter_mesh(const vector4d *vertices, unsigned sections, const unsigned *materials, uint32_t tint, float depth)
{
    nglCreateMesh(NGLMESH_TEMP, sections, 0, nullptr);
    for (unsigned section = 0; section < sections; ++section) {
        auto *definition = nglCreatePCUVVertexDef();
        nglVertexDef_MultipassMesh_Base::AddMeshSection(definition,
                                                        gutter_materials[materials[section]].material(),
                                                        16,
                                                        4,
                                                        0,
                                                        nullptr,
                                                        24,
                                                        D3DPT_TRIANGLESTRIP,
                                                        true);
        auto writer = definition->CreateIterator();
        for (unsigned quad = 0; quad < 4; ++quad) {
            writer.BeginStrip(4);
            for (unsigned corner = 0; corner < 4; ++corner) {
                const auto &vertex = vertices[section * 16 + quad * 4 + corner];
                writer.Write({vertex[0], vertex[1], depth}, tint, {vertex[2], vertex[3]});
                ++writer;
            }
        }
    }
    return nglCloseMesh();
}

nglMesh *gutter_fill_mesh(const rectangle &area, const rectangle &offset, unsigned material, uint32_t tint, float depth)
{
    const float x0 = area.field_0[0][0], y0 = area.field_0[0][1];
    const float x1 = area.field_0[1][0], y1 = area.field_0[1][1];
    const float left = x0 + offset.field_0[0][0], right = x1 + offset.field_0[1][0];
    const float bottom = y0 - offset.field_0[1][1], top = y1 - offset.field_0[0][1];
    const vector4d vertices[]{{left, bottom, .5f, .5f},
                              {x0, y0, .5f, .5f},
                              {right, bottom, .5f, .5f},
                              {x1, y0, .5f, .5f},
                              {x0, y1, .5f, .5f},
                              {left, top, .5f, .5f},
                              {x1, y1, .5f, .5f},
                              {right, top, .5f, .5f},
                              {x1, y0, .5f, .5f},
                              {x1, y1, .5f, .5f},
                              {right, bottom, .5f, .5f},
                              {right, top, .5f, .5f},
                              {left, bottom, .5f, .5f},
                              {left, top, .5f, .5f},
                              {x0, y0, .5f, .5f},
                              {x0, y1, .5f, .5f}};
    return gutter_mesh(vertices, 1, &material, tint, depth);
}

nglMesh *gutter_edge_mesh(const rectangle &area, uint32_t tint, float depth)
{
    const float x0 = area.field_0[0][0], y0 = area.field_0[0][1];
    const float x1 = area.field_0[1][0], y1 = area.field_0[1][1];
    const float left = x0 - 2.0f, right = x1 + 2.0f, bottom = y0 - 2.0f, top = y1 + 2.0f;
    const vector4d vertices[]{{x0, bottom, 0, .75f},
                              {x0, y0, 0, 1},
                              {x1, bottom, 1, .75f},
                              {x1, y0, 1, 1},
                              {x0, y1, 0, 1},
                              {x0, top, 0, .75f},
                              {x1, y1, 1, 1},
                              {x1, top, 1, .75f},
                              {x1, y0, 1, 1},
                              {x1, y1, 0, 1},
                              {right, y0, 1, .75f},
                              {right, y1, 0, .75f},
                              {left, y0, 0, .75f},
                              {left, y1, 1, .75f},
                              {x0, y0, 0, 1},
                              {x0, y1, 1, 1},
                              {left, bottom, .75f, .75f},
                              {left, y0, .75f, 1},
                              {x0, bottom, 1, .75f},
                              {x0, y0, 1, 1},
                              {x1, y1, 1, 1},
                              {x1, top, .75f, 1},
                              {right, y1, 1, .75f},
                              {right, top, .75f, .75f},
                              {left, y1, .75f, 1},
                              {left, top, .75f, .75f},
                              {x0, y1, 1, 1},
                              {x0, top, 1, .75f},
                              {x1, bottom, 1, .75f},
                              {x1, y0, 1, 1},
                              {right, bottom, .75f, .75f},
                              {right, y0, .75f, 1}};
    const unsigned materials[]{2, 3};
    return gutter_mesh(vertices, 2, materials, tint, depth);
}

void render_gutter(void *self, panel_component::render_info &info)
{
    const auto offset = component_field<rectangle>(self, 8);
    const bool empty = offset.field_0[1][0] <= offset.field_0[0][0] || offset.field_0[1][1] <= offset.field_0[0][1];
    const uint32_t tint = 0x969696u | (static_cast<uint32_t>(info.field_138 * 255.0f) << 24);
    float depth = 0.0f;
    if (page_orthographic && info.field_144) {
        const auto view = nglGetMatrix(NGLMTX_WORLD_TO_VIEW);
        if (std::equal_to<float>{}(view[2][3], 0.0f)) {
            const auto position = view * vector3d{info.field_0[3][0], info.field_0[3][1], info.field_0[3][2]};
            depth = position[2] - ((page_far - page_near) * .001f + page_near);
        }
    }
    auto draw = [&](nglMesh *mesh, bool test, bool write, bool mask) {
        nglListBeginScene(static_cast<nglSceneParamType>(1));
        nglSetZTestEnable(test);
        nglSetZWriteEnable(write);
        if (mask)
            nglSetFBWriteMask(0);
        nglListAddMesh(mesh, info.field_0, nullptr, nullptr);
        nglListEndScene();
    };
    if (component_field<bool>(self, 25) && !empty) {
        draw(gutter_fill_mesh(info.field_114, offset, 1, tint, depth - .1f), false, true, false);
        draw(gutter_fill_mesh(info.field_114, offset, 0, tint, depth - .1f), false, true, true);
    }
    if (component_field<bool>(self, 24))
        draw(gutter_edge_mesh(info.field_114, tint, depth), false, false, false);
    if (component_field<bool>(self, 26) && !empty) {
        auto shadow = info.field_114;
        for (unsigned corner = 0; corner < 2; ++corner)
            for (unsigned axis = 0; axis < 2; ++axis)
                shadow.field_0[corner][axis] += offset.field_0[corner][axis];
        draw(gutter_edge_mesh(shadow, tint, 0.0f), true, false, false);
    }
}
template <unsigned Kind>
void __fastcall animation_component<Kind>::render(void *self, void *, panel_component::render_info *info)
{
    if constexpr (Kind == 1) {
        auto area = field<rectangle>(self, 8);
        if (!field<bool>(self, 40)) {
            const auto bounds = project_panel_rectangle(info->field_0, field<rectangle>(self, 24));
            for (int axis = 0; axis < 2; ++axis)
                for (int corner = 0; corner < 2; ++corner)
                    area.field_0[corner][axis] =
                        bounds.field_0[0][axis] +
                        area.field_0[corner][axis] * (bounds.field_0[1][axis] - bounds.field_0[0][axis]);
            area = normalize_rect(area, {vector2d{0.0f, 0.0f}, vector2d{640.0f, 480.0f}});
            const float low = area.field_0[0][1];
            area.field_0[0][1] = -area.field_0[1][1];
            area.field_0[1][1] = -low;
        } else
            area = normalize_rect(area, page_rectangle());
        info->field_124 = intersect_rect(area, {vector2d{-1.0f, -1.0f}, vector2d{1.0f, 1.0f}});
        if (info->field_124.field_0[1][0] - info->field_124.field_0[0][0] < 4.0f / 640.0f ||
            info->field_124.field_0[1][1] - info->field_124.field_0[0][1] < 4.0f / 480.0f)
            info->field_142 = true;
        else
            nglSetScissor(info->field_124.field_0[0][0],
                          info->field_124.field_0[0][1],
                          info->field_124.field_0[1][0],
                          info->field_124.field_0[1][1]);
        info->field_140 = field<bool>(self, 41);
        info->field_141 = !info->field_140;
    } else if constexpr (Kind == 3 || Kind == 4) {
        auto *texture = field<nglTexture *>(self, 40);
        const float opacity = info->field_138 * field<float>(self, 44);
        if (!texture || opacity <= 0.0f)
            return;
        auto uv = field<rectangle>(self, 8);
        if (field<bool>(self, 48)) {
            const auto &rect = info->field_114;
            for (int axis = 0; axis < 2; ++axis)
                for (int corner = 0; corner < 2; ++corner)
                    uv.field_0[corner][axis] = (rect.field_0[corner][axis] - uv.field_0[0][axis]) /
                                               (uv.field_0[1][axis] - uv.field_0[0][axis]);
        }
        const float low = uv.field_0[0][1];
        uv.field_0[0][1] = 1.0f - uv.field_0[1][1];
        uv.field_0[1][1] = 1.0f - low;
        nglListBeginScene(static_cast<nglSceneParamType>(1));
        nglSetZTestEnable(false);
        nglSetZWriteEnable(true);
        draw_panel_quad(info->field_0,
                        info->field_114,
                        uv,
                        texture,
                        {255, 255, 255, static_cast<uint8_t>(opacity * 255.0f)},
                        opacity >= 1.0f,
                        0);
        nglListEndScene();
    } else if constexpr (Kind == 7 || Kind == 8) {
        if (field<bool>(self, 29))
            clear_depth_rect(info->field_114, info->field_0);
        if (field<bool>(self, 28)) {
            auto tint = field<color>(self, 8);
            tint.a *= info->field_138;
            comic_panels::clear_color_rect(info->field_114, tint, info->field_0);
        }
    } else if constexpr (Kind == 10 || Kind == 11) {
        for (unsigned index = 0; index < 5; ++index) {
            if (!(field<unsigned>(self, 44) & (1u << index)))
                continue;
            auto &params = info->field_40.field_8[index];
            params.field_24 = 0;
            if (field<unsigned>(self, 48) == 1) {
                params.field_24 = 4;
                params.field_0 = field<vector4d>(self, 24);
            } else if (field<unsigned>(self, 48) == 2)
                params.field_24 = 8;
            const auto flags = field<uint8_t>(self, 56);
            if (flags & 8) {
                params.field_24 |= 2;
                params.field_10 = field<vector4d>(self, 8);
                params.field_20 = field<int>(self, 40);
            }
            if (flags & 4)
                params.field_24 |= 1;
            if (flags & 2)
                params.field_24 |= 16;
        }
    } else if constexpr (Kind == 9) {
        const unsigned mode = field<unsigned>(self, 16);
        auto *selected = field<nglTexture *>(self, 12);
        if (!mode || !selected)
            return;
        const auto area = intersect_rect(project_panel_rectangle(info->field_0, info->field_114),
                                         {vector2d{0.0f, 0.0f}, vector2d{640.0f, 480.0f}});
        const float width = area.field_0[1][0] - area.field_0[0][0];
        const float height = area.field_0[1][1] - area.field_0[0][1];
        auto &capture = field<nglTexture *>(self, 28);
        if (capture && (capture->m_width < width || capture->m_height < height)) {
            release_panel_texture(capture);
            capture = nullptr;
        }
        if (!capture)
            capture = create_panel_texture((static_cast<unsigned>(width) + 15) & ~15u,
                                           (static_cast<unsigned>(height) + 3) & ~3u);
        if (!capture)
            return;
        nglListBeginScene(static_cast<nglSceneParamType>(1));
        nglSetZTestEnable(false);
        nglSetZWriteEnable(false);
        nglSetRenderTarget(capture);
        nglSetView(-1.0f, -1.0f, 1.0f, 1.0f);
        nglSetScissor(-1.0f, -1.0f, 1.0f, 1.0f);
        if (g_game_ptr->get_cur_state() == game_state::TAKE_SCREENSHOT ||
            g_game_ptr->get_cur_state() == game_state::SAVE_SCREENSHOT)
            nglAdjustViewForHiresScreenshot();
        matrix4x4 view{identity_matrix};
        view[0][0] = 2.0f / width;
        view[1][1] = 2.0f / height;
        view[3] = {-1.0f, -1.0f, 0.0f, 1.0f};
        nglSetWorldToViewMatrix({view});
        nglSetAspectRatio(1.0f);
        nglSetOrthoMatrix(0.0f, 1.0f);
        nglCalculateMatrices(false);
        math::MatClass<4, 3> identity{identity_matrix};
        auto uv = area;
        for (unsigned corner = 0; corner < 2; ++corner) {
            uv.field_0[corner][0] /= 640.0f;
            uv.field_0[corner][1] = 1.0f - uv.field_0[corner][1] / 480.0f;
        }
        std::swap(uv.field_0[0][1], uv.field_0[1][1]);
        draw_panel_quad(identity,
                        {vector2d{0.0f, 0.0f}, vector2d{float(width), float(height)}},
                        uv,
                        nglGetBackBufferTex(),
                        {255, 255, 255, 255},
                        true,
                        5,
                        0.0f,
                        nglWhiteTex,
                        193);
        nglListEndScene();
        nglListBeginScene(static_cast<nglSceneParamType>(1));
        nglSetZTestEnable(false);
        nglSetZWriteEnable(false);
        nglSetRenderTarget(nglGetBackBufferTex());
        draw_panel_quad(info->field_0,
                        info->field_114,
                        {vector2d{0.0f, 0.0f}, vector2d{1.0f, 1.0f}},
                        capture,
                        {255, 255, 255, static_cast<uint8_t>(field<float>(self, 20) * 255.0f)},
                        true,
                        mode == 1 ? 6 : mode,
                        0.0f,
                        selected,
                        193);
        nglListEndScene();
        if (mode == 1) {
            nglListBeginScene(static_cast<nglSceneParamType>(1));
            nglSetZTestEnable(false);
            nglSetZWriteEnable(false);
            nglSetRenderTarget(nglGetBackBufferTex());
            auto overlay = info->field_114;
            const auto projected = project_panel_rectangle(info->field_0, info->field_114);
            overlay.field_0[1][0] = (projected.field_0[1][0] - projected.field_0[0][0]) * 0.015625f;
            overlay.field_0[1][1] = (projected.field_0[1][1] - projected.field_0[0][1]) * 0.015625f;
            draw_panel_quad(info->field_0, info->field_114, overlay, capture, {255, 255, 255, 255}, true, 1);
            nglListEndScene();
        }
    } else if constexpr (Kind == 2) {
        render_gutter(self, *info);
    }
}
}


void clear_color_rect(aarect<float, vector2d> &a1, color &a2, math::MatClass<4, 3> &a3)
{
    if (a2.a <= 0.0f)
        return;
    nglListBeginScene(static_cast<nglSceneParamType>(1));
    nglSetZTestEnable(false);
    nglSetZWriteEnable(false);
    const color32 tint{static_cast<uint8_t>(a2.r * 255.0f),
                       static_cast<uint8_t>(a2.g * 255.0f),
                       static_cast<uint8_t>(a2.b * 255.0f),
                       static_cast<uint8_t>(a2.a * 255.0f)};
    draw_panel_quad(a3, a1, {vector2d{0.0f, 0.0f}, vector2d{1.0f, 1.0f}}, nglWhiteTex, tint, false, 1);
    nglListEndScene();
}

void draw_textured_quad(math::MatClass<4, 3> &a1, aarect<float, vector2d> &a2, aarect<float, vector2d> &a3,
                        nglTexture *a4, color32 a5, bool a6)
{
    draw_panel_quad(a1, a2, a3, a4, a5, a6, 0);
}

void sub_735C80(void *, aarect<float, vector2d> &a1, void *)
{
    a1.field_0[0][0] = 0.0;
    a1.field_0[0][1] = 0.0;
    a1.field_0[1][0] = 1.0;
    a1.field_0[1][1] = 1.0;
}

aarect<float, vector2d> sub_742E00(aarect<float, vector2d> rectangle, const vector2d &offset)
{
    rectangle += offset;
    return rectangle;
}

aarect<float, vector2d> sub_744B00()
{
    aarect<float, vector2d> result{vector2d{-1.0f, -1.0f}, vector2d{1.0f, 1.0f}};
    return result;
}

void set_default_bgcolor(const color &a1)
{
    default_bgcol() = a1;
}

void sub_7315A0()
{
    gutter_materials[0].blend = NGLBM_OPAQUE;
    gutter_materials[0].flags = 194;
    gutter_materials[0].texture = nglWhiteTex;
    gutter_materials[1].blend = NGLBM_BLEND;
    gutter_materials[1].flags = 194;
    gutter_materials[1].texture = nglWhiteTex;
    gutter_materials[2].blend = NGLBM_BLEND;
    gutter_materials[2].flags = 130;
    gutter_materials[2].texture = nglGetTexture(tlFixedString{"us_i_gutter"});
    gutter_materials[3].blend = NGLBM_BLEND;
    gutter_materials[3].flags = 194;
    gutter_materials[3].texture = nglGetTexture(tlFixedString{"us_i_gutter_corn"});
}

void sub_742B20(void *, bool a2, bool a3)
{
    nglListBeginScene(static_cast<nglSceneParamType>(1));
    nglSetZTestEnable(a2);
    nglSetZWriteEnable(a3);
}

void init()
{
    TRACE("comic_panels::init");

    panels() = {};
    current_view_camera() = nullptr;
    world_has_been_rendered = false;
    sub_7315A0();
    auto &cameras = var<fixed_vector<marky_camera *, 10>>(0x0096FAD4);
    cameras = {};
    for (unsigned index = 0; index < 10; ++index) {
        const mString name = mString{"CAMERA0"} + mString{static_cast<int>(index)};
        cameras.push_back(new marky_camera{string_hash{name.c_str()}});
    }
    const char *names[]{"GLAMCAM0", "GLAMCAM1", "GLAMCAM2", "GLAMCAM3"};
    for (unsigned index = 0; index < 4; ++index)
        glamour_cams[index] = new glam_camera{names[index]};
    game_play_panel() = acquire_panel("Game");
    game_play_panel()->add_camera_component(nullptr, false, 255);
    game_play_panel()->field_65 = true;
    for (auto *component = game_play_panel()->field_60; component; component = component->field_4)
        if (component->m_vtbl == base_table())
            component->field_C = false;
}

void clear_projected_shadows()
{
    for (auto &shadow : g_shadow())
        shadow.field_48 = false;
    for (auto &candidate : shadow_candidates()) {
        candidate.field_18 = nullptr;
        candidate.field_28 = 0.0f;
    }
    g_cur_shadow_target = 0;
    g_shadow_scene = nullptr;
}

void setup_main_scene()
{
    nglListBeginScene(static_cast<nglSceneParamType>(1));
    nglSetZTestEnable(true);
    nglSetZWriteEnable(true);
    nglSetClearFlags(0);
    nglCurScene->AnimTime = g_world_ptr->time_manager.get_level_time();
    if (auto *camera = cur_page_camera(); camera && !var<bool>(0x0096F7C1)) {
        page_near = camera->field_CC;
        page_far = camera->field_D0;
        page_orthographic = camera->field_EC;
        page_background = {camera->field_DC, camera->field_E0, camera->field_E4, 1.0f};
        page_bounds = {vector2d{camera->field_3C[0], camera->field_3C[1]},
                       vector2d{camera->field_3C[2], camera->field_3C[3]}};
        if (page_orthographic) {
            const auto size = camera->ortho_size();
            matrix4x4 scale{identity_matrix};
            scale[0][0] = 1.0f / size.x;
            scale[1][1] = 1.0f / size.y;
            nglSetWorldToViewMatrix({camera->field_4C * scale});
            nglSetAspectRatio(1.0f);
            nglSetOrthoMatrix(page_near, page_far);
        } else {
            nglSetWorldToViewMatrix({camera->field_4C});
            nglSetAspectRatio(camera->field_D8);
            const float vertical_fov = 2.0f * std::atan2(std::tan(camera->field_D4 * .5f) / camera->field_D8, 1.0f);
            nglSetPerspectiveMatrix(vertical_fov * (180.0f / PI), page_near, page_far);
            nglCalculateMatrices(false);
        }
    } else {
        page_near = 0.0f;
        page_far = 10000.0f;
        page_orthographic = true;
        page_background = default_bgcol();
        matrix4x4 view{identity_matrix};
        const auto bounds = page_rectangle();
        view[0][0] = 2.0f / (bounds.field_0[1][0] - bounds.field_0[0][0]);
        view[1][1] = 2.0f / (bounds.field_0[1][1] - bounds.field_0[0][1]);
        view[3] = {-1.0f - view[0][0] * bounds.field_0[0][0], -1.0f - view[1][1] * bounds.field_0[0][1], 0.0f, 1.0f};
        nglSetWorldToViewMatrix({view});
        nglSetOrthoMatrix(page_near, page_far);
        nglSetAspectRatio(1.0f);
        nglCalculateMatrices(false);
        page_bounds = {vector2d{0.0f, 0.0f}, vector2d{640.0f, 480.0f}};
    }
}

void render()
{
    if (var<bool>(0x0096F7C0)) {
        game::render_empty_list();
        return;
    }
    clear_projected_shadows();
    nglListInit();
    if (var<bool>(0x0095C878))
        nglSetClearFlags(0);
    nglListBeginScene(static_cast<nglSceneParamType>(1));
    nglSetZTestEnable(true);
    nglSetZWriteEnable(true);
    nglSetClearFlags(0);
    nglListEndScene();
    setup_main_scene();
    render_panels();
    nglListEndScene();
    nglListSend(true);
    IDirect3DDevice9_SetStreamSource(g_Direct3DDevice, 0, nullptr, 0, 0);
    for (unsigned stage = 0; stage < 4; ++stage)
        IDirect3DDevice9_SetTexture(g_Direct3DDevice, stage, nullptr);
    g_renderTextureState().clear();
}


void frame_advance(Float a1)
{
    TRACE("comic_panels::frame_advance");

    if (cur_page_camera())
        cur_page_camera()->advance(a1);
    for (auto *camera : glamour_cams)
        if (camera)
            camera->frame_advance(a1);
}

bool render_panels()
{
    TRACE("comic_panels::render_panels");

    std::sort(panels().m_data, panels().m_data + panels().m_size, [](const panel *left, const panel *right) {
        if (!left)
            return false;
        if (!right)
            return true;
        matrix4x4 view{identity_matrix};
        if (cur_page_camera())
            view = cur_page_camera()->field_4C;
        else
            view[2][2] = -1.0f;
        const float left_depth = (view * left->get_loc())[2];
        const float right_depth = (view * right->get_loc())[2];
        return std::equal_to<float>{}(left_depth, right_depth) ? right < left : left_depth > right_depth;
    });
    world_has_been_rendered = false;
    var<bool>(0x0096F7B8) = false;
    nglListBeginScene(static_cast<nglSceneParamType>(1));
    nglSetView(-1.0f, -1.0f, 1.0f, 1.0f);
    nglSetScissor(-1.0f, -1.0f, 1.0f, 1.0f);
    if (g_game_ptr->get_cur_state() == game_state::TAKE_SCREENSHOT ||
        g_game_ptr->get_cur_state() == game_state::SAVE_SCREENSHOT)
        nglAdjustViewForHiresScreenshot();
    for (auto *value : panels())
        value->capture();
    nglListBeginScene(static_cast<nglSceneParamType>(1));
    nglSetZTestEnable(false);
    nglSetZWriteEnable(true);
    nglSetClearFlags(7);
    nglSetClearColor(page_background.r, page_background.g, page_background.b, page_background.a);
    nglSetView(-1.0f, -1.0f, 1.0f, 1.0f);
    nglSetScissor(-1.0f, -1.0f, 1.0f, 1.0f);
    if (g_game_ptr->get_cur_state() == game_state::TAKE_SCREENSHOT ||
        g_game_ptr->get_cur_state() == game_state::SAVE_SCREENSHOT)
        nglAdjustViewForHiresScreenshot();
    nglCalculateMatrices(false);
    nglListEndScene();
    bool rendered = false;
    for (auto *value : panels())
        if (!value->field_68)
            rendered |= value->render();
    const bool saved_orthographic = page_orthographic;
    const auto saved_bounds = page_bounds;
    page_orthographic = true;
    page_bounds = {vector2d{0.0f, 0.0f}, vector2d{640.0f, 480.0f}};
    if (auto *panel = game_play_panel()) {
        nglListBeginScene(static_cast<nglSceneParamType>(1));
        nglSetZTestEnable(false);
        nglSetZWriteEnable(false);
        matrix4x4 view{identity_matrix};
        view[0][0] = 2.0f / 640.0f;
        view[1][1] = 2.0f / 480.0f;
        view[3] = {-1.0f, -1.0f, 0.0f, 1.0f};
        nglSetWorldToViewMatrix({view});
        nglSetOrthoMatrix(0.0f, 1.0f);
        nglSetAspectRatio(1.0f);
        panel_component::render_info info{*panel, 2};
        for (auto *component = panel->field_60; component; component = component->field_4)
            if (!info.field_142)
                reinterpret_cast<panel_component *>(component)->setup_geomgr(info);
        nglListEndScene();
    }
    g_game_ptr->render_ui();
    page_orthographic = saved_orthographic;
    page_bounds = saved_bounds;
    for (auto *value : panels())
        if (value->field_68)
            rendered |= value->render();
    nglListEndScene();
    return rendered;
}

panel *acquire_panel(const char *)
{
    auto *value = new (mem_alloc(sizeof(panel))) panel{};
    value->m_vtbl = panel_table();
    value->field_4 = identity_matrix;
    value->field_4[3] = {320.0f, 240.0f, -500.0f, 1.0f};
    value->m_size = {640.0f, 480.0f};
    value->field_50 = 1.0f;
    value->field_58 = 2.0f;
    value->field_66 = true;
    value->field_60 = new (mem_alloc(sizeof(panel_component_base)))
        panel_component_base{static_cast<int>(base_table()), nullptr, 1.0f, true};
    panels().push_back(value);
    return value;
}

panel_params_t *get_panel_params()
{
    if (nglCurScene == nullptr || !nglCurScene->field_404.IsSetParam<SMPanelParams>())
        return nullptr;
    return nglCurScene->field_404.Get<SMPanelParams>()->field_0;
}

camera *get_current_view_camera(int)
{
    return current_view_camera();
}

void panel::init_anim(nalPanel::nalPanelAnim *a2)
{
    if (!field_64) {
        field_64 = true;
        while (field_60) {
            auto *component = field_60;
            field_60 = component->field_4;
            auto destroy = reinterpret_cast<void(__fastcall *)(void *, void *, bool)>(get_vfunc(component->m_vtbl, 0));
            destroy(component, nullptr, true);
        }
        panel_component_base *components[12]{};
        unsigned count = 0;
        for (unsigned kind = 0; kind < static_cast<unsigned>(std::min(a2->field_30->GetNumComponents(), 12)); ++kind)
            if (a2->field_40[kind] && count < a2->component_count)
                components[count++] = create_animation_component(kind);
        const auto *order = static_cast<const uint32_t *>(a2->_GetPerAnimUserDataInt());
        auto **tail = &field_60;
        for (unsigned index = 0; index < a2->component_count; ++index) {
            *tail = components[order[index]];
            tail = &(*tail)->field_4;
            *tail = nullptr;
        }
        for (auto *component = field_60; component; component = component->field_4) {
            auto setup = reinterpret_cast<void(__fastcall *)(void *, void *, nalPanel::nalPanelAnim *)>(
                get_vfunc(component->m_vtbl, 8));
            setup(component, nullptr, a2);
        }
    }
    if (auto *data = static_cast<const uint32_t *>(component_animation(*a2, 0))) {
        field_54 = data[0];
        std::memcpy(&field_58, data + 1, 4);
    }
}

void panel::add_camera_component(const char *a2, bool a3, int a4)
{
    auto *component = new (mem_alloc(sizeof(panel_component_camera))) panel_component_camera{};
    component->m_vtbl = camera_table();
    component->field_8 = {vector2d{0.0f, 0.0f}, vector2d{1.0f, 1.0f}};
    component->field_20 = a2 == nullptr || a3;
    component->field_24 = 1.0f;
    component->field_30 = a4;
    component->field_48 = 8 * ((a4 & 0x40) != 0) | 16 * ((a4 & 0x20) != 0);
    component->register_camera(0, a2);
    auto **tail = &field_60;
    while (*tail)
        tail = &(*tail)->field_4;
    *tail = reinterpret_cast<panel_component_base *>(component);
}

void panel::set_gutter_rect(const aarect<float, vector2d> &rect)
{
    for (auto *component = field_60; component; component = component->field_4)
        if (component->m_vtbl == animation_component<2>::table())
            component_field<rectangle>(component, 8) = rect;
}

aarect<float, vector2d> panel::get_gutter_rect() const
{
    rectangle result{};
    for (auto *component = field_60; component; component = component->field_4)
        if (component->m_vtbl == animation_component<2>::table())
            result = component_field<rectangle>(component, 8);
    return result;
}

void panel::add_gutter_component(bool border, bool fill, bool shadow)
{
    auto *component = animation_component<2>::create();
    component_field<bool>(component, 24) = border;
    component_field<bool>(component, 25) = fill;
    component_field<bool>(component, 26) = shadow;
    auto **tail = &field_60;
    while (*tail)
        tail = &(*tail)->field_4;
    *tail = component;
}

void panel::add_color_component(const color &value, bool clear_depth)
{
    auto *component = animation_component<7>::create();
    component_field<color>(component, 8) = value;
    component_field<bool>(component, 28) = clear_depth;
    component_field<bool>(component, 29) = false;
    auto **tail = &field_60;
    while (*tail)
        tail = &(*tail)->field_4;
    *tail = component;
}

void panel::add_texture_component(nglTexture *texture)
{
    auto *component = animation_component<3>::create();
    component_field<nglTexture *>(component, 24) = texture;
    component_field<nglTexture *>(component, 40) = texture;
    auto **tail = &field_60;
    while (*tail)
        tail = &(*tail)->field_4;
    *tail = component;
}

void panel::reset_base_opacity()
{
    for (auto *component = field_60; component; component = component->field_4)
        if (component->m_vtbl == base_table())
            component->field_8 = 1.0f;
}

void panel::reset_gameplay_components()
{
    reset_base_opacity();
    for (auto *component = field_60; component; component = component->field_4)
        if (component->m_vtbl == camera_table())
            reinterpret_cast<panel_component_camera *>(component)->field_2C = 0;
}

void panel::capture()
{
    if (!this->field_67 && (this->field_50 > 0.0f || !this->field_5C)) {
        panel_component::render_info v3{*this, 1};
        for (auto *i = this->field_60; i != nullptr; i = i->field_4) {
            i->capture(v3);
        }
    }
}

bool panel::render()
{
    TRACE("comic_panels::panel::render");

    if (field_67 || field_50 <= 0.0f)
        return false;
    bool draws_world = false;
    for (auto *component = field_60; component; component = component->field_4) {
        if (component->m_vtbl != camera_table())
            continue;
        const auto *camera = reinterpret_cast<panel_component_camera *>(component);
        if ((camera->field_48 & 0x10) && !camera->field_44 && static_cast<unsigned>(camera->field_2C) < 4 &&
            camera->field_34[camera->field_2C].get_volatile_ptr())
            draws_world = true;
    }
    if (draws_world &&
        ((world_has_been_rendered && field_4C && field_4C->field_E1 && field_4C->current_cut_scene->field_32) ||
         var<bool>(0x0096F7B8))) {
        nglListEndScene();
        nglListEndScene();
        nglListSend(false);
        var<bool>(0x0096F7B8) = false;
        clear_projected_shadows();
        nglListInit();
        setup_main_scene();
        nglListBeginScene(static_cast<nglSceneParamType>(1));
        nglSetView(-1.0f, -1.0f, 1.0f, 1.0f);
        nglSetScissor(-1.0f, -1.0f, 1.0f, 1.0f);
        if (g_game_ptr->get_cur_state() == game_state::TAKE_SCREENSHOT ||
            g_game_ptr->get_cur_state() == game_state::SAVE_SCREENSHOT)
            nglAdjustViewForHiresScreenshot();
    }
    nglListBeginScene(static_cast<nglSceneParamType>(1));
    nglSetClearFlags(0);
    panel_component::render_info info{*this, 0};
    for (auto *component = field_60; component; component = component->field_4) {
        auto render =
            reinterpret_cast<void(__fastcall *)(panel_component_base *, void *, panel_component::render_info *)>(
                get_vfunc(component->m_vtbl, 0x14));
        if (!info.field_142)
            render(component, nullptr, &info);
        if (info.field_140) {
            info.field_140 = !info.field_141;
            info.field_141 = info.field_140;
            if (!info.field_141) {
                nglSetScissor(-1.0f, -1.0f, 1.0f, 1.0f);
                info.field_142 = false;
            }
        }
    }
    if (draws_world)
        world_has_been_rendered = true;
    nglListEndScene();
    return true;
}

bool sub_731790(const matrix4x4 &a1)
{
    auto sub_F3F4D0 = [](float a1, float a2) -> bool {
        return std::abs(a2 - a1) < 0.050000001;
    };

    return sub_F3F4D0(a1[0][0], 1.0) && sub_F3F4D0(a1[1][1], 1.0) && sub_F3F4D0(a1[2][2], -1.0);
}

bool panel::is_square() const
{
    auto v3 = this->get_transform();
    v3[3] = vector4d{0.0, 0.0, 0.0, 1.0};
    matrix4x4 v1;
    if (cur_page_camera() != nullptr) {
        v1 = cur_page_camera()->get_transform();
    } else {
        auto v5 = identity_matrix;
        v5[2][2] = -1.0;
        v1 = v5;
    }

    auto v4 = v1;
    v4[3] = {0.0, 0.0, 0.0, 1.0};
    v3 = v4 * v3;
    return sub_731790(v3);
}

matrix4x4 panel::get_transform() const
{
    return this->field_4;
}

aarect<float, vector2d> panel::get_rect() const
{
    auto a2 = this->m_size * 0.5;
    auto v2 = a2 * -1.0f;
    aarect<float, vector2d> arg0{v2, a2};
    return arg0;
}

vector3d panel::get_loc() const
{
    vector3d result{this->field_4[3][0], this->field_4[3][1], this->field_4[3][2]};
    return result;
}

void panel::set_size(const vector2d &a2)
{
    this->m_size = a2;
}

void panel::set_loc(const vector3d &a2)
{
    this->field_4[3] = a2;
}

camera *panel_component_camera::get_default_camera() const
{
    auto v1 = this->field_2C;
    if (v1 >= 4) {
        return g_game_ptr->get_current_view_camera(0);
    }

    auto *result = this->field_34[v1].get_volatile_ptr();
    if (result == nullptr) {
        return g_game_ptr->get_current_view_camera(0);
    }

    return result;
}

void panel_component_camera::register_camera(uint32_t a2, const char *a3)
{
    if (a3 != nullptr && a3[0] != '\0') {
        auto *v4 = g_world_ptr->ent_mgr.get_entity(string_hash{a3});
        if (v4 != nullptr) {
            this->field_34[a2].field_0 = v4->get_my_handle();
        } else {
            this->field_34[a2].field_0 = {0};
        }

    } else {
        this->field_34[a2].field_0 = {0};
    }
}

void panel_component::setup_geomgr(panel_component::render_info &a3)
{
    void(__fastcall * func)(void *, void *, render_info *) = CAST(func, get_vfunc(m_vtbl, 0x1C));
    func(this, nullptr, &a3);
}

panel_component::render_info::render_info(comic_panels::panel &owner, int)
{
    field_0 = math::MatClass<4, 3>{owner.field_4};
    field_40 = {};
    field_40.field_0 = 127;
    field_40.field_4 = &owner;
    for (auto &plane : field_40.field_8)
        plane.field_24 = 17;
    field_40.field_D0 = 255;
    field_114 = owner.get_rect();
    field_124 = {vector2d{-1.0f, -1.0f}, vector2d{1.0f, 1.0f}};
    field_134 = owner.field_4[3][2];
    field_138 = 1.0f;
    field_13C = &owner;
    field_140 = true;
    field_141 = field_142 = field_143 = false;
    field_144 = owner.is_square() || (cur_page_camera() && owner.field_65);
}

void panel_component_camera::set_scene_params(panel_component::render_info &a2)
{
    if (nglCurScene != nullptr) {
        auto *v2 = &a2;
        if (a2.field_40.field_4 != game_play_panel() || a2.field_143 || a2.field_13C->field_4C != nullptr) {
            auto *mem = nglListAlloc(sizeof(panel_params_t), 16u);
            auto *v5 = new (mem) panel_params_t{a2.field_40};

            v5->field_0 = 0;
            if ((this->field_48 & 8) != 0) {
                v5->field_0 = 0x40;
            }

            if ((this->field_48 & 0x10) != 0) {
                v5->field_0 |= 0x20u;
            }

            for (int i = 0; i < 5; ++i) {
                if ((this->field_30 & (1 << i)) != 0) {
                    v5->field_0 |= (1 << i);
                }
            }

            v5->field_D1 ^= (v5->field_D1 ^ v2->field_143) & 1;

            SMPanelParams params{v5};
            auto *SceneParams = nglGetSceneParams();
            SceneParams->SetParam(params);
        } else {
            SMPanelParams params{nullptr};
            auto *v3 = nglGetSceneParams();
            v3->SetParam(params);
        }
    }
}

void panel_component_camera::_render(comic_panels::panel_component::render_info &a2)
{
    TRACE("comic_panels::panel_component_camera::render");

    if (static_cast<unsigned>(this->field_2C) < 4u) {
        auto *v3 = &a2;
        if (!a2.field_142) {
            if (!a2.field_114.sub_560880()) {
                const auto v5 = v3->field_138 * this->field_24;
                if (v5 > 0.0f) {
                    if (this->field_44 == nullptr || v3->field_143) {
                        auto v13 = this->field_48;
                        if ((v13 & 8) != 0 && (v13 & 0x10) != 0 && v3->field_40.field_4 != game_play_panel()) {
                            auto *v14 = g_cut_scene_player();
                            if (v14->is_playing()) {
                                sub_742B20(nullptr, false, false);
                                nglSetClearFlags(0);
                                clear_color_rect(v3->field_114, default_bgcol(), v3->field_0);
                                nglListEndScene();
                            }
                        }

                        auto *def_cam = this->get_default_camera();
                        struct {
                            camera *&field_0;
                            camera *field_4;
                        } a2a = {current_view_camera(), current_view_camera()};
                        current_view_camera() = def_cam;

                        int v22 = 3;
                        struct stru {
                            int *field_0;
                            int field_4;

                            stru(bool a2, int *a3, int *a4)
                            {
                                this->field_0 = (a2 ? a3 : nullptr);
                                this->field_4 = *a3;
                                if (a2) {
                                    *a3 = *a4;
                                }
                            }


                            ~stru()
                            {
                                if (this->field_0 != nullptr) {
                                    *this->field_0 = this->field_4;
                                }
                            }
                        } a1{world_has_been_rendered, &g_disable_occlusion_culling, &v22};

                        geometry_manager::get_xform(geometry_manager::xform_t::XFORM_VIEW_TO_PROJECTION);

                        bool v16 = geometry_manager::get_auto_rebuild_view_frame();
                        geometry_manager::set_auto_rebuild_view_frame(false);
                        sub_742B20(nullptr, true, true);
                        geometry_manager::set_aspect_ratio(1.0);
                        this->setup_geomgr(*v3);
                        if (!v3->field_142) {
                            geometry_manager::set_auto_rebuild_view_frame(v16);
                            geometry_manager::rebuild_view_frame();
                            nglSetClearFlags(0);
                            auto &xform = geometry_manager::get_xform(geometry_manager::XFORM_EFFECTIVE_WORLD_TO_VIEW);
                            nglSetWorldToViewMatrix(xform);

                            struct stru {
                                bool *field_0;
                                bool field_4;

                                stru(bool a2, bool *a3, bool *a4)
                                {
                                    this->field_0 = (a2 ? a3 : nullptr);
                                    this->field_4 = *a3;
                                    if (a2) {
                                        *a3 = *a4;
                                    }
                                }

                                ~stru()
                                {
                                    if (this->field_0 != nullptr) {
                                        *this->field_0 = this->field_4;
                                    }
                                }
                            };

                            bool a6 = (this->field_48 & 0x10) == 0;
                            bool v23 = false;
                            stru v28{a6, &g_player_shadows_enabled, &v23};

                            static Var<bool> byte_922C5D{0x00922C5D};
                            bool v24 = false;
                            stru v27{a6, &byte_922C5D(), &v24};
                            g_game_ptr->render_world();
                        }

                        nglListEndScene();
                        geometry_manager::set_auto_rebuild_view_frame(false);
                        auto v18 = sub_744B00();
                        geometry_manager::set_viewport(v18);
                        if (v3->field_141) {
                            geometry_manager::set_scissor(v3->field_124);
                        } else {
                            auto v19 = sub_744B00();
                            geometry_manager::set_scissor(v19);
                        }

                        geometry_manager::set_auto_rebuild_view_frame(v16);

                        a2a.field_0 = a2a.field_4;
                    } else {
                        bool v6 = (static_cast<unsigned>(v5 * 255.0f) == 0xFF && (this->field_48 & 0x20) != 0);

                        sub_742B20(nullptr, (this->field_48 & 1) == 0, (this->field_48 & 1) != 0);

                        vector2d a3{};
                        a3[0] = v3->field_0[3][0];
                        a3[1] = v3->field_0[3][1];

                        aarect<float, vector2d> a2a = v3->field_114;
                        aarect<float, vector2d> a1 = (a2a += a3);
                        sub_735C80(this, a1, nullptr);

                        {
                            auto *v12 = this->field_44;
                            color32 a2{0xFF, 0xFF, 0xFF, 0xFF};
                            draw_textured_quad(v3->field_0, a2a, a1, v12, a2, v6);
                        }

                        nglListEndScene();
                    }
                }
            }
        }
    }
}

void panel_component_camera::_capture(panel_component::render_info &info)
{
    if (static_cast<unsigned>(field_2C) >= 4) {
        camera_resources(this, nullptr);
        return;
    }
    const float opacity = info.field_138 * field_24;
    auto *page = cur_page_camera();
    if (!(field_48 & 4) && (!info.field_144 || opacity < .995f) && opacity > 0.0f && page)
        info.field_143 = true;
    if (!(field_48 & 4) && !info.field_143) {
        camera_resources(this, nullptr);
        return;
    }
    if ((field_48 & 4) && !field_44 && page)
        info.field_143 = true;
    if (!info.field_143)
        return;
    field_48 = (field_48 & ~32) | ((opacity >= .995f && (field_48 & 8)) ? 32 : 0);
    matrix4x4 size_transform = page->field_8C;
    size_transform[3] = reinterpret_cast<const matrix4x4 &>(info.field_0)[3];
    auto pixels = project_panel_rectangle(size_transform, info.field_114);
    for (unsigned axis = 0; axis < 2; ++axis)
        if (pixels.field_0[0][axis] > pixels.field_0[1][axis])
            std::swap(pixels.field_0[0][axis], pixels.field_0[1][axis]);
    vector2d size = pixels.field_0[1] - pixels.field_0[0];
    if (size.x <= 16.0f || size.y <= 16.0f)
        size = {16.0f, 16.0f};
    if (size.x > 640.0f)
        size *= 640.0f / size.x;
    if (size.y > 480.0f)
        size *= 480.0f / size.y;
    if (field_44 && (size.x > field_44->m_width || size.y > field_44->m_height))
        size = {float(field_44->m_width), float(field_44->m_height)};
    auto local = info.field_114;
    local.field_0[1][0] =
        local.field_0[0][0] + (local.field_0[1][0] - local.field_0[0][0]) * nglGetBackBufferTex()->m_width / size.x;
    local.field_0[1][1] =
        local.field_0[0][1] + (local.field_0[1][1] - local.field_0[0][1]) * nglGetBackBufferTex()->m_height / size.y;
    auto world_bounds = local;
    world_bounds += vector2d{info.field_0[3][0], info.field_0[3][1]};
    matrix4x4 panel_view = reinterpret_cast<const matrix4x4 &>(info.field_0);
    for (unsigned axis = 0; axis < 3; ++axis)
        panel_view[3][axis] += panel_view[2][axis] * ((page_far + page_near) * .5f);
    panel_view = panel_view.inverse();
    const auto saved_view = page->field_4C;
    const auto saved_page_bounds = page->field_3C;
    const auto saved_bounds = page_bounds;
    const bool saved_square = info.field_144;
    const bool saved_ortho = page_orthographic;
    const bool saved_page_ortho = page->field_EC;
    const float saved_aspect = page->field_D8;
    const float half_width = (local.field_0[1][0] - local.field_0[0][0]) * .5f;
    const float half_height = (local.field_0[1][1] - local.field_0[0][1]) * .5f;
    page->field_4C = panel_view;
    page->field_3C = {
        world_bounds.field_0[0][0], world_bounds.field_0[0][1], world_bounds.field_0[1][0], world_bounds.field_0[1][1]};
    page_bounds = world_bounds;
    info.field_144 = page_orthographic = page->field_EC = true;
    page->field_D8 = half_width / half_height;
    nglListBeginScene(static_cast<nglSceneParamType>(1));
    nglSetZTestEnable(true);
    nglSetZWriteEnable(true);
    nglSetRenderTarget(nglGetBackBufferTex());
    nglListBeginScene(static_cast<nglSceneParamType>(1));
    nglSetZTestEnable(false);
    nglSetZWriteEnable(true);
    nglSetClearFlags(7);
    nglSetClearColor(page_background.r, page_background.g, page_background.b, 1.0f);
    nglSetView(-1.0f, -1.0f, size.x / 320.0f - 1.0f, size.y / 240.0f - 1.0f);
    nglSetScissor(-1.0f, -1.0f, size.x / 320.0f - 1.0f, size.y / 240.0f - 1.0f);
    nglCalculateMatrices(false);
    nglListEndScene();
    matrix4x4 scale{identity_matrix};
    scale[0][0] = 1.0f / half_width;
    scale[1][1] = 1.0f / half_height;
    scale[3][0] = -(local.field_0[1][0] + local.field_0[0][0]) * .5f / half_width;
    scale[3][1] = (local.field_0[1][1] + local.field_0[0][1]) * .5f / half_height;
    nglListBeginScene(static_cast<nglSceneParamType>(1));
    nglSetWorldToViewMatrix({panel_view * scale});
    nglSetAspectRatio(1.0f);
    nglSetOrthoMatrix(page_near, page_far);
    nglCalculateMatrices(false);
    size.x *= float(nglGetBackBufferTex()->m_width) / nglGetScreenWidth();
    size.y *= float(nglGetBackBufferTex()->m_height) / nglGetScreenHeight();
    _render(info);
    nglListEndScene();
    if (!(field_48 & 32) && !var<bool>(0x0095C878)) {
        var<bool>(0x0095634D) = true;
        nglListBeginScene(static_cast<nglSceneParamType>(1));
        nglSetZTestEnable(true);
        nglSetZWriteEnable(false);
        nglSetClearFlags(0);
        nglSetFBWriteMask(8);
        nglSetWorldToViewMatrix({identity_matrix});
        nglSetAspectRatio(1.0f);
        nglSetOrthoMatrix(0.0f, 1.0f);
        nglCalculateMatrices(false);
        math::MatClass<4, 3> identity{identity_matrix};
        const rectangle full{vector2d{-1.0f, -1.0f}, vector2d{1.0f, 1.0f}};
        const rectangle uv{vector2d{0.0f, 0.0f}, vector2d{1.0f, 1.0f}};
        nglListBeginScene(static_cast<nglSceneParamType>(1));
        draw_panel_quad(identity, full, uv, nglWhiteTex, {0, 0, 100, 255}, true, 0, 0.0f, nullptr, 1);
        nglListEndScene();
        nglListBeginScene(static_cast<nglSceneParamType>(1));
        draw_panel_quad(identity, full, uv, nglWhiteTex, {0, 100, 0, 0}, true, 0, .9995f, nullptr, 1);
        nglListEndScene();
        nglListEndScene();
    }
    info.field_143 = false;
    nglListEndScene();
    page->field_4C = saved_view;
    page->field_3C = saved_page_bounds;
    page_bounds = saved_bounds;
    info.field_144 = saved_square;
    page_orthographic = saved_ortho;
    page->field_EC = saved_page_ortho;
    page->field_D8 = saved_aspect;
    const unsigned width = static_cast<unsigned>(size.x + .5f);
    const unsigned height = static_cast<unsigned>(size.y + .5f);
    field_18 = float(width);
    field_1C = float(height);
    if (field_44 &&
        (width > static_cast<unsigned>(field_44->m_width) || height > static_cast<unsigned>(field_44->m_height)))
        camera_resources(this, nullptr);
    if (!field_44)
        field_44 = create_panel_texture(width, height);
    if (field_44) {
        auto *capture = new (nglListAlloc(sizeof(panel_capture_data), 16))
            panel_capture_data{field_44, RECT{0, 0, LONG(width), LONG(height)}};
        nglSortInfo sort{};
        sort.Type = NGLSORT_OPAQUE;
        sort.Dist = 0.0f;
        nglListBeginScene(static_cast<nglSceneParamType>(1));
        nglListAddCustomNode(capture_panel_callback, capture, &sort);
        nglListEndScene();
    }
    world_has_been_rendered = true;
    var<bool>(0x0096F7B8) = true;
}

void panel_component_base::_render(panel_component::render_info &info)
{
    info.field_138 *= field_8;
}

void panel_component_base::capture(panel_component::render_info &a2)
{
    auto callback =
        reinterpret_cast<void(__fastcall *)(panel_component_base *, void *, panel_component::render_info *)>(
            get_vfunc(m_vtbl, 0x18));
    callback(this, nullptr, &a2);
}


void panel_component_camera::setup_geomgr(render_info &info)
{
    if (info.field_142)
        return;
    auto pixels = project_panel_rectangle(reinterpret_cast<const matrix4x4 &>(info.field_0), info.field_114);
    const rectangle full{vector2d{-1.0f, -1.0f}, vector2d{1.0f, 1.0f}};
    auto clip = intersect_rect(screen_rectangle(pixels), full);
    if (info.field_141)
        clip = intersect_rect(clip, info.field_124);
    if (clip.field_0[1][0] - clip.field_0[0][0] < 4.0f / 640.0f ||
        clip.field_0[1][1] - clip.field_0[0][1] < 4.0f / 480.0f) {
        info.field_142 = true;
        return;
    }
    geometry_manager::set_scissor(clip);
    if (field_48 & 2) {
        pixels = project_panel_rectangle(identity_matrix, field_8);
    } else {
        for (unsigned axis = 0; axis < 2; ++axis) {
            const float size = pixels.field_0[1][axis] - pixels.field_0[0][axis];
            pixels.field_0[1][axis] = pixels.field_0[0][axis] + size * field_8.field_0[1][axis];
            pixels.field_0[0][axis] += size * field_8.field_0[0][axis];
        }
    }
    geometry_manager::set_viewport(screen_rectangle(pixels));
    auto *selected = get_default_camera();
    float fov = geometry_manager::get_field_of_view();
    if (selected && !geometry_manager::is_scene_analyzer_enabled()) {
        fov = selected->get_fov();
        selected->adjust_geometry_pipe(false);
    }
    const float aspect = (pixels.field_0[1][0] - pixels.field_0[0][0]) / (pixels.field_0[1][1] - pixels.field_0[0][1]);
    if (aspect < 1.3333334f)
        fov = 2.0f * std::atan(aspect * std::tan(fov * 0.5f) * 0.75f);
    geometry_manager::set_field_of_view(fov);
    set_scene_params(info);
}


vector2d panel::project_position(const vector3d &position)
{
    const bool rebuild = geometry_manager::get_auto_rebuild_view_frame();
    geometry_manager::set_auto_rebuild_view_frame(false);
    panel_component::render_info info{*this, 0};
    for (auto *component = field_60; component; component = component->field_4) {
        if (component->m_vtbl == camera_table())
            reinterpret_cast<panel_component_camera *>(component)->setup_geomgr(info);
    }
    geometry_manager::set_auto_rebuild_view_frame(rebuild);
    geometry_manager::rebuild_view_frame();
    const auto &matrix = geometry_manager::get_xform(geometry_manager::XFORM_WORLD_TO_SCREEN);
    const auto projected = matrix * position;
    if (projected.z >= 1.0f)
        return {-1.0f, -1.0f};
    const auto &bounds = page_rectangle();
    return {(projected.x - bounds.field_0[0][0]) * 640.0f / (bounds.field_0[1][0] - bounds.field_0[0][0]),
            480.0f - (projected.y - bounds.field_0[0][1]) * 480.0f / (bounds.field_0[1][1] - bounds.field_0[0][1])};
}


void release_panel(panel *value)
{
    for (uint32_t i = 0; i < panels().m_size; ++i) {
        if (panels().m_data[i] == value) {
            panels().sub_CBF970(i);
            break;
        }
    }
    for (auto *component = value->field_60; component;) {
        auto *next = component->field_4;
        auto destroy =
            reinterpret_cast<void(__fastcall *)(panel_component_base *, void *, bool)>(get_vfunc(component->m_vtbl, 0));
        destroy(component, nullptr, true);
        component = next;
    }
    mem_dealloc(value, sizeof(panel));
}


}  // namespace comic_panels


void __fastcall sub_742C50(void *self, int, comic_panels::panel_component_base *a2)
{
    sp_log("0x%08X", a2->m_vtbl);

    THISCALL(0x00742C50, self, a2);
}

void comic_panels_patch()
{
    {
        FUNC_ADDRESS(address, &comic_panels::panel::init_anim);
        REDIRECT(0x0073661A, address);
    }

    FUNC_ADDRESS(address, &comic_panels::panel::add_camera_component);
    REDIRECT(0x006424BB, address);

    {
        FUNC_ADDRESS(address, &comic_panels::panel::render);
        REDIRECT(0x0073E84C, address);
        REDIRECT(0x0073EA2E, address);
    }

    REDIRECT(0x00736AE5, comic_panels::sub_7315A0);

    {
        REDIRECT(0x005D7112, comic_panels::render);
    }

    REDIRECT(0x0055D8CC, comic_panels::frame_advance);

    REDIRECT(0x0073EB04, comic_panels::render_panels);

    {
        FUNC_ADDRESS(address, &comic_panels::panel_component_camera::_render);
        set_vfunc(0x008AA340, address);
    }

    {
        FUNC_ADDRESS(address, &comic_panels::panel_component_camera::_capture);
        set_vfunc(0x008AA344, address);
    }

    {
        FUNC_ADDRESS(address, &comic_panels::panel_component_base::_render);
        set_vfunc(0x008A9E04, address);
    }

    REDIRECT(0x00743453, sub_742C50);
}
