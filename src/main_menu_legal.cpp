#include "main_menu_legal.h"

#include "common.h"
#include "frontendmenusystem.h"
#include "femultilinetext.h"
#include "multilinestring.h"
#include "resource_manager.h"

VALIDATE_SIZE(main_menu_legal, 0x38);

main_menu_legal::main_menu_legal(FEMenuSystem *a2, int a3, int a4)
    : FEMenu(a2, 0, a3, a4, 8, 0), field_2C(nullptr), field_30(0.0f), field_34(a2)
{
    m_vtbl = 0x00894598;
}

void main_menu_legal::Update(Float delta_time)
{
    field_30 += delta_time;
    if (field_30 >= 4.0f) {
        static_cast<FrontEndMenuSystem *>(field_34)->GoNextState();
    }
}

void main_menu_legal::Draw()
{
    if (field_2C != nullptr)
        field_2C->Draw();
}

void main_menu_legal::OnActivate()
{
    OnDeactivate();

    field_2C = new FEMultiLineText{
        static_cast<font_index>(1), 320.0f, 240.0f, 0, static_cast<panel_layer>(2), 1.2f, 0, 0, color32{}};
    field_2C->SetNumLines(20);
    field_2C->SetNoFlash(color32{0xFFC8C8C8u});
    field_2C->SetButtonColor(color32{0xFFFFFFFFu});
    field_2C->SetButtonScale(1.0f);
    resource_manager::push_resource_context(resource_manager::get_best_context(RESOURCE_PARTITION_LANG));

    field_2C->ReadFileBoxFormat("__BX_LEGAL__", 480, false);
    field_30 = 0.0f;
    resource_manager::pop_resource_context();
}

void main_menu_legal::OnDeactivate()
{
    if (field_2C != nullptr) {
        delete[] field_2C->lines;
        field_2C->lines = nullptr;
        delete field_2C;
        field_2C = nullptr;
    }
}
