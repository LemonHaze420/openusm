#include "fe_menu_nav_bar.h"

#include "common.h"
#include "fetext.h"
#include "game.h"
#include "localized_string_table.h"
#include "panelquad.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(menu_nav_bar, 0x2Cu);

menu_nav_bar::menu_nav_bar() : field_4() {
    this->text_box = nullptr;
    this->background_a = nullptr;
    this->field_1C = nullptr;
    this->field_20 = nullptr;
    this->field_24 = nullptr;
}

void menu_nav_bar::Load()
{
    TRACE("menu_nav_bar::Load");
    if (text_box == nullptr || background_a == nullptr) {
        sp_log("menu_nav_bar::Load: required panel objects were not loaded");
        std::fflush(nullptr);
        std::exit(3);
    }

    text_box->SetShown(true);
    background_a->TurnOn(true);
    if (field_1C != nullptr)
        field_1C->TurnOn(true);
    if (field_20 != nullptr)
        field_20->SetShown(true);
    if (field_24 != nullptr)
        field_24->SetShown(true);

    text_box->SetNoFlash(color32 {0xFFC8C8C8});
    text_box->SetFlash(
        color32 {0xFFC8C8C8},
        color32 {0xFFFFFFFF},
        Float {1.0f});
    field_28 = false;
}

void menu_nav_bar::AddButtons(menu_nav_bar::button_type a2,
                              menu_nav_bar::button_type a3,
                              global_text_enum a4)
{
    if constexpr (STANDALONE_SYSTEM)
    {
        mString label{g_game_ptr->field_7C->lookup_localized_string(a4)};
        this->field_4 += label + " ";
    }
    else
    {
        THISCALL(0x006121C0, this, a2, a3, a4);
    }
}

void menu_nav_bar::Reformat()
{
    if constexpr (STANDALONE_SYSTEM)
    {
        FEText::string text {this->field_4};
        this->text_box->SetTextNoLocalize(text);
    }
    else
    {
        THISCALL(0x006122B0, this);
    }
}

void menu_nav_bar_patch()
{
    {
        FUNC_ADDRESS(address, &menu_nav_bar::Load);
        //REDIRECT(0x006431A4, address);
    }
}
