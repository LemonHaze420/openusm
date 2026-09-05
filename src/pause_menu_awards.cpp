#include "pause_menu_awards.h"

#include "common.h"
#include "game.h"
#include "localized_string_table.h"

VALIDATE_SIZE(pause_menu_awards, 0x250u);

void pause_menu_awards::initialize()
{
    auto *table = g_game_ptr->field_7C;
    field_0 = table->lookup_localized_string(static_cast<global_text_enum>(151));
    field_10 = table->lookup_localized_string(static_cast<global_text_enum>(152));
    field_20 = table->lookup_localized_string(static_cast<global_text_enum>(153));

    for (int i = 0; i < 17; ++i)
    {
        field_30[i] = table->lookup_localized_string(static_cast<global_text_enum>(154 + i));
        field_140[i] = table->lookup_localized_string(static_cast<global_text_enum>(171 + i));
    }
}
