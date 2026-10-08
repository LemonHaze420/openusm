#include "femultilinetext.h"

#include "common.h"
#include "config.h"
#include "fetextflashinfo.h"
#include "filespec.h"
#include "func_wrapper.h"
#include "game.h"
#include "gamepadinput.h"
#include "localized_string_table.h"
#include "mash_config.h"
#include "log.h"
#include "ngl.h"
#include "ngl_font.h"
#include "multilinestring.h"
#include "resource_key.h"
#include "resource_manager.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"

#include <algorithm>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <new>
VALIDATE_OFFSET(FEMultiLineText, lines, 0x88);
VALIDATE_SIZE(FEMultiLineText, 0xA0u);

FEMultiLineText::FEMultiLineText() : FEText()
{
    m_vtbl = 0x0087AE58;
    field_68 = color32{0xFFFFFFFFu};
    field_6C = 0.0f;
    field_70 = 1.0f;
    field_74 = 0.0f;
    field_78 = 0.0f;
    field_7C = -1;
    field_80 = 0;
    line_avail_num = 0;
    lines = nullptr;
    field_8C = 0;
    field_90 = 0;
    field_94 = 0;
    field_98 = 0;
    field_9C = false;
    field_9D = false;
    field_9E = false;
}

FEMultiLineText::FEMultiLineText(font_index a2, Float a4, Float a5, int a6, panel_layer a7, Float a8, int a9, int a10,
                                 color32 a11)
    : FEText(a2, static_cast<global_text_enum>(0), a4, a5, a6, a7, a8, a9, a10, a11)
{
    m_vtbl = 0x0087AE58;
    field_68 = color32{0xFFFFFFFFu};
    field_6C = 0.0f;
    field_70 = 1.0f;
    field_74 = 0.0f;
    field_78 = 0.0f;
    field_7C = -1;
    field_80 = 0;
    line_avail_num = 0;
    lines = nullptr;
    field_8C = 0;
    field_90 = 0;
    field_94 = 0;
    field_98 = 0;
    field_9C = false;
    field_9D = false;
    field_9E = false;

    nglFont *font = nullptr;
    if (a2 != static_cast<font_index>(5) && a2 != static_cast<font_index>(6)) {
        font = g_femanager.GetFont(a2);
    }
    uint32_t width = 0;
    uint32_t height = 0;
    char sample[] = "!";
    nglGetStringDimensions(font, sample, &width, &height, 1.0f, 1.0f);
    field_74 = static_cast<float>(height);
    field_78 = static_cast<float>(height);
}

void FEMultiLineText::Draw()
{
    Draw(0, field_80);
}

void FEMultiLineText::_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("FEMultiLineText::unmash");
    FEText::_unmash(a1, a3);
    this->lines = nullptr;
    this->SetNumLines(this->line_avail_num);
}

int FEMultiLineText::_get_mash_sizeof()
{
#if OPENUSM_XBOX_MASH_FORMAT && !defined(OPENUSM_XBPACK_V10)
    return 0x98;
#else
    return 0xA0;
#endif
}

void FEMultiLineText::Draw(int a2, int a3)
{
    if (this->IsShown()) {
        if (a2 < 0) {
            a2 = 0;
        }

        if (a3 > this->field_80) {
            a3 = this->field_80;
        }

        auto v4 = this->field_64;
        auto v17 = this->field_4C;
        if ((v4 & 8) != 0) {
            v17 = this->flash_info->GetColor(this->field_4C);
        }

        v17.set_alpha(v17.get_alpha() * this->field_4);

        if ((v4 & 1) == 0) {
            v17.set_alpha(0xFFu);
            v17.set_blue(0xFFu);
            v17.set_green(0xFFu);
            v17.set_red(0xFFu);
        }

        auto v8 = color32::to_int(v17);

        auto v9 = color32::to_int(this->field_68);
        if (a2 < a3) {
            int i = a2;
            int a3a = a3 - a2;
            do {
                auto *v11 = &this->lines[i];
                if (v11->m_font_index != 6) {
                    if (v11->field_10 != "") {
                        auto v16 = this->field_78;
                        auto v15 = this->field_6C;
                        auto v14 = this->field_40;
                        auto v13 = this->field_3C;
                        auto z_value = this->GetZvalue();
                        v11->Draw(z_value, v8, v9, v13, v14, v15, v16);
                    }
                }
                ++i;
                --a3a;
            } while (a3a);
        }
    }
}

void FEMultiLineText::GetPos(Float &a2, Float &a3)
{
    a2 = this->field_34[0];
    a3 = this->field_34[1];
}

void FEMultiLineText::SetPos(Float x, Float y)
{
    field_34.x = x;
    field_34.y = y + flt_965BDC;
    AdjustForJustification();
}

void FEMultiLineText::SetLineSpacing(int height)
{
    if (height == -1) {
        auto *font = field_18 != static_cast<font_index>(5) && field_18 != static_cast<font_index>(6)
                         ? g_femanager.GetFont(field_18)
                         : nullptr;
        uint32_t width = 0;
        uint32_t measured_height = 0;
        char sample[] = "!";
        nglGetStringDimensions(font, sample, &width, &measured_height, 1.0f, 1.0f);
        field_74 = static_cast<float>(measured_height);
    } else {
        field_74 = static_cast<float>(height);
    }
    field_78 = field_74;
}

void FEMultiLineText::SetButtonColor(color32 a2)
{
    this->field_68 = a2;
}

mString FEMultiLineText::ReplaceEndlines(mString a2)
{
    for (auto i = a2.find("\\n", 0); i > 0; i = a2.find("\\n", i + 2)) {
        a2.data()[i] = ' ';
        a2.data()[i + 1] = '\n';
    }

    return a2;
}

void FEMultiLineText::SetButtonScale(Float a2)
{
    this->field_6C = a2;
}

void FEMultiLineText::SetTextBox(global_text_enum a2, int a3, Float a4)
{
    if constexpr (STANDALONE_SYSTEM) {
        mString text{g_game_ptr->field_7C->lookup_localized_string(a2)};
        SetTextBoxNoLocalize(string{text}, a3, a4);
    } else {
        THISCALL(0x00618070, this, a2, a3, a4);
    }
}

char *sub_609580(const char *a1, const char *a2, const char *a3)
{
    if constexpr (1) {
        auto *v3 = a1;
        auto v4 = strlen(a1);
        auto v12 = strlen(a2);
        auto *v5 = &a3[strlen(a3) + 1];
        uint32_t v11 = v5 - (a3 + 1);
        auto *result = static_cast<char *>(malloc(v4 * (v5 - a3) + 1));
        auto *v7 = result;
        auto *v13 = result;
        auto *v8 = result;
        if (result != nullptr) {
            result[0] = '\0';
            auto *v9 = strstr(a1, a2);
            if (v9 != nullptr) {
                do {
                    std::memcpy(v8, v3, v9 - v3);
                    auto *v10 = &v8[v9 - v3];

                    std::memcpy(v10, a3, v11);
                    v8 = &v10[v11];
                    v3 = &v9[v12];
                    v10[v11] = 0;
                    v9 = strstr(&v9[v12], a2);
                } while (v9);

                v7 = v13;
            }

            strcat(v8, v3);
            result = (char *)std::realloc(v7, strlen(v7) + 1);
        }

        sp_log("a1 = %s, a2 = %s, a3 = %s -> %s", a1, a2, a3, result);

        return result;

    } else {
        return (char *)CDECL_CALL(0x00609580, a1, a2, a3);
    }
}

void FEMultiLineText::sub_60A4A0(mString &text)
{
    if (std::strchr(text.c_str(), '~') == nullptr)
        return;

    char *value = static_cast<char *>(std::malloc(text.size() + 1));
    std::strcpy(value, text.c_str());
    const auto replace = [&value](const char *search, const char *token, const char *replacement) {
        if (std::strstr(value, search) != nullptr) {
            char *next = sub_609580(value, token, replacement);
            std::free(value);
            value = next;
        }
    };
    replace("~cross", "~cross", dword_965C24[GamepadInput::Cross]);
    replace("~triangle", "~triangle", dword_965C24[GamepadInput::Triangle]);
    replace("~square", "~square", dword_965C24[GamepadInput::Square]);
    replace("~circle", "~circle", dword_965C24[GamepadInput::Circle]);
    replace("~updown", "~updown", "\"UP & DOWN\"");
    replace("~l2", "~l2", dword_965C24[GamepadInput::L2]);
    replace("~r2", "~r2", dword_965C24[GamepadInput::R2]);
    replace("~r3", "~r3", dword_965C24[GamepadInput::R3]);
    if (std::strstr(value, "~both_lr") != nullptr) {
        char replacement[256];
        std::sprintf(replacement, "\"%s & %s\"", dword_965C24[GamepadInput::L2], dword_965C24[GamepadInput::R2]);
        replace("~both_lr", "~both_lr", replacement);
    }
    replace("~right", "~r2", dword_965C24[GamepadInput::Right]);
    replace("~select", "~select", dword_965C24[GamepadInput::Select]);
    replace("~forward", "~forward", dword_965C24[GamepadInput::Forward]);
    replace("~left", "~left", dword_965C24[GamepadInput::Left]);
    replace("~start", "~start", dword_965C24[GamepadInput::Start]);
    text.update_guts(value, -1);
    std::free(value);
}

namespace {

double box_word_width(const char *word, font_index font, Float scale, Float button_scale)
{
    if (*word == '\0')
        return 0.0;

    mString text{word};
    int start = 0;
    int end = text.find({start}, '~');
    if (end == -1)
        return MultiLineString::GetWidth(*bit_cast<MultiLineString::string *>(&text), scale, font);
    float width = 0.0f;
    for (; end != -1; end = text.find({start}, '~')) {
        mString segment = text.slice(start, end);
        width += MultiLineString::GetWidth(*bit_cast<MultiLineString::string *>(&segment), scale, font);
        const char *button_text = nullptr;
        start = end + MultiLineString::ConvertStringToButtonCode(text.c_str() + end, &button_text, text);
        mString button{button_text};
        width += MultiLineString::GetWidth(
            *bit_cast<MultiLineString::string *>(&button), button_scale, static_cast<font_index>(3));
    }
    mString remainder = text.slice(start, text.size());
    return width + MultiLineString::GetWidth(*bit_cast<MultiLineString::string *>(&remainder), scale, font);
}
}


int FEMultiLineText::MakeBox(char *text, int size, int box_width, Float scale_x, Float scale_y, bool store_lines)
{
    if constexpr (STANDALONE_SYSTEM) {
        if (field_18 == static_cast<font_index>(6))
            return 0;

        mString current_line{""};
        std::string word;
        int line_count = 0;
        float line_width = 0.0f;
        auto finish_line = [&]() {
            if (store_lines && line_count < line_avail_num) {
                lines[line_count].Set(
                    *bit_cast<MultiLineString::string *>(&current_line), field_18, field_3C, field_6C);
            }
            ++line_count;
        };

        const char *cursor = text;
        while (*cursor != '\0') {
            const auto length = std::strcspn(cursor, " -\n\r");
            word.assign(cursor, length);
            cursor += length;
            const char delimiter = *cursor;
            if (delimiter != '\0')
                ++cursor;
            const bool newline = delimiter == '\n';
            if (delimiter == '-' || delimiter == ' ')
                word.push_back(delimiter);

            if (word.size() >= 2 && word[0] == '\f' && word[1] == '[') {
                field_18 = static_cast<font_index>(std::atoi(word.c_str() + 2));
                word.erase(0, 4);
            }

            double word_width = box_word_width(word.c_str(), field_18, field_3C, field_6C);
            const double combined_width = line_width + word_width;
            if (combined_width > box_width) {
                if (newline) {
                    word.push_back(' ');
                    word_width = box_word_width(word.c_str(), field_18, field_3C, field_6C);
                }
                line_width = static_cast<float>(word_width);
                current_line.remove_surrounding_whitespace();
                finish_line();
                current_line = word.c_str();
            } else if (newline) {
                current_line += word.c_str();
                line_width = 0.0f;
                current_line.remove_surrounding_whitespace();
                finish_line();
                current_line = "";
            } else {
                current_line += word.c_str();
                line_width = static_cast<float>(combined_width);
            }

            if (*cursor == '\0') {
                current_line.remove_surrounding_whitespace();
                if (!current_line.empty())
                    finish_line();
            }
        }
        return line_count;
    } else {
        return THISCALL(0x0062EAD0, this, text, size, box_width, scale_x, scale_y, store_lines);
    }
}


void FEMultiLineText::ReadFileBoxFormat(const char *name, int box_width, bool flatten_newlines)
{
    if constexpr (STANDALONE_SYSTEM) {
        const filespec spec{mString{name}};
        const auto key = create_resource_key_from_path(spec.m_name.c_str(), RESOURCE_KEY_TYPE_TEXTFILE);
        field_7C = box_width;
        int size = 0;
        char *text = reinterpret_cast<char *>(resource_manager::get_resource(key, &size, nullptr));
        if (text == nullptr) {
            static std::vector<char> credits_text;
            static std::vector<char> legal_text;
            std::vector<char> *cached_text = nullptr;
            char filename[100];
            if (std::strcmp(name, "__BX_CREDITS__") == 0) {
                cached_text = &credits_text;
                std::strcpy(filename, "data\\packs\\bcfusm.dat");
            } else if (std::strcmp(name, "__BX_LEGAL__") == 0) {
                cached_text = &legal_text;
                char language = 'e';
                switch (globalTextLanguage) {
                case 1:
                    language = 'f';
                    break;
                case 2:
                    language = 'g';
                    break;
                case 3:
                    language = 's';
                    break;
                case 4:
                    language = 'i';
                    break;
                }
                std::sprintf(filename, "data\\packs\\blfusm%c.dat", language);
            } else {
                return;
            }

            if (cached_text->empty()) {
                auto *file = std::fopen(filename, "rb");
                if (file == nullptr)
                    return;
                std::fseek(file, 0, SEEK_END);
                const long file_size = std::ftell(file);
                std::fseek(file, 0, SEEK_SET);
                if (file_size < 0) {
                    std::fclose(file);
                    return;
                }
                cached_text->resize(static_cast<std::size_t>(file_size) + 1);
                std::fread(cached_text->data(), 1, static_cast<std::size_t>(file_size), file);
                std::fclose(file);
                std::size_t output = 0;
                for (std::size_t input = 0; input < static_cast<std::size_t>(file_size); ++input) {
                    const char value = (*cached_text)[input];
                    if (value != '\\') {
                        (*cached_text)[output++] = value;
                    } else {
                        const char escape = (*cached_text)[++input];
                        if (escape == '2')
                            (*cached_text)[output++] = '\2';
                        else if (escape == 'f')
                            (*cached_text)[output++] = '\f';
                    }
                }
                (*cached_text)[output] = '\0';
                cached_text->resize(output + 1);
            }
            text = cached_text->data();
        }

        if (flatten_newlines) {
            for (int i = 0; i < size; ++i) {
                if (text[i] == '\n')
                    text[i] = ' ';
            }
        }
        field_80 = MakeBox(text, size, box_width, field_3C, field_40, false);
        delete[] lines;
        line_avail_num = field_80;
        lines = new MultiLineString[line_avail_num];
        MakeBox(text, size, box_width, field_3C, field_40, true);
        AdjustForJustification();
    } else {
        THISCALL(0x00633DB0, this, name, box_width, flatten_newlines);
    }
}

bool FEMultiLineText::CheckIfNotTooLong(int a2)
{
    if (a2 < this->line_avail_num) {
        return true;
    }

    if (this->field_9E) {
        sp_log("MultiLineString is too long (cut off).  Number allocated lines: %d\n", this->line_avail_num);
        auto *v4 = this->lines->field_10.c_str();
        sp_log("Start of text: %s\n", v4);
    } else {
        sp_log("MultiLineString is too long (not cut off).  Number allocated lines: %d\n", this->line_avail_num);
        auto *v3 = this->lines->field_10.c_str();
        sp_log("Start of text: %s\n", v3);

        assert(0 && "MultiLineString is too long");
    }

    return false;
}

void FEMultiLineText::SetTextBoxNoLocalize(FEMultiLineText::string a2, int a7, Float a8)
{
    TRACE("FEMultiLineText::SetTextBoxNoLocalize");

    if (line_avail_num == 0) {
        sp_log("FEMultiLineText::SetTextBoxNoLocalize: no allocated lines");
        std::fflush(nullptr);
        std::exit(3);
    }

    if constexpr (STANDALONE_SYSTEM) {
        this->sub_60A4A0(*bit_cast<mString *>(&a2));
        auto v12 = *bit_cast<mString *>(&a2);
        auto v5 = FEMultiLineText::ReplaceEndlines(v12);
        a2 = *bit_cast<string *>(&v5);
        auto v6 = a7;
        const bool v7 = std::equal_to<float>{}(a8, -1.0f);
        auto v8 = this->field_3C;
        const auto scale_y = v7 ? field_40 : float(a8);
        this->field_7C = v6;
        auto a5 = v8;
        if (!v7) {
            a5 = a8;
        }

        auto v9 = this->MakeBox(a2.guts, a2.m_size, v6, a5, scale_y, true);
        this->field_80 = std::min(v9, this->line_avail_num);
        field_1C = lines[0].field_10;
        this->AdjustForJustification();
    } else {
        THISCALL(0x00633AB0, this, a2, a7, a8);
    }
}

void FEMultiLineText::SetTextAlloc(global_text_enum a2)
{
    sp_log("FEMultiLineText::SetTextAlloc: ");

    THISCALL(0x006180B0, this, a2);
}

void FEMultiLineText::SetText(global_text_enum a2)
{
    const char *localized = g_game_ptr->field_7C->lookup_localized_string(a2);
    if constexpr (STANDALONE_SYSTEM) {
        mString text{localized};
        _SetTextNoLocalize(*bit_cast<string *>(&text));
    } else {
        THISCALL(0x00618030, this, a2);
    }
}

void FEMultiLineText::AdjustForJustification()
{
    TRACE("FEMultiLineText::AdjustForJustification");
    if constexpr (STANDALONE_SYSTEM) {
        for (int i = 0; i < field_80; ++i) {
            auto &line = lines[i];
            if (GetFlag(0x10)) {
                line.field_4[0] = field_34[0];
            } else if (GetFlag(0x20)) {
                line.field_4[0] = field_34[0] - line.field_C;
            } else {
                line.field_4[0] = field_34[0] - line.field_C * 0.5f;
            }

            if (GetFlag(0x40)) {
                line.field_4[1] = field_34[1] + i * field_74;
            } else if (GetFlag(0x80)) {
                line.field_4[1] = field_34[1] + (i - field_80 + 1) * field_74 - field_74;
            } else {
                line.field_4[1] = field_34[1] + (i + 0.5f - field_80 * 0.5f) * field_74 - field_74 * 0.5f;
            }
        }
    } else {
        THISCALL(0x006182D0, this);
    }
}

void FEMultiLineText::_SetTextNoLocalize(FEMultiLineText::string a1)
{
    TRACE("FEMultiLineText::SetTextNoLocalize");
    if (line_avail_num == 0) {
        sp_log("FEMultiLineText::_SetTextNoLocalize: no allocated lines");
        std::fflush(nullptr);
        std::exit(3);
    }

    if constexpr (STANDALONE_SYSTEM) {
        mString text = *bit_cast<mString *>(&a1);
        sub_60A4A0(text);
        int count = 1;
        for (int i = 0; i < text.size(); ++i) {
            if (text.data()[i] == '\n')
                ++count;
        }
        field_80 = std::min(count, line_avail_num);
        const char *cursor = text.c_str();
        for (int i = 0; i < field_80; ++i) {
            const auto length = std::strcspn(cursor, "\n");
            char line_text[256];
            std::memcpy(line_text, cursor, length);
            line_text[length] = '\0';
            const mString line{line_text};
            lines[i].Set(bit_cast<MultiLineString::string>(string{line}), field_18, field_3C, field_6C);
            cursor += length + 1;
        }
        field_1C = lines[0].field_10;
        AdjustForJustification();
    } else {
        THISCALL(0x0062E720, this, a1);
    }
}

int FEMultiLineText::SetTextAllocNoLocalize(const char *a2, int a3)
{
    sp_log("FEMultiLineText::SetTextAllocNoLocalize:");
    return THISCALL(0x0062E8D0, this, a2, a3);
}

void FEMultiLineText::SetTextBoxAlloc(global_text_enum a1, int a3, Float a4)
{
    sp_log("FEMultiLineText::SetTextBoxAlloc:");

    THISCALL(0x00618140, this, a1, a3, a4);
}

void FEMultiLineText::SetTextBoxAllocNoLocalize(mString a2, int a6, Float a7)
{
    sp_log("FEMultiLineText::SetTextBoxAllocNoLocalize:");

    THISCALL(0x00633C00, this, a2, a6, a7);
}

void FEMultiLineText::SetNumLines(int n)
{
    TRACE("FEMultiLineText::SetNumLines", std::to_string(n).c_str());

    if (n <= 0) {
        sp_log("FEMultiLineText::SetNumLines: invalid line count %d", n);
        std::fflush(nullptr);
        std::exit(3);
    }

    if constexpr (STANDALONE_SYSTEM) {
        delete[] lines;
        line_avail_num = n;
        field_80 = 0;
        lines = new (std::nothrow) MultiLineString[n];
        if (lines == nullptr) {
            sp_log("FEMultiLineText::SetNumLines: allocation failed for %d lines", n);
            std::fflush(nullptr);
            std::exit(3);
        }
    } else {
        THISCALL(0x00617F30, this, n);
    }
}

void FEMultiLineText_patch()
{
    {
        FUNC_ADDRESS(address, &FEMultiLineText::_get_mash_sizeof);
        set_vfunc(0x0087AEA4, address);
    }

    {
        FUNC_ADDRESS(address, &FEMultiLineText::AdjustForJustification);
        REDIRECT(0x0062E878, address);
    }

    {
        FUNC_ADDRESS(address, &FEMultiLineText::_unmash);
        set_vfunc(0x0087AE5C, address);
    }

    {
        FUNC_ADDRESS(address, &FEMultiLineText::_SetTextNoLocalize);
        set_vfunc(0x0087AEE4, address);
    }


    {
        FUNC_ADDRESS(address, &FEMultiLineText::SetTextBoxNoLocalize);
        set_vfunc(0x0087AFA4, address);
    }

    return;

    SET_JUMP(0x00609580, sub_609580);

    if constexpr (0) {
        {
            FUNC_ADDRESS(address, &FEMultiLineText::SetText);
            //set_vfunc(0x0087AEE0, address);
        }

        auto addr = 0x0087AFA0;

        {
            FUNC_ADDRESS(address, &FEMultiLineText::SetTextBox);
            set_vfunc(addr, address);
        }

        addr += 0x4;

        addr += 0x4;

        {
            FUNC_ADDRESS(address, &FEMultiLineText::SetTextAlloc);
            set_vfunc(addr, address);
        }

        addr += 0x4;

        {
            FUNC_ADDRESS(address, &FEMultiLineText::SetTextAllocNoLocalize);
            set_vfunc(addr, address);
        }

        addr += 0x4;

        {
            FUNC_ADDRESS(address, &FEMultiLineText::SetTextBoxAlloc);
            set_vfunc(addr, address);
        }

        addr += 0x4;

        {
            FUNC_ADDRESS(address, &FEMultiLineText::SetTextBoxAllocNoLocalize);
            set_vfunc(addr, address);
        }

        {
            FUNC_ADDRESS(address, &MultiLineString::Draw);
            REDIRECT(0x00617E5F, address);
        }
    }
}
