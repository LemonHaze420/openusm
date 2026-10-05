#include "beam.h"

#include "app.h"
#include "camera.h"
#include "comic_panels.h"
#include "common.h"
#include "debug_render.h"
#include "func_wrapper.h"
#include "game.h"
#include "geometry_manager.h"
#include "local_collision.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "ngl_dx_vertexdef.h"
#include "oldmath_po.h"
#include "us_pcuv_shader.h"
#include "trace.h"
#include "time_interface.h"
#include "variable.h"
#include "vtbl.h"
#include "wds.h"

#include <cmath>
#include "collide.h"
#include "conglom.h"
#include "dynamic_rtree.h"
#include "event.h"
#include "filespec.h"
#include "memory.h"
#include "moved_entities.h"
#include "region.h"
#include "rtree.h"
#include "subdivision_visitor.h"
#include "terrain.h"
#include <algorithm>
#include <array>
#include <cstring>
#include "subdivision_obb.h"
#include "region_mash_info.h"

VALIDATE_SIZE(beam, 0xE8u);
static Var<beam *> active_beams{0x0095C750};
static Var<beam *> inactive_beams{0x0095C74C};

namespace {
std::array<void *, 0x214 / 4> beam_table;
void *__fastcall beam_delete(beam *self, void *, unsigned flags)
{
    self->~beam();
    if (flags & 1) mem_dealloc(self, sizeof(beam));
    return self;
}
int __fastcall beam_size(beam *, void *) { return sizeof(beam); }
int __fastcall beam_flavor(beam *, void *) { return 14; }
bool __fastcall beam_chunk(beam *, void *, void *, void *) { return false; }
bool __fastcall beam_query(beam *, void *) { return true; }
float __fastcall beam_radius(beam *self, void *) { return self->field_A8 * 0.5f; }
vector3d *__fastcall beam_center(beam *self, void *, vector3d *out)
{
    const auto &pose = self->get_abs_po();
    *out = pose.get_position() + pose.get_z_facing() * self->get_visual_radius();
    return out;
}
void __fastcall beam_release(beam *self, void *) { self->release_mem(); }
void __fastcall beam_changed(beam *self, void *) { self->po_changed(); }
void __fastcall beam_visible(beam *self, void *, bool value, bool recursive) { self->_set_visible(value, recursive); }
void __fastcall beam_unmash(beam *self, void *, generic_mash_header *h, void *o, generic_mash_data_ptrs *p) { self->un_mash(h, o, p); }
void __fastcall beam_advance(beam *self, void *, Float dt) { self->_frame_advance(dt); }
void __fastcall beam_render(beam *self, void *, Float fade) { self->_render(fade); }
float __fastcall beam_length(beam *self, void *) { return self->field_A8; }
void __fastcall beam_regions(beam *self, void *, region *r, int value) { self->visit_connected_regions(r, value); }
}

void *beam::native_vtable(void **entity_table)
{
    std::copy_n(entity_table, 0x20C / 4, beam_table.begin());
    beam_table[0] = reinterpret_cast<void *>(&beam_delete);
    beam_table[1] = reinterpret_cast<void *>(&beam_size);
    beam_table[0x10 / 4] = reinterpret_cast<void *>(&beam_release);
    beam_table[0x20 / 4] = reinterpret_cast<void *>(&beam_chunk);
    beam_table[0x28 / 4] = reinterpret_cast<void *>(&beam_radius);
    beam_table[0x2C / 4] = reinterpret_cast<void *>(&beam_center);
    beam_table[0x34 / 4] = reinterpret_cast<void *>(&beam_changed);
    beam_table[0x44 / 4] = reinterpret_cast<void *>(&beam_visible);
    beam_table[0x54 / 4] = reinterpret_cast<void *>(&beam_flavor);
    beam_table[0xFC / 4] = reinterpret_cast<void *>(&beam_query);
    beam_table[0x164 / 4] = reinterpret_cast<void *>(&beam_unmash);
    beam_table[0x1A4 / 4] = reinterpret_cast<void *>(&beam_advance);
    beam_table[0x1AC / 4] = reinterpret_cast<void *>(&beam_render);
    beam_table[0x20C / 4] = reinterpret_cast<void *>(&beam_length);
    beam_table[0x210 / 4] = reinterpret_cast<void *>(&beam_regions);
    return beam_table.data();
}

beam::beam() : beam(string_hash{}, 0) {}

beam::beam(const string_hash &id, uint32_t flags) : entity(id, flags)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(beam_table.data());
    field_68 = field_6C = nullptr;
    field_84 = field_88 = field_8C = field_90 = 0;
    field_7C = field_80 = color32{0, 0, 255, 128};
    field_E2 = field_E3 = false;
    update_list();
    field_70 = 0.05f;
    field_74 = 30.0f;
    field_78 = 0.0f;
    field_94 = 0;
    field_98 = field_AC = field_B8 = ZEROVEC;
    field_A4 = field_A8 = 0.0f;
    field_4 |= 0x300;
    field_E0 = 0;
    field_C4 = field_C8 = 0.0f;
    field_CC[0] = vector2d{0.0f, 0.0f};
    field_CC[1] = vector2d{1.0f, 1.0f};
    field_DC = -1.0f;
    my_material = nullptr;
    set_texture(mString{"c_alpha"});
}

void beam::unlink()
{
    auto *next = reinterpret_cast<beam *>(field_68);
    auto *prev = reinterpret_cast<beam *>(field_6C);
    if (prev) prev->field_68 = field_68;
    else if (inactive_beams() == this) inactive_beams() = next;
    else if (active_beams() == this) active_beams() = next;
    if (next) next->field_6C = field_6C;
    field_68 = field_6C = nullptr;
}

void beam::update_list()
{
    unlink();
    auto &head = (field_4 & 0x200) ? active_beams() : inactive_beams();
    field_68 = reinterpret_cast<int *>(head);
    head = this;
    if (field_68) reinterpret_cast<beam *>(field_68)->field_6C = reinterpret_cast<int *>(this);
}

beam::~beam()
{
    unlink();
    auto **begin = reinterpret_cast<void **>(field_88);
    auto **end = reinterpret_cast<void **>(field_8C);
    for (auto **it = begin; it != end; ++it) {
        if (*it) {
            auto destroy = reinterpret_cast<void (__fastcall *)(void *, void *, unsigned)>(
                get_vfunc(*reinterpret_cast<std::intptr_t *>(*it), 0xC));
            destroy(*it, nullptr, 1);
        }
    }
    ::operator delete(begin);
    field_88 = field_8C = field_90 = 0;
    if (my_material) {
        if (my_material->m_texture) nglReleaseTexture(my_material->m_texture);
        delete my_material;
    }
}

void beam::release_mem() { unlink(); entity::release_mem(); }
void beam::po_changed() { field_94 &= ~1; entity_base::po_changed(); }
void beam::_set_visible(bool visible, bool recursive) { entity::_set_visible(visible, recursive); update_list(); }
void beam::un_mash(generic_mash_header *header, void *object, generic_mash_data_ptrs *data)
{
    entity::un_mash(header, object, data);
    field_68 = field_6C = nullptr;
    update_list();
}

void beam::set_texture(const mString &name)
{
    if (!my_material) my_material = new PCUV_ShaderMaterial;
    else if (my_material->m_texture) {
        nglReleaseTexture(my_material->m_texture);
        my_material->m_texture = nullptr;
    }
    filespec file{name};
    if (file.m_dir.size() == 0) file.m_dir = "textures\\";
    nglSetTexturePath(file.m_dir.c_str());
    my_material->m_texture = nglLoadTexture(tlFixedString{file.m_name.c_str()});
}

void beam::set_point_to_point(const vector3d &start, const vector3d &end)
{
    if (field_8 & 0x8000000) compute_rel_po_from_model();
    vector3d direction = end - start;
    const float length = std::sqrt(direction.length2());
    field_74 = std::max(length, 0.0f);
    field_78 = 0.0f;
    if (length > 0.0f) {
        if (direction.x <= 0.0f && direction.x >= 0.0f &&
            direction.y <= 0.0f && direction.y >= 0.0f) {
            if (direction.z < 0.0f) {
                po pose;
                pose.set_rot(YVEC, Float{3.1415927f});
                pose.set_position(get_rel_position());
                set_abs_po(pose);
            } else if (direction.z > 0.0f) {
                const auto position = get_rel_position();
                set_abs_po(po_identity_matrix);
                set_abs_position(position);
            }
        } else {
            direction *= 1.0f / length;
            vector3d axis{direction.y, -direction.x, 0.0f};
            po pose = po_identity_matrix;
            pose.set_rot(axis, Float{std::acos(std::clamp(direction.z, -1.0f, 1.0f))});
            *my_rel_po = pose;
            dirty_family(false);
            if (field_4 & 0x8004) dirty_model_po_family();
            po_changed();
        }
        set_abs_position(start);
    }
    field_94 &= ~1;
    vhandle_type<entity> handle;
    handle.field_0 = my_handle;
    moved_entities::add_moved(handle);
}

namespace {
bool beam_hit_entity(beam *self, entity *candidate, const vector3d &start, vector3d &end)
{
    bool hit = false;
    if ((candidate->field_4 & 0x84000) == 0x84000 &&
        collide_segment_entity(start, end, candidate, candidate->get_abs_po(), &self->field_AC, &self->field_B8)) {
        self->field_98 = self->field_AC;
        end = self->field_98;
        hit = true;
    }
    if (candidate->field_4 & 4) {
        auto *members = static_cast<conglomerate *>(candidate)->field_FC;
        if (members)
            for (auto *member : *members)
                if (beam_hit_entity(self, member, start, end)) hit = true;
    }
    return hit;
}

struct beam_collision_visitor : subdivision_visitor {
    beam *owner;
    const vector3d *start;
    vector3d *end;
    bool hit = false;
    static int visit_node(subdivision_visitor &base, const subdivision_node &node)
    {
        auto &self = static_cast<beam_collision_visitor &>(base);
        auto *candidate = reinterpret_cast<entity *>(const_cast<subdivision_node *>(&node));
        if (candidate->field_5C != entity::visit_key) {
            candidate->field_5C = entity::visit_key;
            if (beam_hit_entity(self.owner, candidate, *self.start, *self.end)) self.hit = true;
        }
        return self.hit ? 2 : 0;
    }
    beam_collision_visitor(beam *b, const vector3d &s, vector3d &e) : owner(b), start(&s), end(&e)
    {
        static const native_vtable table{visit_node, nullptr};
        m_vtbl = reinterpret_cast<std::intptr_t>(&table);
    }
};
}

void beam::_frame_advance(Float elapsed)
{
    const float dt = elapsed.value;
    const auto &pose = get_abs_po();
    const vector3d start = pose.get_position();
    const vector3d forward = pose.get_z_facing();
    vector3d end = start + forward * field_74;
    field_A4 = field_74;
    field_AC = field_98 = end;
    field_94 &= ~2;
    if (!(field_94 & 1) || (field_94 & 0x200) || ((field_94 & 4) && !(field_94 & 8))) {
        if (!(field_94 & 0x80) &&
            find_intersection(start, end, *local_collision::entfilter_reject_all,
                *local_collision::obbfilter_lineseg_test, &field_AC, &field_B8, nullptr, nullptr, nullptr, false)) {
            field_98 = end = field_AC;
            field_94 |= 2;
        }
        if (!(field_94 & 0x40)) {
            ++entity::visit_key;
            const int count = count_in_regions();
            for (int i = 0; i < count; ++i) {
                auto *r = i < 2 ? regions[i] : extended_regions->m_data[i - 2];
                beam_collision_visitor visitor{this, start, end};
                traverse_rtree(start, end, r->collision_proximity_map->state->field_110, visitor);
            }
        }
        field_94 |= 1;
    }
    bool hit_player = false;
    if (field_A8 > 0.01f) {
        for (int i = 0; i < g_world_ptr->num_players; ++i) {
            auto *player = g_world_ptr->get_hero_ptr(i);
            if (!player) continue;
            if (collide_segment_entity(start, end, player, player->get_abs_po(), &field_AC, &field_B8)) {
                hit_player = true;
                if (!(field_94 & 8)) field_98 = end = field_AC;
            } else if (!hit_player) {
                if (field_94 & 4) {
                    field_94 &= ~4;
                    raise_event(event::LEAVE);
                }
                continue;
            }
            if (!(field_94 & 8)) {
                field_AC = get_abs_position() + get_abs_po().get_z_facing() * field_A8;
                field_B8 = get_abs_po().get_z_facing() * -1.0f;
            }
            if (!(field_94 & 4)) {
                field_94 |= 4;
                raise_event(event::ENTER);
            }
        }
    }
    field_A4 = field_A8 = dot(end - start, forward);
    auto **it = reinterpret_cast<void **>(field_88);
    auto **end_effects = reinterpret_cast<void **>(field_8C);
    while (it != end_effects) {
        auto *effect = *it;
        auto table = *reinterpret_cast<std::intptr_t *>(effect);
        reinterpret_cast<void (__fastcall *)(void *, void *, float)>(get_vfunc(table, 0x24))(effect, nullptr, dt);
        if (reinterpret_cast<bool (__fastcall *)(void *, void *)>(get_vfunc(table, 0x2C))(effect, nullptr)) {
            reinterpret_cast<void (__fastcall *)(void *, void *, unsigned)>(get_vfunc(table, 0xC))(effect, nullptr, 1);
            std::memmove(it, it + 1, (--end_effects - it) * sizeof(void *));
            field_8C = reinterpret_cast<int>(end_effects);
        } else ++it;
    }
    if (field_E3) {
        for (int axis = 0; axis < 2; ++axis) {
            const float movement = dt * (axis ? field_C8 : field_C4);
            field_CC[0][axis] += movement;
            field_CC[1][axis] += movement;
            while (field_CC[0][axis] >= 1.0f) {
                field_CC[0][axis] -= 1.0f;
                field_CC[1][axis] -= 1.0f;
            }
            while (field_CC[0][axis] <= -1.0f) {
                field_CC[0][axis] += 1.0f;
                field_CC[1][axis] += 1.0f;
            }
        }
    }
}

void beam::visit_connected_regions(region *origin, int mode)
{
    origin->visited = region::visit_key;
    for (auto index : origin->neighbors) {
        auto *neighbor = g_world_ptr->the_terrain->regions[index];
        if (neighbor->visited == region::visit_key) continue;
        if (neighbor->obb->sphere_intersection(get_abs_position(), Float{get_visual_radius()})) {
            auto *loaded = g_world_ptr->the_terrain->find_region(string_hash{neighbor->mash_info->field_0.to_string()});
            if (loaded) visit_connected_regions(loaded, mode);
        }
    }
}

void beam::_render(Float a1)
{
    TRACE("beam::render");

    if constexpr (1) {
        if (this->field_A8 > 0.0f && this->field_78 < this->field_A8) {
            auto *the_game = app::instance->m_game;
            auto *cam = the_game->get_current_view_camera(0);

            vector3d abs_position = this->get_abs_position();
            po &abs_po = this->get_abs_po();

            vector3d cam_abs_pos = cam->get_abs_position();

            auto minus_result = cam_abs_pos - abs_position;
            auto y = dot(minus_result, abs_po.get_y_facing());
            auto x = dot(minus_result, abs_po.get_x_facing());
            minus_result = vector3d{x, y, 0.0};

            auto len = minus_result.length2();
            if (len > 0.001f) {
                auto v16 = 1.0f / std::sqrt(len);
                minus_result *= v16;

                auto v52 = this->field_70 * 0.5f;

                vector3d v45[3]{
                    vector3d{0.0, 0.0, this->field_A8},
                    vector3d{0.0, 0.0, this->field_78},
                    vector3d{static_cast<float>((0.0 - minus_result[1]) * v52), minus_result[0] * v52, 0.0}};

                vector3d v44[4]{};
                v44[3] = v45[1] - v45[2];
                v44[2] = v45[1] + v45[2];
                v44[1] = v45[0] + v45[2];
                v44[0] = v45[0] - v45[2];

                vector2d v43[4]{};

                if (this->field_DC <= 0.0f) {
                    v43[3] = this->field_CC[0];
                    v43[2][0] = this->field_CC[1][0];
                    v43[2][1] = this->field_CC[0][1];
                    v43[1] = this->field_CC[1];
                    v43[0][0] = this->field_CC[0][0];
                    v43[0][1] = this->field_CC[1][1];
                } else {
                    float v42 = (this->field_A8 * this->field_DC) - 1.0;
                    v43[3] = this->field_CC[0];
                    v43[2][0] = this->field_CC[1][0];
                    v43[2][1] = this->field_CC[0][1];
                    v43[1][0] = this->field_CC[1][0];
                    v43[1][1] = this->field_CC[1][1] + v42;
                    v43[0][0] = this->field_CC[0][0];
                    v43[0][1] = this->field_CC[1][1] + v42;
                }

                nglCreateMesh(0x40000u, 2u, 0, nullptr);

                assert(this->my_material != nullptr);

                auto v24 = (this->field_E2 != 0);
                this->my_material->m_blend_mode = static_cast<nglBlendModeType>(v24 + 2);

                uint32_t color0 = color32::to_int(this->field_7C);
                uint32_t color1 = color32::to_int(this->field_80);

                nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator Iter{};
                nglMaterialBase *v29 =
                    (this->my_material != nullptr ? bit_cast<nglMaterialBase *>(&this->my_material->field_4) : nullptr);

                auto *v30 = sub_507920(v29, 3, 1, 0, nullptr, D3DPT_TRIANGLESTRIP, true);
                Iter = v30->CreateIterator();
                Iter.BeginStrip(3u);

                Iter.Write(v44[3], color0, v43[3]);
                ++Iter;

                Iter.Write(v44[2], color0, v43[2]);
                ++Iter;

                Iter.Write(v44[1], color1, v43[1]);
                ++Iter;

                auto *v33 = Iter.field_4->field_4;
                if ((v33->Flags & 0x40000) == 0) {
                    IDirect3DVertexBuffer9_Unlock(v33->field_3C.getVertexBuffer());
                }

                nglMaterialBase *v36 =
                    (this->my_material != nullptr ? bit_cast<nglMaterialBase *>(&this->my_material->field_4) : nullptr);

                auto *v37 = sub_507920(v36, 3, 1, 0, nullptr, D3DPT_TRIANGLESTRIP, true);
                Iter = v37->CreateIterator();
                Iter.BeginStrip(3u);

                Iter.Write(v44[3], color0, v43[3]);
                ++Iter;

                Iter.Write(v44[0], color1, v43[0]);
                ++Iter;

                Iter.Write(v44[1], color1, v43[1]);
                ++Iter;

                auto *v40 = Iter.field_4->field_4;
                if ((v40->Flags & 0x40000) == 0) {
                    IDirect3DVertexBuffer9_Unlock(v40->field_3C.getVertexBuffer());
                }

                auto v49 = *bit_cast<math::MatClass<4, 3> *>(&this->get_abs_po().get_matrix());
                auto v42 = nglCloseMesh();
                nglListAddMesh(v42, v49, nullptr, nullptr);
            }
        }
    } else {
        THISCALL(0x005406D0, this, a1);
    }
}

void beam::frame_advance_all_beams(Float elapsed)
{
    for (auto *current = active_beams(); current != nullptr;) {
        auto *next = reinterpret_cast<beam *>(current->field_68);
        const float scale = current->field_58 != nullptr
            ? static_cast<float>(current->field_58->sub_4ADE50())
            : g_world_ptr->time_manager.field_0;
        if (current->m_vtbl != 0) {
            auto *address = get_vfunc(current->m_vtbl, 0x1A4);
            if (address != nullptr) {
                void(__fastcall *frame_advance)(beam *, void *, Float) =
                    CAST(frame_advance, address);
                frame_advance(current, nullptr, Float{scale * elapsed.value});
            }
        }
        current = next;
    }
}

bool sub_CB3D60(unsigned int a5)
{
    nglCreateMesh(0x40000, a5, 0, nullptr);
    assert(nglScratch() != nullptr);
    return (nglScratch() != nullptr);
}

void sub_78EA60(vector3d &a1, float a2)
{
    auto v4 = a1.length2();
    if (v4 > (0.0000099999997 * 0.0000099999997)) {
        auto v3 = [](float a1) {
            return 1.0 / std::sqrt(a1);
        }(v4)*a2;

        a1 *= v3;
    }
}

void sub_CB4800(const vector3d &a1, const vector3d &arg4, int a3, float a4, void *a6)
{
    auto v44 = arg4 - a1;
    auto a3a = v44.length2();
    if (a3a >= 9.9999997e-10) {
        a3a = std::sqrt(a3a);
        auto v42 = v44 / a3a;

        auto camera_pos = g_game_ptr->get_current_view_camera(0)->get_abs_position();

        auto v40 = camera_pos - a1;
        auto v39 = camera_pos - arg4;
        auto v38 = v40.length2();
        auto v37 = v39.length2();

        auto sub_6BAED0 = [](float a1) {
            return 1.0 / std::sqrt(a1);
        };

        if (not_equal<float>(v38, 0.0f)) {
            auto v17 = 0.5 * a4;

            v40 *= sub_6BAED0(v38) * v17;
        }

        if (not_equal<float>(v37, 0.0f)) {
            auto v18 = 0.5 * a4;
            v39 *= sub_6BAED0(v37) * v18;
        }

        v40 += a1;
        v39 += arg4;

        auto sub_CB4C90 = [](float a1) {
            return 1.0 - 0.75 / ((0.25 * 0.25) * a1 + 1.0);
        };

        auto v36 = sub_CB4C90(v38);
        auto v35 = sub_CB4C90(v37);
        auto v5 = a1 + arg4;
        auto v6 = v5 * 0.5;
        auto v34 = v6 - camera_pos;
        auto v33 = vector3d::cross(v34, v42);
        if (v33.length2() >= 9.9999997e-10 || geometry_manager::is_scene_analyzer_enabled()) {
            sub_78EA60(v33, a4 * 0.5);
            vector3d v32{};
            vector3d v31{};
            vector3d v30{};
            vector3d a4a{};
            auto v7 = v33 * v36;
            v32 = v40 - v7;

            auto v9 = v33 * v36;
            v31 = v40 + v9;

            auto v11 = v33 * v35;
            v30 = v39 + v11;

            auto v13 = v33 * v35;
            a4a = v39 - v13;
            sub_CB3F80(v32, v31, v30, a4a, a3, a6);
        }
    }
}

void beam_patch()
{
    {
        FUNC_ADDRESS(address, &beam::_render);
        set_vfunc(0x00889B9C, address);
    }
}
