#include "main_menu_legal.h"

#include "common.h"
#include "frontendmenusystem.h"
#include "femultilinetext.h"
#include "multilinestring.h"
#include "osassert.h"

#include <cstdio>
#include <vector>

VALIDATE_SIZE(main_menu_legal, 0x38);

main_menu_legal::main_menu_legal(FEMenuSystem *a2, int a3, int a4)
    : FEMenu(a2, 0, a3, a4, 8, 0),
      field_2C(nullptr),
      field_30(0.0f),
      field_34(a2)
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

    auto *file = std::fopen("data\\packs\\blfusm.dat", "rb");
    if (file == nullptr) {
        error("Unable to open legal text file data\\packs\\blfusm.dat.\n");
    }
    std::fseek(file, 0, SEEK_END);
    const long size = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);
    std::vector<char> text(static_cast<std::size_t>(size) + 1, '\0');
    const auto bytes_read = std::fread(text.data(), 1, text.size() - 1, file);
    std::fclose(file);
    if (bytes_read != text.size() - 1) {
        error("Unable to read legal text file data\\packs\\blfusm.dat.\n");
    }

    field_2C = new FEMultiLineText{
        static_cast<font_index>(1),
        320.0f,
        240.0f,
        0,
        static_cast<panel_layer>(2),
        1.2f,
        0,
        0,
        color32{}};
    field_2C->SetNumLines(20);
    field_2C->field_74 = 20.0f;
    field_2C->SetNoFlash(color32{0xFFC8C8C8u});
    field_2C->SetButtonScale(1.0f);
    field_2C->SetButtonColor(color32{0xFFFFFFFFu});

    mString legal_text{text.data()};
    field_2C->SetTextBoxNoLocalize(*bit_cast<FEMultiLineText::string *>(&legal_text), 480, -1.0f);
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
