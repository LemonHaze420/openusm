#include "game_data_essentials.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"

#include <string.h>

VALIDATE_SIZE(game_data_essentials, 0x3C);
VALIDATE_SIZE(game_save_timestamp, 0xCu);
VALIDATE_OFFSET(game_data_essentials, timestamp, 0x0);

game_data_essentials::game_data_essentials()
{
    if constexpr (1) {
        this->timestamp = {};
        this->field_10 = 0;
        this->field_C = 0;
        strncpy(this->field_14, "02:29:05", 25u);

        std::memset(this->field_2E, 0, sizeof(this->field_2E));
    } else {
        THISCALL(0x00580DD0, this);
    }
}
