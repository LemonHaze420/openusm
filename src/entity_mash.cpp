#include "entity_mash.h"

#include "entity_base.h"
#include "entity.h"
#include "conglom.h"
#include "light_source.h"
#include "effect_mash_layout.h"
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
    if ( type >= 27 )
        return type - 1;
#endif
    return type;
}

uint32_t entity_mash_size(uint16_t type)
{
#ifdef OPENUSM_XBPACK_V10
    static constexpr uint16_t v10_sizes[] = {
        0x44,  0x48,  0x68,  0xBC,  0xE8,  0x12C, 0xCC,  0x1A0,
        0xFC,  0x110, 0x328, 0x148, 0x340, 0x274, 0x6C,  0x158,
        0x68,  0x68,  0x68,  0x70,  0x68,  0x84,  0xBC,  0x6C,
        0x150, 0xDC,  0xD8,  0xC4,  0x78,
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

static void __fastcall standalone_actor_ifl_lock(actor *, void *, int)
{
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
static void __fastcall standalone_light_source_unmash(
    light_source *self,
    void *,
    generic_mash_header *header,
    void *object,
    generic_mash_data_ptrs *data)
{
    self->_un_mash(header, object, data);
}

static variant_interface *__fastcall standalone_conglomerate_variant(conglomerate *self)
{
    return self->m_variant_interface;
}

static void __fastcall standalone_entity_base_unmash(entity_base *self,
                                                     void *,
                                                     generic_mash_header *header,
                                                     void *object,
                                                     generic_mash_data_ptrs *data)
{
    self->entity_base::_un_mash(header, object, data);
}

static void __fastcall standalone_entity_unmash(entity *self,
                                                void *,
                                                generic_mash_header *header,
                                                void *object,
                                                generic_mash_data_ptrs *data)
{
    self->entity::un_mash(header, object, data);
}
static void __fastcall standalone_actor_unmash(actor *self,
                                               void *,
                                               generic_mash_header *header,
                                               void *object,
                                               generic_mash_data_ptrs *data)
{
    self->actor::_un_mash(header, object, data);
}

static void __fastcall standalone_conglomerate_unmash(conglomerate *self,
                                                      void *,
                                                      generic_mash_header *header,
                                                      void *object,
                                                      generic_mash_data_ptrs *data)
{
    self->conglomerate::_un_mash(header, object, data);
}
static int __fastcall standalone_pfx_flavor(entity_base *)
{
    return PFX;
}

static uint32_t standalone_effect_u32(const uint8_t *ptr)
{
    uint32_t value;
    std::memcpy(&value, ptr, sizeof(value));
    return value;
}

static void standalone_effect_align(uint8_t *&ptr, uintptr_t alignment)
{
    const auto remainder = reinterpret_cast<uintptr_t>(ptr) % alignment;
    if (remainder != 0)
        ptr += alignment - remainder;
}

static uint8_t *standalone_effect_record(uint8_t *&ptr, size_t size, size_t alignment)
{
    if (alignment != 0) {
        standalone_effect_align(ptr, alignment);
    } else {
        auto *end = ptr;
        while (*end == effect_mash::alignment_marker)
            ++end;
        ptr += static_cast<size_t>(end - ptr) & ~size_t{3};
    }

    auto *result = ptr;
    ptr += size;
    return result;
}

static std::pair<uint8_t *, uint32_t> standalone_effect_pointers(uint8_t *vector,
                                                                uint8_t *&stream)
{
    if (standalone_effect_u32(vector + offsetof(effect_mash::Vector, data)) == 0)
        return {nullptr, 0};

    const auto count =
        standalone_effect_u32(vector + offsetof(effect_mash::Vector, count));
    standalone_effect_align(stream, 4);
    auto *entries = standalone_effect_record(
        stream, static_cast<size_t>(count) * sizeof(uint32_t), 4);
    return {entries, count};
}

static void standalone_walk_aps_curve(uint8_t *&stream, unsigned depth)
{
    assert(depth <= 64);
    auto *object = standalone_effect_record(stream, sizeof(uint32_t), 0);
    const auto type = standalone_effect_u32(object);
    assert(type >= 8 && type < effect_mash::aps_sizes.size());
    standalone_effect_record(
        stream, effect_mash::aps_sizes[type] - sizeof(uint32_t), 1);
    if (type < 23)
        return;

    standalone_effect_pointers(
        object + offsetof(effect_mash::Curve, samples), stream);
    const auto [values, count] = standalone_effect_pointers(
        object + offsetof(effect_mash::Curve, values), stream);
    for (uint32_t i = 0; i < count; ++i) {
        if (standalone_effect_u32(values + i * sizeof(uint32_t)) != 0)
            standalone_walk_aps_curve(stream, depth + 1);
    }
}

static void standalone_walk_aps_template(uint8_t *&shared)
{
    auto *object =
        standalone_effect_record(shared, sizeof(effect_mash::EffectTemplate), 16);
    const auto [particles, particle_count] = standalone_effect_pointers(
        object + offsetof(effect_mash::EffectTemplate, particles), shared);
    std::vector<bool> textured(particle_count, false);

    for (uint32_t i = 0; i < particle_count; ++i) {
        if (standalone_effect_u32(particles + i * sizeof(uint32_t)) == 0)
            continue;

        auto *particle = standalone_effect_record(
            shared, sizeof(effect_mash::ParticleTemplate), 4);
        if (standalone_effect_u32(
                particle + offsetof(effect_mash::ParticleTemplate, graphics)) != 0) {
            textured[i] = true;
            auto *graphics =
                standalone_effect_record(shared, sizeof(uint32_t), 0);
            const auto type = standalone_effect_u32(graphics);
            assert(type < 8);
            standalone_effect_record(
                shared, effect_mash::aps_sizes[type] - sizeof(uint32_t), 1);
        }

        if (standalone_effect_u32(
                particle + offsetof(effect_mash::ParticleTemplate, curves)) != 0) {
            auto *curves = standalone_effect_record(
                shared, sizeof(effect_mash::Vector), 4);
            const auto [entries, count] = standalone_effect_pointers(curves, shared);
            for (uint32_t j = 0; j < count; ++j) {
                if (standalone_effect_u32(entries + j * sizeof(uint32_t)) != 0)
                    standalone_walk_aps_curve(shared, 0);
            }
        }
    }

    const auto [auxiliary, auxiliary_count] = standalone_effect_pointers(
        object + offsetof(effect_mash::EffectTemplate, auxiliaries), shared);
    for (uint32_t i = 0; i < auxiliary_count; ++i) {
        if (standalone_effect_u32(auxiliary + i * sizeof(uint32_t)) != 0)
            standalone_effect_record(
                shared, sizeof(effect_mash::Auxiliary), 4);
    }

    standalone_effect_align(shared, 4);
    for (const bool has_texture : textured) {
        if (has_texture && *shared != '\0') {
            while (*shared++ != '\0') {
            }
        }
    }
    standalone_effect_align(shared, 16);
}

static void standalone_walk_particle_instance(generic_mash_data_ptrs *data)
{
    auto *&normal = data->field_0;
    auto *&shared = data->field_4;
    const bool mismatch = (reinterpret_cast<uintptr_t>(normal) & 15u) == 8u;
    standalone_effect_align(normal, 16);
    if (mismatch) {
        standalone_effect_record(normal, 80, 1);
        shared += 8;
        standalone_effect_align(shared, 8);
        shared += 8;
    }

    auto *instance = standalone_effect_record(
        normal, sizeof(effect_mash::ParticleInstance), 0);
    assert(standalone_effect_u32(instance) == effect_mash::particle_instance_type);
    const auto [points, count] = standalone_effect_pointers(
        instance + offsetof(effect_mash::ParticleInstance, points), normal);
    (void)points;
    standalone_effect_align(normal, 4);
    standalone_effect_record(
        normal, static_cast<size_t>(count) * sizeof(effect_mash::Point), 4);
    standalone_effect_align(normal, 16);
    standalone_walk_aps_template(shared);
}

static void __fastcall standalone_pfx_unmash(entity *self,
                                             void *,
                                             generic_mash_header *header,
                                             void *object,
                                             generic_mash_data_ptrs *data)
{
    self->entity::un_mash(header, object, data);
    standalone_walk_particle_instance(data);
    *reinterpret_cast<void **>(reinterpret_cast<uint8_t *>(self) + 0x68) = nullptr;
}

void construct_v_table_lookup()
{
#if STANDALONE_SYSTEM
    static bool initialized;
    static void *marker_vtables[6][192]{};
    static void *entity_base_vtable[192]{};
    static void *pfx_vtable[192]{};
    static void *light_source_vtable[192]{};
    static void *actor_vtable[192]{};
    static void *conglomerate_vtable[192]{};
    if (initialized)
        return;
    initialized = true;

    static constexpr int sizes[28] = {
        0x44, 0x48, 0x68, 0xC0, 0xE8, 0x130, 0xD0, 0x1A4, 0x100, 0x114, 0x374, 0x14C, 0x350, 0x274,
        0x6C, 0x15C, 0x68, 0x68, 0x68, 0x70, 0x68, 0x84, 0xBC, 0x6C, 0x178, 0xDC, 0xC8, 0x78};
    std::copy(std::begin(sizes), std::end(sizes), std::begin(ent_size_lookup));
    entity_base_vtable[0x60 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_base_vtable[0x64 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_base_vtable[0x90 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_base_vtable[0x108 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    entity_base_vtable[0x164 / 4] = reinterpret_cast<void *>(standalone_entity_base_unmash);
    entity_vtables()[0] = reinterpret_cast<int>(entity_base_vtable);
    std::copy(std::begin(entity_base_vtable), std::end(entity_base_vtable),
              std::begin(pfx_vtable));
    pfx_vtable[0x54 / 4] = reinterpret_cast<void *>(standalone_pfx_flavor);
    pfx_vtable[0x60 / 4] = reinterpret_cast<void *>(standalone_entity_true);
    pfx_vtable[0x164 / 4] = reinterpret_cast<void *>(standalone_pfx_unmash);
    entity_vtables()[23] = reinterpret_cast<int>(pfx_vtable);

    std::copy(std::begin(entity_base_vtable), std::end(entity_base_vtable),
              std::begin(light_source_vtable));
    light_source_vtable[0x60 / 4] = reinterpret_cast<void *>(standalone_entity_true);
    light_source_vtable[0x54 / 4] =
        reinterpret_cast<void *>(standalone_light_source_flavor);
    light_source_vtable[0x90 / 4] = reinterpret_cast<void *>(standalone_entity_true);
    light_source_vtable[0x164 / 4] =
        reinterpret_cast<void *>(standalone_light_source_unmash);
    entity_vtables()[14] = reinterpret_cast<int>(light_source_vtable);

    const auto initialize_marker_vtable = [&](void **vtable, void *flavor) {
        std::copy(std::begin(entity_base_vtable), std::end(entity_base_vtable), vtable);
        vtable[0x54 / 4] = flavor;
        vtable[0x60 / 4] = reinterpret_cast<void *>(standalone_entity_true);
        vtable[0x64 / 4] = reinterpret_cast<void *>(standalone_entity_false);
        vtable[0x164 / 4] = reinterpret_cast<void *>(standalone_entity_unmash);
    };
    initialize_marker_vtable(marker_vtables[0], reinterpret_cast<void *>(standalone_marker_flavor));
    initialize_marker_vtable(marker_vtables[1], reinterpret_cast<void *>(standalone_parking_marker_flavor));
    initialize_marker_vtable(marker_vtables[2], reinterpret_cast<void *>(standalone_water_exit_marker_flavor));
    initialize_marker_vtable(marker_vtables[3], reinterpret_cast<void *>(standalone_anchor_marker_flavor));
    std::copy(std::begin(marker_vtables[0]), std::end(marker_vtables[0]),
              std::begin(marker_vtables[5]));
    marker_vtables[5][0xB8 / 4] = reinterpret_cast<void *>(standalone_entity_true);
    marker_vtables[5][0x54 / 4] =
        reinterpret_cast<void *>(standalone_line_anchor_flavor);
    entity_vtables()[16] = reinterpret_cast<int>(marker_vtables[0]);
    entity_vtables()[17] = reinterpret_cast<int>(marker_vtables[1]);
    entity_vtables()[18] = reinterpret_cast<int>(marker_vtables[2]);
    entity_vtables()[20] = reinterpret_cast<int>(marker_vtables[3]);
    entity_vtables()[21] = reinterpret_cast<int>(marker_vtables[5]);
    actor_vtable[0x54 / 4] = reinterpret_cast<void *>(standalone_actor_flavor);
    actor_vtable[0x60 / 4] = reinterpret_cast<void *>(standalone_entity_true);
    actor_vtable[0x64 / 4] = reinterpret_cast<void *>(standalone_actor_true);
    actor_vtable[0x48 / 4] = reinterpret_cast<void *>(standalone_actor_ai_core);
    actor_vtable[0x90 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    actor_vtable[0x108 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    actor_vtable[0x190 / 4] = reinterpret_cast<void *>(standalone_entity_false);
    actor_vtable[0x164 / 4] = reinterpret_cast<void *>(standalone_actor_unmash);
    entity_vtables()[3] = reinterpret_cast<int>(actor_vtable);
    std::copy(std::begin(actor_vtable), std::end(actor_vtable), std::begin(conglomerate_vtable));
    conglomerate_vtable[0x164 / 4] = reinterpret_cast<void *>(standalone_conglomerate_unmash);
    conglomerate_vtable[0x54 / 4] =
        reinterpret_cast<void *>(standalone_conglomerate_flavor);
    conglomerate_vtable[0x12C / 4] = reinterpret_cast<void *>(standalone_entity_true);
    conglomerate_vtable[0x268 / 4] = reinterpret_cast<void *>(standalone_actor_ifl_lock);
    conglomerate_vtable[0x294 / 4] =
        reinterpret_cast<void *>(standalone_conglomerate_has_tentacle);
    conglomerate_vtable[0x298 / 4] =
        reinterpret_cast<void *>(standalone_conglomerate_tentacle);
    conglomerate_vtable[0x29C / 4] =
        reinterpret_cast<void *>(standalone_conglomerate_has_variant);
    conglomerate_vtable[0x2A0 / 4] =
        reinterpret_cast<void *>(standalone_conglomerate_variant);
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
        return (entity_base *) CDECL_CALL(0x004FF610, ent_vec_ptr, item_vec_ptr, a3, a7, a8, a9);
    }
}

void entity_mash_patch()
{
    if constexpr (1) {
        REDIRECT(0x0055A8D9, parse_entity_mash);
        REDIRECT(0x005E0AE9, parse_entity_mash);
    }
}
