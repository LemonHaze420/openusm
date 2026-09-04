#include "game_data_essentials.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"

#include <cstdint>
#include <string.h>

VALIDATE_SIZE(game_data_essentials, 0x3C);

game_data_essentials::game_data_essentials()
{
    if constexpr (1) {
        this->field_2 = 1;
        this->field_4 = 1;
        this->field_6 = 0;
        this->field_8 = 0;
        this->field_A = 0;
        this->field_0 = 2000;
        *bit_cast<uint32_t *>(&this->field_0) = 0;
        *bit_cast<uint32_t *>(&this->field_4) = 0;
        *bit_cast<uint32_t *>(&this->field_8) = 0;
        this->field_10 = 0;
        this->field_C = 0;
        strncpy(this->field_14, "02:29:05", 25u);

        std::memset(this->field_2E, 0, sizeof(this->field_2E));
    } else {
        THISCALL(0x00580DD0, this);
    }
}
