#include "entity_mash.h"

#include "entity_base.h"
#include "func_wrapper.h"
#include "parse_generic_mash.h"
#include "trace.h"
#include "utility.h"

#include <cassert>

Var<int[28]> ent_v_table_lookup{0x0095A5F0};
Var<int[28]> ent_size_lookup{0x0095A2A0};

std::array<int, 11> &ifc_v_table_lookup = var<std::array<int, 11>>(0x0095A66C);

void fix_entity_v_table(char *addr, eEntityMashTypeEnum type)
{
    assert(addr[0] == ((const char *)&MASH_V_TABLE_VAL)[0] || addr[0] == ((char *)&ent_v_table_lookup()[type])[0]);

    assert(addr[1] == ((const char *)&MASH_V_TABLE_VAL)[1] || addr[1] == ((char *)&ent_v_table_lookup()[type])[1]);

    assert(addr[2] == ((const char *)&MASH_V_TABLE_VAL)[2] || addr[2] == ((char *)&ent_v_table_lookup()[type])[2]);

    assert(addr[3] == ((const char *)&MASH_V_TABLE_VAL)[3] || addr[3] == ((char *)&ent_v_table_lookup()[type])[3]);

    std::memcpy(addr, &ent_v_table_lookup()[type], 4);
}

void fix_ifc_v_table(char *addr, eEntityMashIFCTypeEnum ifc_type)
{
    assert(addr[0] == ((const char *)&MASH_V_TABLE_VAL)[0] || addr[0] == ((char *)&ifc_v_table_lookup[ifc_type])[0]);

    assert(addr[1] == ((const char *)&MASH_V_TABLE_VAL)[1] || addr[1] == ((char *)&ifc_v_table_lookup[ifc_type])[1]);

    assert(addr[2] == ((const char *)&MASH_V_TABLE_VAL)[2] || addr[2] == ((char *)&ifc_v_table_lookup[ifc_type])[2]);

    assert(addr[3] == ((const char *)&MASH_V_TABLE_VAL)[3] || addr[3] == ((char *)&ifc_v_table_lookup[ifc_type])[3]);

    std::memcpy(addr, &ifc_v_table_lookup[ifc_type], 4);
}

void construct_v_table_lookup()
{
    CDECL_CALL(0x004FE6A0);
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

#if 1
        bool (*func)(
            entity_base **, void *, const string_hash *, uint32_t *, uint32_t *, unsigned int, unsigned int, void *) =
            CAST(func, 0x00509D70);
        auto v6 = func(
            &ent_ptr, a3, a7, (unsigned int *)ent_v_table_lookup(), (unsigned int *)ent_size_lookup(), 0x1Cu, 4u, a8);
#else
        auto v6 = parse_generic_object_mash(ent_ptr,
                                            a3,
                                            (void *)a7,
                                            (unsigned int *)ent_v_table_lookup(),
                                            (unsigned int *)ent_size_lookup(),
                                            0x1Cu,
                                            4u,
                                            a8);
#endif
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
