#include "entity_mash.h"

#include "entity_base.h"
#include "entity.h"
#include "conglom.h"
#include "conglomerate_clone.h"
#include "light_source.h"
#include "item.h"
#include "handheld_item.h"
#include "melee_item.h"
#include "gun.h"
#include "beam.h"
#include "thrown_item.h"
#include "grenade.h"
#include "visual_item.h"
#include "ai_cover_marker.h"
#include "memory.h"
#include "vtbl.h"
#include "physical_interface.h"
#include "damage_interface.h"
#include "facial_expression_interface.h"
#include "time_interface.h"
#include "signaller.h"
#include "skeleton_interface.h"
#include "animation_interface.h"
#include "script_data_interface.h"
#include "variant_interface.h"
#include "ngl_mesh.h"
#include "collision_geometry.h"
#include "oldmath_po.h"
#include "effect_mash_layout.h"
#include "native_pfx.h"
#include "sound_and_pfx_interface.h"
#include "func_wrapper.h"
#include "parse_generic_mash.h"
#include "trace.h"
#include "utility.h"

#include <cassert>
#ifdef OPENUSM_XBPACK_MODE
#include <cstdio>
#include <windows.h>
#endif

#if STANDALONE_SYSTEM
int ent_v_table_lookup[28]{};
int ent_size_lookup[28]{};
std::array<int, 13> ifc_v_table_lookup{};
#else
Var<int[28]> ent_v_table_lookup{0x0095A5F0};
Var<int[28]> ent_size_lookup{0x0095A2A0};
std::array<int, 11> &ifc_v_table_lookup = var<std::array<int, 11>>(0x0095A66C);
#endif
static int *entity_vtables()
{
#if STANDALONE_SYSTEM
    return ent_v_table_lookup;
#else
    return ent_v_table_lookup();
#endif
}

static int *entity_sizes()
{
#if STANDALONE_SYSTEM
    return ent_size_lookup;
#else
    return ent_size_lookup();
#endif
}

uint16_t pc_entity_mash_type(uint16_t type)
{
#ifdef OPENUSM_XBPACK_V10
    if (type >= 27)
        return type - 1;
#endif
    return type;
}

uint32_t entity_mash_size(uint16_t type)
{
#ifdef OPENUSM_XBPACK_V10
    static constexpr uint16_t v10_sizes[] = {
        0x44,  0x48, 0x68, 0xBC, 0xE8, 0x12C, 0xCC, 0x1A0, 0xFC, 0x110, 0x328, 0x148, 0x340, 0x274, 0x6C,
        0x158, 0x68, 0x68, 0x68, 0x70, 0x68,  0x84, 0xBC,  0x6C, 0x150, 0xDC,  0xD8,  0xC4,  0x78,
    };

    assert(type < sizeof(v10_sizes) / sizeof(v10_sizes[0]));
    return v10_sizes[type];
#else
    assert(type < 28);
    return entity_sizes()[type];
#endif
}

void fix_entity_v_table(char *addr, eEntityMashTypeEnum type)
{
#if defined(OPENUSM_XBPACK_MODE) && !STANDALONE_SYSTEM
#ifndef OPENUSM_XBPACK_V10
    uint32_t current_vtable = 0;
    uint32_t mashed_vtable = 0;
    std::memcpy(&current_vtable, addr, sizeof(current_vtable));
    std::memcpy(&mashed_vtable, MASH_V_TABLE_VAL, sizeof(mashed_vtable));
    const auto expected_vtable = static_cast<uint32_t>(entity_vtables()[type]);
    if (current_vtable != mashed_vtable && current_vtable != expected_vtable) {
        char message[192];
        std::snprintf(message,
                      sizeof(message),
                      "XBPACK invalid entity vtable: addr=%p type=%d actual=0x%08X expected=0x%08X\n",
                      addr,
                      static_cast<int>(type),
                      current_vtable,
                      expected_vtable);
        OutputDebugStringA(message);
        DebugBreak();
        return;
    }
#endif
#elif !STANDALONE_SYSTEM
    assert(addr[0] == MASH_V_TABLE_VAL[0] || addr[0] == ((char *)&entity_vtables()[type])[0]);
    assert(addr[1] == MASH_V_TABLE_VAL[1] || addr[1] == ((char *)&entity_vtables()[type])[1]);
    assert(addr[2] == MASH_V_TABLE_VAL[2] || addr[2] == ((char *)&entity_vtables()[type])[2]);
    assert(addr[3] == MASH_V_TABLE_VAL[3] || addr[3] == ((char *)&entity_vtables()[type])[3]);
#endif

    std::memcpy(addr, &entity_vtables()[type], 4);
}

void fix_ifc_v_table(char *addr, eEntityMashIFCTypeEnum ifc_type)
{
#ifndef OPENUSM_XBPACK_V10
    assert(addr[0] == ((const char *)&MASH_V_TABLE_VAL)[0] || addr[0] == ((char *)&ifc_v_table_lookup[ifc_type])[0]);

    assert(addr[1] == ((const char *)&MASH_V_TABLE_VAL)[1] || addr[1] == ((char *)&ifc_v_table_lookup[ifc_type])[1]);

    assert(addr[2] == ((const char *)&MASH_V_TABLE_VAL)[2] || addr[2] == ((char *)&ifc_v_table_lookup[ifc_type])[2]);

    assert(addr[3] == ((const char *)&MASH_V_TABLE_VAL)[3] || addr[3] == ((char *)&ifc_v_table_lookup[ifc_type])[3]);
#endif

    std::memcpy(addr, &ifc_v_table_lookup[ifc_type], 4);
}

#if STANDALONE_SYSTEM
template <typename T>
static void __fastcall native_entity_destroy(T *self, void *, bool free_memory)
{
    self->~T();
    if (free_memory)
        mem_dealloc(self, sizeof(T));
}

template <typename T>
static void __fastcall native_entity_release(T *self, void *)
{
    self->T::release_mem();
}

template <typename T>
static void __fastcall native_interface_release(T *self, void *)
{
    self->T::release_ifc();
}

template <typename T>
static void __fastcall native_heap_interface_destroy(T *self, void *, bool free_memory)
{
    self->~T();
    if (free_memory)
        ::operator delete(self);
}


static void __fastcall native_time_release(time_interface *, void *) {}

static void __fastcall native_line_anchor_destroy(entity *self, void *, bool free_memory)
{
    self->~entity();
    if (free_memory)
        mem_dealloc(self, 0x84);
}

static void __fastcall native_entity_family_visible(entity *self, void *, bool visible)
{
    self->entity::set_family_visible(visible);
}

static void __fastcall native_entity_clear_region(entity *self, void *, region *reg, int sentinel)
{
    self->entity::clear_region(reg, sentinel);
}

static void __fastcall native_entity_compute_sector(entity *self, void *, terrain *terrain_ptr, bool loading_scene,
                                                    entity *fallback)
{
    self->entity::_compute_sector(terrain_ptr, loading_scene, fallback);
}

static void __fastcall native_entity_update_ai_proximity(entity *self, void *)
{
    self->entity::update_ai_proximity_map_recursive();
}


static void __fastcall native_entity_set_age(entity *, void *, float) {}

static void __fastcall native_entity_set_recursive_age(entity *self, void *, float age)
{
    auto set_age = reinterpret_cast<void(__fastcall *)(entity *, void *, float)>(get_vfunc(self->m_vtbl, 0x204));
    set_age(self, nullptr, age);
}

static void __fastcall native_conglomerate_set_recursive_age(conglomerate *self, void *, float age)
{
    native_entity_set_recursive_age(self, nullptr, age);
    for (auto *member : self->members) {
        if (member->is_an_entity()) {
            auto set_age =
                reinterpret_cast<void(__fastcall *)(entity_base *, void *, float)>(get_vfunc(member->m_vtbl, 0x208));
            set_age(member, nullptr, age);
        }
    }
}

static void __fastcall native_conglomerate_update_ai_proximity(conglomerate *self, void *)
{
    for (auto *member : self->members) {
        if (member->is_an_entity()) {
            auto update = reinterpret_cast<void(__fastcall *)(entity_base *, void *)>(get_vfunc(member->m_vtbl, 0x184));
            update(member, nullptr);
        }
    }
}

static light_manager *__fastcall native_actor_light_set(actor *, void *)
{
    return nullptr;
}

static light_manager *__fastcall native_conglomerate_light_set(conglomerate *self, void *)
{
    return self->get_light_set();
}

static int __fastcall native_item_flavor(item *, void *)
{
    return ENTITY_ITEM;
}

static void __fastcall native_item_unmash(item *self, void *, generic_mash_header *header, void *object,
                                          generic_mash_data_ptrs *data)
{
    self->item::un_mash(header, object, data);
}

static bool __fastcall standalone_entity_true(entity_base *)
{
    return true;
}

static int __fastcall standalone_marker_flavor(entity_base *)
{
    return 5;
}
static int __fastcall standalone_parking_marker_flavor(entity_base *)
{
    return 6;
}

static int __fastcall standalone_water_exit_marker_flavor(entity_base *)
{
    return 7;
}

static int __fastcall standalone_anchor_marker_flavor(entity_base *)
{
    return 25;
}

static int __fastcall standalone_actor_flavor(entity_base *)
{
    return 0;
}
static int __fastcall standalone_conglomerate_flavor(entity_base *)
{
    return 12;
}

static int __fastcall standalone_light_source_flavor(entity_base *)
{
    return 9;
}

static int __fastcall standalone_line_anchor_flavor(entity_base *)
{
    return 26;
}

static bool __fastcall standalone_actor_true(const entity_base *)
{
    return true;
}

static bool __fastcall standalone_entity_false(const entity_base *)
{
    return false;
}

static ai::ai_core *__fastcall standalone_actor_ai_core(actor *self)
{
    return self->_get_ai_core();
}

static int __fastcall native_entity_flavor(entity *, void *)
{
    return 4;
}
static float __fastcall native_entity_visual_radius(entity *self, void *)
{
    if (!self->is_flagged(0x8004))
        return 0.0f;
    auto *owner = self->get_conglom_owner();
    return owner == nullptr ? 0.0f : owner->get_visual_radius();
}
static vector3d *__fastcall native_entity_visual_center(entity *self, void *, vector3d *out)
{
    if ((self->field_4 & 0x8004) != 0) {
        if (auto *owner = self->get_conglom_owner()) {
            *out = owner->get_visual_center();
            return out;
        }
    }
    *out = self->get_abs_position();
    return out;
}
static bool __fastcall native_entity_visible(entity *self, void *)
{
    return self->is_still_visible();
}
static color32 *__fastcall native_entity_color(entity *, void *, color32 *out)
{
    *out = color32{255, 255, 255, 255};
    return out;
}
static float __fastcall native_entity_alpha(entity *, void *)
{
    return 1.0f;
}
static vector3d *__fastcall native_entity_scale(entity *, void *, vector3d *out)
{
    *out = vector3d{1.0f, 1.0f, 1.0f};
    return out;
}
static void __fastcall native_entity_po_changed(entity_base *self, void *)
{
    self->po_changed();
}
static void __fastcall native_actor_frame_delta(actor *self, void *, const vector3d &translation, Float elapsed)
{
    self->set_frame_delta_trans_native(translation, elapsed);
}
static void __fastcall native_actor_invalidate_frame_delta(actor *self, void *)
{
    self->invalidate_frame_delta();
}
static void __fastcall native_entity_set_flag(entity_base *self, void *, entity_flag_t flag, bool enabled)
{
    self->set_flag_recursive(flag, enabled);
}
static void __fastcall native_entity_set_ext_flag(entity_base *self, void *, entity_ext_flag_t flag, bool enabled)
{
    self->set_ext_flag_recursive_internal(flag, enabled);
}
static void __fastcall native_entity_set_active(entity_base *self, void *, bool enabled)
{
    self->set_active(enabled);
}
static void __fastcall native_base_set_visible(entity_base *self, void *, bool visible, bool)
{
    self->field_4 = visible ? self->field_4 | 0x200 : self->field_4 & ~0x200u;
}
static void __fastcall native_entity_set_visible(entity *self, void *, bool visible, bool suppress_owner_update)
{
    self->_set_visible(visible, suppress_owner_update);
}
static void __fastcall native_actor_suspend(actor *self, void *, bool propagate)
{
    self->actor::suspend(propagate);
}
static void __fastcall native_actor_unsuspend(actor *self, void *, bool propagate)
{
    self->actor::unsuspend(propagate);
}

static float __fastcall native_actor_visual_radius(actor *self, void *)
{
    return self->_get_visual_radius();
}
static vector3d *__fastcall native_actor_visual_center(actor *self, void *, vector3d *out)
{
    *out = self->_get_visual_center();
    return out;
}
static bool __fastcall native_entity_renderable(entity *self, void *)
{
    return self->is_flagged(0x100u);
}
static bool __fastcall native_entity_has_time(entity *self, void *)
{
    return self->field_58 != nullptr;
}
static time_interface *__fastcall native_entity_time(entity *self, void *)
{
    return self->field_58;
}
static bool __fastcall native_actor_material_switching(actor *self, void *)
{
    return self->field_90.field_C != nullptr;
}
static void __fastcall native_actor_render(actor *self, void *, float fade)
{
    self->_render(Float{fade});
}
static nglMesh *__fastcall native_actor_mesh(actor *self, void *)
{
    return self->_get_mesh();
}
static color32 *__fastcall native_actor_color(actor *self, void *, color32 *out)
{
    *out = self->_get_render_color();
    return out;
}
static float __fastcall native_actor_alpha(actor *self, void *)
{
    return self->_get_render_alpha_mod();
}
static vector3d *__fastcall native_actor_scale(actor *self, void *, vector3d *out)
{
    *out = self->actor::get_render_scale();
    return out;
}
static void __fastcall native_actor_set_color(actor *self, void *, color32 value)
{
    self->_set_render_color(value);
}
static void __fastcall native_entity_set_alpha(entity *, void *, float) {}
static void __fastcall native_actor_set_alpha(actor *self, void *, float value)
{
    self->_set_render_alpha_mod(Float{value});
}
static void __fastcall native_conglomerate_set_alpha(conglomerate *self, void *, float value)
{
    self->_set_render_alpha_mod(Float{value});
}
static void __fastcall native_actor_set_scale(actor *self, void *, const vector3d &value)
{
    self->actor::set_render_scale(value);
}
static float __fastcall native_actor_floor(actor *self, void *)
{
    return self->actor::get_floor_offset();
}
static bool __fastcall native_actor_has_physical(actor *self, void *)
{
    return self->m_physical_interface != nullptr;
}
static physical_interface *__fastcall native_actor_physical(actor *self, void *)
{
    return self->m_physical_interface;
}
static void __fastcall native_actor_set_collisions(actor *self, void *, bool enabled, bool update_region)
{
    self->_set_collisions_active(enabled, update_region);
}
static void __fastcall native_entity_update_collision_region(entity *self, void *)
{
    self->region_update_poss_collide();
}
static bool __fastcall native_entity_possibly_collide(entity *self, void *)
{
    return self->possibly_collide();
}
static bool __fastcall native_conglomerate_possibly_collide(conglomerate *self, void *)
{
    return (self->field_FC != nullptr && !self->field_FC->empty()) ||
           (self->colgeom != nullptr && self->are_collisions_active());
}
static float __fastcall native_conglomerate_visual_radius(conglomerate *self, void *)
{
    return self->_get_visual_radius();
}
static bool __fastcall native_conglomerate_renderable(conglomerate *self, void *)
{
    return self->_is_renderable();
}
static void __fastcall native_conglomerate_render(conglomerate *self, void *, float fade)
{
    self->_render(Float{fade});
}
static skeleton_interface *__fastcall native_conglomerate_skeleton(conglomerate *self, void *)
{
    return self->skeleton_ifc;
}
static float __fastcall native_actor_colgeom_radius(actor *self, void *)
{
    return self->colgeom == nullptr ? 0.0f : self->colgeom->get_bounding_sphere_radius();
}
static vector3d *__fastcall native_actor_colgeom_center(actor *self, void *, vector3d *out)
{
    *out = self->get_abs_po().m * self->colgeom->get_local_space_bounding_sphere_center();
    return out;
}
static vector3d *__fastcall native_conglomerate_colgeom_center(conglomerate *self, void *, vector3d *out)
{
    *out = self->conglomerate::get_colgeom_center();
    return out;
}
static float __fastcall native_conglomerate_colgeom_radius(conglomerate *self, void *)
{
    return self->conglomerate::get_colgeom_radius();
}

static bool __fastcall native_base_get_ifc_num(entity_base *, void *, const resource_key &, float &, bool)
{
    return false;
}
static bool __fastcall native_actor_get_ifc_num(actor *self, void *, const resource_key &key, float &value, bool log)
{
    if (self->has_damage_ifc() && self->damage_ifc()->get_ifc_num(key, &value, log))
        return true;
    if (self->has_physical_ifc() && self->physical_ifc()->get_ifc_num(key, value, log))
        return true;

    return false;
}
static bool __fastcall native_base_set_ifc_num(entity_base *, void *, const resource_key &, float, bool)
{
    return false;
}
static bool __fastcall native_actor_set_ifc_num(actor *self, void *, const resource_key &key, float value, bool log)
{
    if (self->has_damage_ifc()) {
        auto *damage = self->damage_ifc();
        using setter = bool(__fastcall *)(damage_interface *, void *, const resource_key &, Float, bool);
        if (reinterpret_cast<setter>(get_vfunc(damage->m_vtbl, 0x8))(damage, nullptr, key, value, log))
            return true;
    }
    if (self->has_physical_ifc()) {
        auto *physical = self->physical_ifc();
        using setter = bool(__fastcall *)(physical_interface *, void *, const resource_key &, Float, bool);
        if (reinterpret_cast<setter>(get_vfunc(physical->m_vtbl, 0x8))(physical, nullptr, key, value, log))
            return true;
    }

    return false;
}
static void __fastcall native_actor_ifl_play(actor *self, void *)
{
    if (self->_get_mesh() != nullptr)
        self->field_90.field_6 |= 0x3FFF;
}
static void __fastcall native_actor_ifl_lock(actor *self, void *, int frame)
{
    if (self->_get_mesh() != nullptr)
        self->field_90.field_6 ^= (frame ^ self->field_90.field_6) & 0x3FFF;
}
static nglMorphSet *__fastcall native_actor_morph(actor *self, void *, const tlFixedString *name, bool create)
{
    return self->_get_morph(*name, create);
}
static nglMorphSet *__fastcall native_conglomerate_morph(conglomerate *self, void *, const tlFixedString *name,
                                                         bool create)
{
    return self->_get_morph(*name, create);
}

static bool __fastcall standalone_conglomerate_has_tentacle(conglomerate *self)
{
    return self->m_tentacle_interface != nullptr;
}

static tentacle_interface *__fastcall standalone_conglomerate_tentacle(conglomerate *self)
{
    return self->m_tentacle_interface;
}

static bool __fastcall standalone_conglomerate_has_variant(conglomerate *self)
{
    return self->m_variant_interface != nullptr;
}
static void __fastcall standalone_light_source_unmash(light_source *self, void *, generic_mash_header *header,
                                                      void *object, generic_mash_data_ptrs *data)
{
    self->_un_mash(header, object, data);
}

static variant_interface *__fastcall standalone_conglomerate_variant(conglomerate *self)
{
    return self->m_variant_interface;
}

static void __fastcall standalone_entity_base_unmash(entity_base *self, void *, generic_mash_header *header,
                                                     void *object, generic_mash_data_ptrs *data)
{
    self->entity_base::_un_mash(header, object, data);
}

static void __fastcall standalone_entity_unmash(entity *self, void *, generic_mash_header *header, void *object,
                                                generic_mash_data_ptrs *data)
{
    self->entity::un_mash(header, object, data);
}
static void __fastcall standalone_actor_unmash(actor *self, void *, generic_mash_header *header, void *object,
                                               generic_mash_data_ptrs *data)
{
    self->actor::_un_mash(header, object, data);
}

static void __fastcall standalone_conglomerate_unmash(conglomerate *self, void *, generic_mash_header *header,
                                                      void *object, generic_mash_data_ptrs *data)
{
    self->conglomerate::_un_mash(header, object, data);
}
static int __fastcall standalone_pfx_flavor(entity_base *)
{
    return PFX;
}


static void __fastcall standalone_pfx_unmash(entity *self, void *, generic_mash_header *header, void *object,
                                             generic_mash_data_ptrs *data)
{
    self->entity::un_mash(header, object, data);
    auto *owner = static_cast<native_pfx::Entity *>(self);
    owner->particle = native_pfx::load(data, owner);
}
static damage_interface *__fastcall native_base_damage(entity_base *, void *)
{
    return nullptr;
}

static bool __fastcall native_actor_has_damage(actor *self, void *)
{
    return self->m_damage_interface != nullptr;
}

static damage_interface *__fastcall native_actor_damage(actor *self, void *)
{
    return self->m_damage_interface;
}

static bool __fastcall native_actor_hero(actor *self, void *)
{
    return self->m_player_controller != nullptr;
}

static bool __fastcall native_actor_alive(actor *self, void *)
{
    return !self->has_damage_ifc() || self->damage_ifc()->field_1FC.field_0[0] > 0.0f;
}

static bool __fastcall native_damage_get_num(damage_interface *self, void *, const resource_key &key, float &value,
                                             bool log)
{
    return self->get_ifc_num(key, &value, log);
}

static bool __fastcall native_damage_set_num(damage_interface *self, void *, const resource_key &key, Float value,
                                             bool log)
{
    return self->set_ifc_num(key, value, log);
}

static bool __fastcall native_physical_get_num(physical_interface *self, void *, const resource_key &key, float &value,
                                               bool log)
{
    return self->get_ifc_num(key, value, log);
}

static bool __fastcall native_physical_set_num(physical_interface *self, void *, const resource_key &key, Float value,
                                               bool log)
{
    return self->set_ifc_num(key, value, log);
}

static bool __fastcall native_physical_get_vec(physical_interface *self, void *, const resource_key &key,
                                               vector3d &value, bool log)
{
    return self->get_ifc_vec(key, value, log);
}

static bool __fastcall native_physical_set_vec(physical_interface *self, void *, const resource_key &key,
                                               const vector3d &value, bool log)
{
    return self->set_ifc_vec(key, value, log);
}

static bool __fastcall native_physical_string(physical_interface *, void *, const resource_key &, mString &, bool)
{
    return false;
}

static void __fastcall native_physical_unmash(physical_interface *self, void *, generic_mash_header *header,
                                              void *owner, void *object, generic_mash_data_ptrs *data)
{
    self->un_mash(header, owner, object, data);
}

static void __fastcall native_animation_unmash(animation_interface *self, void *, generic_mash_header *header,
                                               void *owner, void *, generic_mash_data_ptrs *data)
{
    self->_un_mash(header, owner, 0, data);
}

static void __fastcall native_script_data_unmash(script_data_interface *self, void *, generic_mash_header *header,
                                                 void *owner, void *object, generic_mash_data_ptrs *data)
{
    self->_un_mash(header, owner, object, data);
}

static void __fastcall native_variant_unmash(variant_interface *self, void *, generic_mash_header *header, void *owner,
                                             void *object, generic_mash_data_ptrs *data)
{
    self->_un_mash(header, owner, object, data);
}

static void __fastcall native_skeleton_unmash(skeleton_interface *self, void *, generic_mash_header *, void *owner,
                                              void *, generic_mash_data_ptrs *data)
{
    self->my_conglomerate = static_cast<conglomerate *>(owner);
    self->dynamic = false;
    data->rebase(16);
    data->rebase(4);
    self->abs_po = data->get<po>(self->po_count);
    self->my_conglomerate->field_8 |= 0x10000000;
}

static const char *__fastcall native_physical_type(physical_interface *, void *)
{
    return "physical";
}

static void __fastcall native_physical_frame(physical_interface *self, void *, Float elapsed)
{
    self->frame_advance(elapsed);
}

static void __fastcall native_physical_force(physical_interface *self, void *, const vector3d &force,
                                             physical_interface::force_type type, const vector3d &point, int limb)
{
    self->apply_force_increment(force, type, point, limb);
}

#endif

void construct_v_table_lookup()
{
#if STANDALONE_SYSTEM
    static bool initialized;
    static void *marker_vtables[6][192]{};
    static void *entity_base_vtable[192]{};
    static void *entity_vtable[192]{};
    static void *pfx_vtable[192]{};
    static void *light_source_vtable[192]{};
    static void *actor_vtable[192]{};
    static void *conglomerate_vtable[192]{};
    static void *item_vtable[192]{};
    static void *signaller_vtable[192]{};
    static void *damage_vtable[64]{};
    static void *physical_vtable[12]{};
    static void *facial_vtable[64]{};
    static void *time_vtable[64]{};
    static void *skeleton_vtable[64]{};
    static void *animation_vtable[64]{};
    static void *script_data_vtable[64]{};
    static void *variant_vtable[64]{};
    if (initialized)
        return;
    initialized = true;
    ifc_v_table_lookup[12] = sound_and_pfx_interface::native_vtable();
    animation_vtable[0] = reinterpret_cast<void *>(native_heap_interface_destroy<animation_interface>);
    animation_vtable[0x1C / 4] = reinterpret_cast<void *>(native_animation_unmash);
    animation_vtable[0x24 / 4] = reinterpret_cast<void *>(native_interface_release<animation_interface>);
    ifc_v_table_lookup[0] = reinterpret_cast<int>(animation_vtable);
    script_data_vtable[0] = reinterpret_cast<void *>(native_heap_interface_destroy<script_data_interface>);
    script_data_vtable[0x1C / 4] = reinterpret_cast<void *>(native_script_data_unmash);
    script_data_vtable[0x24 / 4] = reinterpret_cast<void *>(native_interface_release<script_data_interface>);
    ifc_v_table_lookup[3] = reinterpret_cast<int>(script_data_vtable);
    damage_vtable[0] = reinterpret_cast<void *>(native_entity_destroy<damage_interface>);
    damage_vtable[0x4 / 4] = reinterpret_cast<void *>(native_damage_get_num);
    damage_vtable[0x8 / 4] = reinterpret_cast<void *>(native_damage_set_num);
    damage_vtable[0x24 / 4] = reinterpret_cast<void *>(native_interface_release<damage_interface>);
    physical_vtable[0] = reinterpret_cast<void *>(native_heap_interface_destroy<physical_interface>);
    physical_vtable[0x4 / 4] = reinterpret_cast<void *>(native_physical_get_num);
    physical_vtable[0x8 / 4] = reinterpret_cast<void *>(native_physical_set_num);
    physical_vtable[0xC / 4] = reinterpret_cast<void *>(native_physical_get_vec);
    physical_vtable[0x10 / 4] = reinterpret_cast<void *>(native_physical_set_vec);
    physical_vtable[0x14 / 4] = reinterpret_cast<void *>(native_physical_string);
    physical_vtable[0x18 / 4] = reinterpret_cast<void *>(native_physical_string);
    physical_vtable[0x1C / 4] = reinterpret_cast<void *>(native_physical_unmash);
    physical_vtable[0x20 / 4] = reinterpret_cast<void *>(native_physical_type);
    physical_vtable[0x24 / 4] = reinterpret_cast<void *>(native_interface_release<physical_interface>);
    physical_vtable[0x28 / 4] = reinterpret_cast<void *>(native_physical_frame);
    physical_vtable[0x2C / 4] = reinterpret_cast<void *>(native_physical_force);
    facial_vtable[0] = reinterpret_cast<void *>(native_heap_interface_destroy<facial_expression_interface>);
    facial_vtable[0x24 / 4] = reinterpret_cast<void *>(native_interface_release<facial_expression_interface>);
    time_vtable[0] = reinterpret_cast<void *>(native_entity_destroy<time_interface>);
    time_vtable[0x24 / 4] = reinterpret_cast<void *>(native_time_release);
    skeleton_vtable[0] = reinterpret_cast<void *>(native_heap_interface_destroy<skeleton_interface>);
    skeleton_vtable[0x1C / 4] = reinterpret_cast<void *>(native_skeleton_unmash);
    skeleton_vtable[0x24 / 4] = reinterpret_cast<void *>(native_interface_release<skeleton_interface>);
    ifc_v_table_lookup[1] = reinterpret_cast<int>(damage_vtable);
    ifc_v_table_lookup[2] = reinterpret_cast<int>(physical_vtable);
    ifc_v_table_lookup[4] = reinterpret_cast<int>(facial_vtable);
    ifc_v_table_lookup[5] = reinterpret_cast<int>(time_vtable);
    ifc_v_table_lookup[6] = reinterpret_cast<int>(skeleton_vtable);
    variant_vtable[0] = reinterpret_cast<void *>(native_heap_interface_destroy<variant_interface>);
    variant_vtable[0x1C / 4] = reinterpret_cast<void *>(native_variant_unmash);
    variant_vtable[0x24 / 4] = reinterpret_cast<void *>(native_interface_release<variant_interface>);
    ifc_v_table_lookup[10] = reinterpret_cast<int>(variant_vtable);

    static constexpr int sizes[28] = {0x44,  0x48,  0x68,  0xC0,  0xE8,  0x130, 0xD0, 0x1A4, 0x100, 0x114,
                                      0x374, 0x14C, 0x350, 0x274, 0x6C,  0x15C, 0x68, 0x68,  0x68,  0x70,
                                      0x68,  0x84,  0xBC,  0x6C,  0x178, 0xDC,  0xC8, 0x78};
    std::copy(std::begin(sizes), std::end(sizes), std::begin(ent_size_lookup));
    entity_base_vtable[0] = reinterpret_cast<void *>(native_entity_destroy<entity_base>);
    entity_base_vtable[0x10 / 4] = reinterpret_cast<void *>(native_entity_release<entity_base>);
    entity_base_vtable[0x44 / 4] = reinterpret_cast<void *>(native_base_set_visible);
    entity_base_vtable[0x60 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_base_vtable[0x64 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_base_vtable[0x68 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_base_vtable[0x9C / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_base_vtable[0x90 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_base_vtable[0x108 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_base_vtable[0xC8 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_base_vtable[0xF0 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_base_vtable[0x4C / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_base_vtable[0x50 / 4] = reinterpret_cast<void *>(standalone_entity_true);
    entity_base_vtable[0x114 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_base_vtable[0x118 / 4] = reinterpret_cast<void *>(native_base_damage);
    entity_base_vtable[0x124 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_base_vtable[0x14C / 4] = reinterpret_cast<void *>(native_base_get_ifc_num);
    entity_base_vtable[0x150 / 4] = reinterpret_cast<void *>(native_base_set_ifc_num);
    entity_base_vtable[0x164 / 4] = reinterpret_cast<void *>(standalone_entity_base_unmash);
    std::copy(std::begin(entity_base_vtable), std::end(entity_base_vtable), std::begin(signaller_vtable));
    signaller_vtable[0] = reinterpret_cast<void *>(native_entity_destroy<signaller>);
    signaller_vtable[0x10 / 4] = reinterpret_cast<void *>(native_entity_release<signaller>);
    entity_vtables()[1] = reinterpret_cast<int>(signaller_vtable);
    entity_vtables()[0] = reinterpret_cast<int>(entity_base_vtable);
    std::copy(std::begin(entity_base_vtable), std::end(entity_base_vtable), std::begin(entity_vtable));
    entity_vtable[0] = reinterpret_cast<void *>(native_entity_destroy<entity>);
    entity_vtable[0x10 / 4] = reinterpret_cast<void *>(native_entity_release<entity>);
    entity_vtable[0x188 / 4] = reinterpret_cast<void *>(native_entity_family_visible);
    entity_vtable[0x28 / 4] = reinterpret_cast<void *>(native_entity_visual_radius);
    entity_vtable[0x2C / 4] = reinterpret_cast<void *>(native_entity_visual_center);
    entity_vtable[0x34 / 4] = reinterpret_cast<void *>(native_entity_po_changed);
    entity_vtable[0x38 / 4] = reinterpret_cast<void *>(native_entity_set_flag);
    entity_vtable[0x3C / 4] = reinterpret_cast<void *>(native_entity_set_ext_flag);
    entity_vtable[0x40 / 4] = reinterpret_cast<void *>(native_entity_set_active);
    entity_vtable[0x44 / 4] = reinterpret_cast<void *>(native_entity_set_visible);
    entity_vtable[0x54 / 4] = reinterpret_cast<void *>(native_entity_flavor);
    entity_vtable[0x60 / 4] = reinterpret_cast<void *>(standalone_entity_true);
    entity_vtable[0x74 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_vtable[0xD4 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_vtable[0x10C / 4] = reinterpret_cast<void *>(native_entity_has_time);
    entity_vtable[0x110 / 4] = reinterpret_cast<void *>(native_entity_time);
    entity_vtable[0x164 / 4] = reinterpret_cast<void *>(standalone_entity_unmash);
    entity_vtable[0x168 / 4] = reinterpret_cast<void *>(native_entity_clear_region);
    entity_vtable[0x16C / 4] = reinterpret_cast<void *>(native_entity_compute_sector);
    entity_vtable[0x184 / 4] = reinterpret_cast<void *>(native_entity_update_ai_proximity);
    entity_vtable[0x18C / 4] = reinterpret_cast<void *>(native_entity_renderable);
    entity_vtable[0x190 / 4] = reinterpret_cast<void *>(native_entity_possibly_collide);
    entity_vtable[0x1A0 / 4] = reinterpret_cast<void *>(native_entity_update_collision_region);
    entity_vtable[0x1A8 / 4] = reinterpret_cast<void *>(native_entity_visible);
    entity_vtable[0x1C4 / 4] = reinterpret_cast<void *>(native_entity_color);
    entity_vtable[0x1C8 / 4] = reinterpret_cast<void *>(native_entity_set_alpha);
    entity_vtable[0x1CC / 4] = reinterpret_cast<void *>(native_entity_alpha);
    entity_vtable[0x1D4 / 4] = reinterpret_cast<void *>(native_entity_scale);
    entity_vtable[0x204 / 4] = reinterpret_cast<void *>(native_entity_set_age);
    entity_vtable[0x208 / 4] = reinterpret_cast<void *>(native_entity_set_recursive_age);
    entity_vtables()[2] = reinterpret_cast<int>(entity_vtable);
    entity_vtables()[27] = reinterpret_cast<int>(ai_cover_marker::native_vtable(entity_vtable));
    std::copy(std::begin(entity_vtable), std::end(entity_vtable), std::begin(pfx_vtable));
    pfx_vtable[0x54 / 4] = reinterpret_cast<void *>(standalone_pfx_flavor);
    pfx_vtable[0x60 / 4] = reinterpret_cast<void *>(standalone_entity_true);
    pfx_vtable[0xD4 / 4] = reinterpret_cast<void *>(standalone_entity_true);
    pfx_vtable[0x164 / 4] = reinterpret_cast<void *>(standalone_pfx_unmash);
    native_pfx::install_entity_callbacks(pfx_vtable);
    entity_vtables()[23] = reinterpret_cast<int>(pfx_vtable);

    std::copy(std::begin(entity_vtable), std::end(entity_vtable), std::begin(light_source_vtable));
    light_source_vtable[0x60 / 4] = reinterpret_cast<void *>(standalone_entity_true);
    light_source_vtable[0x54 / 4] = reinterpret_cast<void *>(standalone_light_source_flavor);
    light_source_vtable[0x90 / 4] = reinterpret_cast<void *>(standalone_entity_true);
    light_source_vtable[0x164 / 4] = reinterpret_cast<void *>(standalone_light_source_unmash);
    light_source_vtable[0] = reinterpret_cast<void *>(native_entity_destroy<light_source>);
    entity_vtables()[14] = reinterpret_cast<int>(light_source_vtable);

    const auto initialize_marker_vtable = [&](void **vtable, void *flavor) {
        std::copy(std::begin(entity_vtable), std::end(entity_vtable), vtable);
        vtable[0x54 / 4] = flavor;
        vtable[0x60 / 4] = reinterpret_cast<void *>(standalone_entity_true);
        vtable[0x64 / 4] = reinterpret_cast<void *>(standalone_entity_false);
        vtable[0x164 / 4] = reinterpret_cast<void *>(standalone_entity_unmash);
    };
    initialize_marker_vtable(marker_vtables[0], reinterpret_cast<void *>(standalone_marker_flavor));
    initialize_marker_vtable(marker_vtables[1], reinterpret_cast<void *>(standalone_parking_marker_flavor));
    marker_vtables[1][0x9C / 4] = reinterpret_cast<void *>(standalone_entity_true);
    initialize_marker_vtable(marker_vtables[2], reinterpret_cast<void *>(standalone_water_exit_marker_flavor));
    initialize_marker_vtable(marker_vtables[3], reinterpret_cast<void *>(standalone_anchor_marker_flavor));
    std::copy(std::begin(marker_vtables[0]), std::end(marker_vtables[0]), std::begin(marker_vtables[5]));
    marker_vtables[5][0xB8 / 4] = reinterpret_cast<void *>(standalone_entity_true);
    marker_vtables[5][0x54 / 4] = reinterpret_cast<void *>(standalone_line_anchor_flavor);
    marker_vtables[5][0] = reinterpret_cast<void *>(native_line_anchor_destroy);
    entity_vtables()[16] = reinterpret_cast<int>(marker_vtables[0]);
    entity_vtables()[17] = reinterpret_cast<int>(marker_vtables[1]);
    entity_vtables()[18] = reinterpret_cast<int>(marker_vtables[2]);
    entity_vtables()[20] = reinterpret_cast<int>(marker_vtables[3]);
    entity_vtables()[21] = reinterpret_cast<int>(marker_vtables[5]);
    std::copy(std::begin(entity_vtable), std::end(entity_vtable), std::begin(actor_vtable));
    actor_vtable[0] = reinterpret_cast<void *>(native_entity_destroy<actor>);
    actor_vtable[0x10 / 4] = reinterpret_cast<void *>(native_entity_release<actor>);
    actor_vtable[0x54 / 4] = reinterpret_cast<void *>(standalone_actor_flavor);
    actor_vtable[0x60 / 4] = reinterpret_cast<void *>(standalone_entity_true);
    actor_vtable[0x64 / 4] = reinterpret_cast<void *>(standalone_actor_true);
    actor_vtable[0x48 / 4] = reinterpret_cast<void *>(standalone_actor_ai_core);
    actor_vtable[0x4C / 4] = reinterpret_cast<void *>(native_actor_hero);
    actor_vtable[0x50 / 4] = reinterpret_cast<void *>(native_actor_alive);
    actor_vtable[0x114 / 4] = reinterpret_cast<void *>(native_actor_has_damage);
    actor_vtable[0x118 / 4] = reinterpret_cast<void *>(native_actor_damage);
    actor_vtable[0x90 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    actor_vtable[0x108 / 4] = reinterpret_cast<void *>(native_actor_material_switching);
    actor_vtable[0x164 / 4] = reinterpret_cast<void *>(standalone_actor_unmash);
    actor_vtable[0x28 / 4] = reinterpret_cast<void *>(native_actor_visual_radius);
    actor_vtable[0x2C / 4] = reinterpret_cast<void *>(native_actor_visual_center);
    actor_vtable[0x124 / 4] = reinterpret_cast<void *>(native_actor_has_physical);
    actor_vtable[0x128 / 4] = reinterpret_cast<void *>(native_actor_physical);
    actor_vtable[0x18C / 4] = reinterpret_cast<void *>(native_entity_renderable);
    actor_vtable[0x1AC / 4] = reinterpret_cast<void *>(native_actor_render);
    actor_vtable[0x1B0 / 4] = reinterpret_cast<void *>(native_actor_mesh);
    actor_vtable[0x1B8 / 4] = reinterpret_cast<void *>(native_actor_suspend);
    actor_vtable[0x1BC / 4] = reinterpret_cast<void *>(native_actor_unsuspend);
    actor_vtable[0x1C0 / 4] = reinterpret_cast<void *>(native_actor_set_color);
    actor_vtable[0x1C8 / 4] = reinterpret_cast<void *>(native_actor_set_alpha);
    actor_vtable[0x1D0 / 4] = reinterpret_cast<void *>(native_actor_set_scale);
    actor_vtable[0x1C4 / 4] = reinterpret_cast<void *>(native_actor_color);
    actor_vtable[0x1CC / 4] = reinterpret_cast<void *>(native_actor_alpha);
    actor_vtable[0x1D4 / 4] = reinterpret_cast<void *>(native_actor_scale);
    actor_vtable[0x220 / 4] = reinterpret_cast<void *>(native_actor_floor);
    actor_vtable[0x254 / 4] = reinterpret_cast<void *>(native_actor_colgeom_radius);
    actor_vtable[0x258 / 4] = reinterpret_cast<void *>(native_actor_colgeom_center);
    actor_vtable[0x260 / 4] = reinterpret_cast<void *>(native_actor_morph);
    actor_vtable[0x14C / 4] = reinterpret_cast<void *>(native_actor_get_ifc_num);
    actor_vtable[0x150 / 4] = reinterpret_cast<void *>(native_actor_set_ifc_num);
    actor_vtable[0x264 / 4] = reinterpret_cast<void *>(native_actor_ifl_play);
    actor_vtable[0x268 / 4] = reinterpret_cast<void *>(native_actor_ifl_lock);
    actor_vtable[0x1F0 / 4] = reinterpret_cast<void *>(native_actor_set_collisions);
    actor_vtable[0x1E0 / 4] = reinterpret_cast<void *>(native_actor_light_set);
    actor_vtable[0x280 / 4] = reinterpret_cast<void *>(native_actor_frame_delta);
    actor_vtable[0x284 / 4] = reinterpret_cast<void *>(native_actor_invalidate_frame_delta);
    actor::install_inventory_callbacks(actor_vtable);
    entity_vtables()[3] = reinterpret_cast<int>(actor_vtable);
    entity_vtables()[4] = reinterpret_cast<int>(beam::native_vtable(entity_vtable));
    entity_vtables()[6] = reinterpret_cast<int>(conglomerate_clone::native_vtable(actor_vtable));
    entity_vtables()[7] = reinterpret_cast<int>(grenade::native_vtable(actor_vtable));
    std::copy(std::begin(actor_vtable), std::end(actor_vtable), std::begin(item_vtable));
    item_vtable[0] = reinterpret_cast<void *>(native_entity_destroy<item>);
    item_vtable[0x10 / 4] = reinterpret_cast<void *>(native_entity_release<item>);
    item_vtable[0x54 / 4] = reinterpret_cast<void *>(native_item_flavor);
    item_vtable[0x74 / 4] = reinterpret_cast<void *>(standalone_entity_true);
    item_vtable[0x164 / 4] = reinterpret_cast<void *>(native_item_unmash);
    item::install_weapon_callbacks(item_vtable);
    entity_vtables()[8] = reinterpret_cast<int>(item_vtable);
    auto **handheld_vtable = static_cast<void **>(handheld_item::native_vtable(item_vtable));
    entity_vtables()[9] = reinterpret_cast<int>(handheld_vtable);
    entity_vtables()[10] = reinterpret_cast<int>(gun::native_vtable(handheld_vtable));
    entity_vtables()[12] = reinterpret_cast<int>(thrown_item::native_vtable(handheld_vtable));
    entity_vtables()[11] = reinterpret_cast<int>(melee_item::native_vtable(handheld_vtable));
    entity_vtables()[26] = reinterpret_cast<int>(visual_item::native_vtable(actor_vtable));
    std::copy(std::begin(actor_vtable), std::end(actor_vtable), std::begin(conglomerate_vtable));


    conglomerate_vtable[0] = reinterpret_cast<void *>(native_entity_destroy<conglomerate>);
    conglomerate_vtable[0x10 / 4] = reinterpret_cast<void *>(native_entity_release<conglomerate>);
    conglomerate_vtable[0x190 / 4] = reinterpret_cast<void *>(native_conglomerate_possibly_collide);
    conglomerate_vtable[0x164 / 4] = reinterpret_cast<void *>(standalone_conglomerate_unmash);
    conglomerate_vtable[0x184 / 4] = reinterpret_cast<void *>(native_conglomerate_update_ai_proximity);
    conglomerate_vtable[0x1E0 / 4] = reinterpret_cast<void *>(native_conglomerate_light_set);
    conglomerate_vtable[0x208 / 4] = reinterpret_cast<void *>(native_conglomerate_set_recursive_age);
    conglomerate_vtable[0x54 / 4] = reinterpret_cast<void *>(standalone_conglomerate_flavor);
    conglomerate_vtable[0x12C / 4] = reinterpret_cast<void *>(standalone_entity_true);
    conglomerate_vtable[0x130 / 4] = reinterpret_cast<void *>(native_conglomerate_skeleton);
    conglomerate_vtable[0x28 / 4] = reinterpret_cast<void *>(native_conglomerate_visual_radius);
    conglomerate_vtable[0x18C / 4] = reinterpret_cast<void *>(native_conglomerate_renderable);
    conglomerate_vtable[0x1AC / 4] = reinterpret_cast<void *>(native_conglomerate_render);
    conglomerate_vtable[0x254 / 4] = reinterpret_cast<void *>(native_conglomerate_colgeom_radius);
    conglomerate_vtable[0x258 / 4] = reinterpret_cast<void *>(native_conglomerate_colgeom_center);
    conglomerate_vtable[0x260 / 4] = reinterpret_cast<void *>(native_conglomerate_morph);
    conglomerate_vtable[0x294 / 4] = reinterpret_cast<void *>(standalone_conglomerate_has_tentacle);
    conglomerate_vtable[0x298 / 4] = reinterpret_cast<void *>(standalone_conglomerate_tentacle);
    conglomerate_vtable[0x29C / 4] = reinterpret_cast<void *>(standalone_conglomerate_has_variant);
    conglomerate_vtable[0x2A0 / 4] = reinterpret_cast<void *>(standalone_conglomerate_variant);
    conglomerate_vtable[0x1C8 / 4] = reinterpret_cast<void *>(native_conglomerate_set_alpha);
    entity_vtables()[5] = reinterpret_cast<int>(conglomerate_vtable);
#else
    CDECL_CALL(0x004FE6A0);
#endif
}

bool mash_was_allocated(void *a1)
{
    auto *address = static_cast<uint8_t *>(a1);

    auto *header = bit_cast<generic_mash_header *>(address - sizeof(generic_mash_header));
    if (static_cast<int>(header->safety_key) != header->generate_safety_key()) {
        return true;
    }

    if (header->is_flagged(0x40000000) && header->class_id == 0xFFFF) {
        return true;
    }

    return !header->is_flagged(0x40000000) && header->class_id != 0xFFFF;
}

void release_generic_mash(void *a1)
{
    auto *address = static_cast<uint8_t *>(a1);

    auto *header = bit_cast<generic_mash_header *>(address - sizeof(generic_mash_header));

    assert((static_cast<int>(header->safety_key) == header->generate_safety_key()) && "Safety keys do not match!");

    assert(!mash_was_allocated(address) && "Mash appears to be a dynamically allocated clone...");

    assert(header->is_flagged(_MASH_FLAG_IN_USE) && "Uh-oh header is not in use!");

    header->field_4 &= 0x7FFFFFFFu;
}

entity_base *parse_entity_mash(_std::vector<entity *> *ent_vec_ptr, _std::vector<item *> *item_vec_ptr, void *a3,
                               const string_hash *a7, void *a8, bool a9)
{
    TRACE("parse_entity_mash");

    if constexpr (1) {
        assert(ent_vec_ptr != nullptr && "MUST specify an entity vector to push entities into");
        assert(item_vec_ptr != nullptr && "MUST specify an item vector to push entities into");

        construct_v_table_lookup();

        auto *header = static_cast<generic_mash_header *>(a3);

        if (!a9) {
            header->field_4 |= _MASH_FLAG_IN_USE;
        }

        entity_base *ent_ptr = nullptr;

        auto v6 = parse_generic_object_mash(ent_ptr,
                                            a3,
                                            const_cast<string_hash *>(a7),
                                            reinterpret_cast<unsigned int *>(entity_vtables()),
                                            reinterpret_cast<unsigned int *>(entity_sizes()),
                                            0x1Cu,
                                            4u,
                                            a8);
        assert(ent_ptr != nullptr);

        if (v6) {
            ent_ptr->field_8 |= 0x400u;
        } else {
            ent_ptr->field_8 &= ~0x400u;
        }

        return ent_ptr;
    } else {
        return (entity_base *)CDECL_CALL(0x004FF610, ent_vec_ptr, item_vec_ptr, a3, a7, a8, a9);
    }
}

void entity_mash_patch()
{
    if constexpr (1) {
        REDIRECT(0x0055A8D9, parse_entity_mash);
        REDIRECT(0x005E0AE9, parse_entity_mash);
    }
}
