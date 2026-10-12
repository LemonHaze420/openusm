#pragma once

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace sxcc {

using pc_pointer_t = std::uint32_t;
inline constexpr int pc_vm_stack_capacity = 384;

struct so_data_block {
    int32_t m_size;
    uint32_t buffer, flags;
};
constexpr uint32_t SO_DATA_BLOCK_FLAG_FROM_MASH = 1;
constexpr uint32_t SCRIPT_OBJECT_FLAG_GLOBAL = 1, SCRIPT_OBJECT_FLAG_EXTERNAL = 4;
constexpr uint32_t VM_EXECUTABLE_FLAG_STATIC = 1, VM_EXECUTABLE_FLAG_LINKED = 2, VM_EXECUTABLE_FLAG_FROM_MASH = 4,
                   VM_EXECUTABLE_FLAG_UN_MASHED = 8;
struct script_object {
    uint32_t name, parent, global_instance, debug_info;
    so_data_block static_data;
    int32_t data_blocksize;
    uint32_t funcs;
    int32_t total_funcs, field_28;
    uint32_t instances, flags;
};
struct vm_executable {
    uint32_t owner, fullname, name;
    int32_t parms_stacksize;
    uint32_t buffer;
    int32_t buffer_len;
    uint32_t debug_info, flags;
    int32_t field_20;
};
struct script_executable {
    char field_0[32];
    uint32_t sx_exe_image;
    int32_t sx_exe_image_size;
    uint32_t script_objects, script_objects_by_name;
    int32_t total_script_objects;
    uint32_t global_script_object, permanent_string_table;
    int32_t permanent_string_table_size;
    uint32_t system_string_table;
    int32_t system_string_table_size;
    uint32_t script_object_dummy_list, script_allocated_stuff_map, flags, field_54;
    int32_t field_58;
    struct info_t {
        uint32_t field_0, so_name, field_8;
        int32_t field_C, field_10, field_14, field_18;
    };
    static constexpr uint32_t SCRIPT_EXECUTABLE_FLAG_FROM_MASH = 2;
};
struct generic_mash_header {
    uint32_t safety_key;
    int32_t field_4, field_8;
    uint16_t class_id, field_E;
    uint32_t generate_safety_key() const
    {
        return ((uint32_t(field_8) + 0x7BADBA5D - (field_4 & 0xfffffff) + class_id + field_E) & 0xfffffff) | 0x70000000;
    }
};
struct script_var_container {
    struct script_var_to_addr_t {
        uint32_t name;
        int32_t address;
    };
    so_data_block script_var_block;
    uint32_t script_var_to_addr;
    int32_t field_10;
    uint32_t debug_info;
    int32_t flags;
};
struct vm_vector3d_t {
    float x, y, z;
};
constexpr uint32_t UNINITIALIZED_SCRIPT_PARM = 0x7BAD05CF;
inline uint32_t to_hash(const char *str)
{
    uint32_t h = 0;
    for (int c = *str; c; ++str, c = *str) {
        if (c >= 'A' && c <= 'Z')
            c += 32;
        h = c + 33 * h;
    }
    return h;
}

static_assert(sizeof(script_executable) == 0x5c);
static_assert(sizeof(script_object) == 0x34);
static_assert(sizeof(vm_executable) == 0x24);
static_assert(sizeof(generic_mash_header) == 0x10);
static_assert(sizeof(script_var_container) == 0x1c);

}
