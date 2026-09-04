#include "ai_quad_path.h"

#include "ai_quad_path_cell.h"
#include "common.h"
#include "region.h"

#include <func_wrapper.h>

#include <cassert>

VALIDATE_SIZE(ai_quad_path, 0x34);

ai_quad_path::ai_quad_path() {}

void ai_quad_path::un_mash(void *buffer_ptr, region *reg, int a4, int a5, int a6)
{
    if constexpr (1) {
        this->field_18 = reg;
        if (reg->get_district_id() != this->field_1C) {
            auto &scene_id = reg->get_scene_id(false);

            auto *str = scene_id.c_str();
            auto district_id = this->field_18->get_district_id();

            sp_log("Path graph %02d/%02d erroneously thinks it is in district %d when it is in "
                   "district %d.\n"
                   "  This is an art bug in the quad paths in region %s.",
                   this->field_1C,
                   this->field_1E,
                   this->field_1C,
                   district_id,
                   str);

            assert(0);
        }

        this->field_24 = (ai_quad_path_cell *)((char *)this->field_24 + (uintptr_t)buffer_ptr + a4);
        this->field_20 += (int)buffer_ptr + a5;

        for (int i = 0; i < this->field_2A; ++i) {
            this->field_24[i].un_mash(buffer_ptr, a4, a6);
        }

        this->field_2C = 0;
        this->field_30 = 0;

    } else {
        THISCALL(0x00464FA0, this, buffer_ptr, reg, a4, a5, a6);
    }
}
