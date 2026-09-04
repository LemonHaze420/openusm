#include "terrain_types_manager.h"

#include "func_wrapper.h"
#include "variable.h"
#include "variables.h"
#include "resource_key.h"
#include "resource_manager.h"
#include "trace.h"

int &s_num_terrain_types = var<int>(0x00968508);

void terrain_types_manager::delete_inst()
{
    s_num_terrain_types = 0;
}

constexpr auto MAX_OBB_TERRAIN_TYPES = 16;

Var<fixedstring<8> *> s_terrain_types_string_table{0x0096850C};

Var<string_hash[256]> s_terrain_types{0x00969FF8};

void terrain_types_manager::create_inst()
{
    TRACE("terrain_types_manager::create_inst");

    if constexpr (1) {
        s_num_terrain_types = 0;
        if (!g_is_the_packer) {
            resource_key a1{string_hash{"TERRAIN_TYPES"}, (resource_key_type)35};
            auto *ptr = (int *)resource_manager::get_resource(a1, nullptr, nullptr);
            assert(ptr != nullptr);

            auto num_terrain_types_in_file = *ptr;
            assert(num_terrain_types_in_file < MAX_OBB_TERRAIN_TYPES);

            s_terrain_types_string_table() = bit_cast<fixedstring<8> *>(ptr + 8);
            if (num_terrain_types_in_file > 0) {
                auto v2 = 0;
                auto v3 = num_terrain_types_in_file;

                do {
                    string_hash v9{s_terrain_types_string_table()[v2].to_string()};
                    auto v4 = s_num_terrain_types;
                    auto v5 = 0;
                    if (s_num_terrain_types <= 0) {
                    LABEL_7:
                        s_terrain_types()[s_num_terrain_types] = v9;
                        s_num_terrain_types = v4 + 1;
                    } else {
                        while (s_terrain_types()[v5] != v9) {
                            if (++v5 >= s_num_terrain_types) {
                                goto LABEL_7;
                            }
                        }
                    }

                    v2 += 8;
                    --v3;
                } while (v3);
            }
        }
    } else {
        CDECL_CALL(0x005C54B0);
    }
}
