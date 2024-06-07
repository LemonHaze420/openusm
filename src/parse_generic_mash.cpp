#include "parse_generic_mash.h"

#include "common.h"
#include "entity_base.h"
#include "entity_mash.h"
#include "log.h"
#include "memory.h"
#include "ngl.h"
#include "trace.h"
#include "parse_generic_mash.h"
#include "resource_directory.h"
#include "resource_pack_header.h"
#include "mission_table_container.h"

#include <cassert>
#include <cstring>

VALIDATE_SIZE(generic_mash_header, 0x10);

void *parse_generic_mash_init(generic_mash_header *&header,
                              void *a2,
                              bool *allocated_mem,
                              generic_mash_data_ptrs *a4,
                              uint32_t struct_size,
                              uint32_t *virtual_table_lookup,
                              uint32_t *size_table_lookup,
                              [[maybe_unused]] uint32_t num_table_entries,
                              [[maybe_unused]] uint32_t base_class_size,
                              void *a10) {
    assert(allocated_mem != nullptr);

    *allocated_mem = false;

    uint8_t *cur_ptr;

    auto *copy_a2 = static_cast<uint8_t *>(a2);

    //condition is false
    if (a10 != nullptr) {
        cur_ptr = copy_a2;
        header = static_cast<generic_mash_header *>(a10);
    } else if (header->is_flagged(_MASH_FLAG_IN_USE)) {
        //condition is false

        size_t v12 = header->field_8 - sizeof(generic_mash_header);
        auto *object_mash_data = static_cast<uint8_t *>(
            arch_memalign(sizeof(generic_mash_header), v12));
        assert(object_mash_data != nullptr && "Out of memory?  Prepare to crash.\"");

        memcpy(object_mash_data, copy_a2 + sizeof(generic_mash_header), v12);
        cur_ptr = object_mash_data;
        *allocated_mem = true;

    } else {
        cur_ptr = copy_a2 + sizeof(generic_mash_header);
        header->field_4 |= _MASH_FLAG_IN_USE;
        *allocated_mem = false;
    }

    sp_log("cur_ptr = 0x%08X", cur_ptr);
    assert(cur_ptr != nullptr);

    auto *addr = cur_ptr;
    uint8_t *v16;

    //condition is false
    if (header->is_flagged(0x40000000))
    {
        assert(base_class_size >= 4);
        assert(header->class_id < num_table_entries);
        assert(num_table_entries > 0);
        assert(size_table_lookup != nullptr);
        assert(virtual_table_lookup != nullptr);

        v16 = cur_ptr + size_table_lookup[header->class_id];

        //sp_log("%d %d %d %d", addr[0], addr[1], addr[2], addr[3]);
        assert(addr[0] == MASH_V_TABLE_VAL[0] ||
               addr[0] == ((char *) &virtual_table_lookup[header->class_id])[0]);

        assert(addr[1] == MASH_V_TABLE_VAL[1] ||
               addr[1] == ((char *) &virtual_table_lookup[header->class_id])[1]);

        assert(addr[2] == MASH_V_TABLE_VAL[2] ||
               addr[2] == ((char *) &virtual_table_lookup[header->class_id])[2]);

        assert(addr[3] == MASH_V_TABLE_VAL[3] ||
               addr[3] == ((char *) &virtual_table_lookup[header->class_id])[3]);

        std::memcpy(addr, &virtual_table_lookup[header->class_id], 4);

    }
    else
    {
        assert(base_class_size == 0);
        assert(num_table_entries == 0);
        assert(size_table_lookup == nullptr);
        assert(virtual_table_lookup == nullptr);

        v16 = cur_ptr + struct_size;
    }

    *a4 = generic_mash_data_ptrs {v16, header->get_mash_data()};
    return addr;
}

template<>
bool parse_generic_object_mash(mission_table_container *&a1,
                            void *a2,
                            [[maybe_unused]] void *a3,
                            unsigned int *a4,
                            unsigned int *a5,
                            uint32_t a6,
                            uint32_t a7,
                            void *a8)
{
    TRACE("parse_generic_object_mash");

    bool allocated_mem = false;
    generic_mash_data_ptrs a4a;

    auto *header = static_cast<generic_mash_header *>(a2);
    auto *v8 = static_cast<mission_table_container *>(parse_generic_mash_init(
                                    header,
                                    a2,
                                    &allocated_mem,
                                    &a4a,
                                    0x48u,
                                    a4,
                                    a5,
                                    static_cast<uint32_t>(a6),
                                    a7,
                                    a8));
    a1 = v8;
    s_current_un_mashing_mission_table_container() = v8;
    v8->un_mash(header, v8, v8, &a4a);
    s_current_un_mashing_mission_table_container() = nullptr;
    return allocated_mem;
}

template<>
bool parse_generic_object_mash(resource_directory *&arg0,
                               void *a1,
                               void *a5,
                               uint32_t *a6,
                               uint32_t *a7,
                               uint32_t a8,
                               uint32_t a9,
                               void *a10) {
    bool allocated_mem = false;

    generic_mash_data_ptrs a4;

    auto *header = static_cast<generic_mash_header *>(a1);

    auto *dir = static_cast<resource_directory *>(parse_generic_mash_init(
        header, a1, &allocated_mem, &a4, sizeof(resource_directory), a6, a7, static_cast<uint32_t>(a8), a9, a10));

    generic_mash_header *v10 = static_cast<generic_mash_header *>(a1);
    arg0 = dir;
    dir->un_mash_start(v10, dir, &a4, a5);

    return allocated_mem;
}

#if 0
template<>
bool parse_generic_object_mash(
    nglMeshFile **arg0, void *a1, void *a5, uint32_t *a6, uint32_t *a7, int a8, int a9, int *a10) {
    bool allocated_mem = false;

    generic_mash_data_ptrs a4;
    auto header = bit_cast<generic_mash_header *>(a1);
    nglMeshFile *v4 = (nglMeshFile *) parse_generic_mash_init(
        header, a1, &allocated_mem, &a4, 328, a6, a7, static_cast<uint32_t>(a8), a9, a10);

    void *v9 = CAST(v9, a5);
    generic_mash_header *v10 = CAST(v10, a1);
    *arg0 = v4;
    v4->un_mash_start(v10, v4, &a4, nullptr);

    return allocated_mem;
}
#endif

//0x00509D70
#if 0
template<>
bool parse_generic_object_mash<entity_base>(
    entity_base **arg0, void *a1, int a5, uint32_t *a6, uint32_t *a7, int a8, int a9, int *a10) {
    bool allocated_mem = false;

    generic_mash_data_ptrs a4;
    auto header = bit_cast<generic_mash_header *>(a1);
    auto *v4 = static_cast<entity_base *>(parse_generic_mash_init(header,
                                                                  a1,
                                                                  &allocated_mem,
                                                                  &a4,
                                                                  sizeof(entity_base),
                                                                  a6,
                                                                  a7,
                                                                  static_cast<uint32_t>(a8),
                                                                  a9,
                                                                  a10));

    void *v9 = CAST(v9, a5);
    generic_mash_header *v10 = CAST(v10, a1);
    *arg0 = v4;
    v4->un_mash_start(v10, v4, &a4, nullptr);

    return allocated_mem;
}
#endif
