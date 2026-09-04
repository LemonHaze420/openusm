#include "fe_menu_nav_bar.h"

#include "common.h"
#include "fetext.h"
#include "func_wrapper.h"
#include "game.h"
#include "localized_string_table.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(menu_nav_bar, 0x2Cu);

menu_nav_bar::menu_nav_bar() : field_4()
{
    this->text_box = nullptr;
    this->background_a = nullptr;
    this->field_1C = nullptr;
    this->field_20 = nullptr;
    this->field_24 = nullptr;
}

void menu_nav_bar::Load()
{
    TRACE("menu_nav_bar::Load");
    assert(text_box != nullptr && background_a != nullptr);

    THISCALL(0x00612080, this);
}

void menu_nav_bar::AddButtons(menu_nav_bar::button_type a2, menu_nav_bar::button_type a3, global_text_enum a4)
{
    TRACE("menu_nav_bar::AddButtons");

    if constexpr (1) {
        if (this->field_28) {
            mString v5{g_game_ptr->field_7C->lookup_localized_string(a4)};
            this->field_4 += v5 + "  ";
        } else {
            mString v8{g_game_ptr->field_7C->lookup_localized_string(a4)};
            this->field_4 += v8 + "    ";
        }
    } else {
        THISCALL(0x006121C0, this, a2, a3, a4);
    }
}

void menu_nav_bar::Reformat()
{
    TRACE("menu_nav_bar::Reformat");

    if constexpr (1) {
        FEText::string v3{this->field_4};
        this->text_box->SetTextNoLocalize(v3);
    } else {
        THISCALL(0x006122B0, this);
    }
}

void menu_nav_bar::Reset()
{
    static const char str[3]{0, 0, 0};
    this->field_4 = {str};

    this->field_28 = false;
}

void menu_nav_bar_patch()
{
    {
        FUNC_ADDRESS(address, &menu_nav_bar::Load);
        //REDIRECT(0x006431A4, address);
    }
}
