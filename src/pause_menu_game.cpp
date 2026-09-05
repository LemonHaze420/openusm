#include "pause_menu_game.h"

#include "common.h"
#include "game.h"
#include "localized_string_table.h"

VALIDATE_SIZE(pause_menu_game, 0x2B0u);

void pause_menu_game::initialize()
{
    auto *table = g_game_ptr->field_7C;
    field_0 = table->lookup_localized_string(static_cast<global_text_enum>(223));

    static constexpr int left_text_ids[21] = {
        224, 225, 226, 227,
        189, 190, 191, 192, 193, 194, 195, 196, 197,
        198, 199, 200, 201, 202, 203, 204, 205,
    };
    static constexpr int right_text_ids[21] = {
        230, 231, 232, 233,
        206, 207, 208, 209, 210, 211, 212, 213, 214,
        215, 216, 217, 218, 219, 220, 221, 222,
    };

    for (int i = 0; i < 21; ++i)
    {
        field_10[i] = table->lookup_localized_string(
            static_cast<global_text_enum>(left_text_ids[i]));
        field_160[i] = table->lookup_localized_string(
            static_cast<global_text_enum>(right_text_ids[i]));
    }
}
