#include "localized_string_table.h"

#include "femanager.h"
#include "fileusm.h"
#include "func_wrapper.h"
#include "game.h"
#include "os_developer_options.h"
#include "resource_directory.h"
#include "resource_manager.h"
#include "settings.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"

#include <cassert>

namespace {
constexpr int PC_GLOBAL_TEXT_COUNT = 478;
constexpr int XBOX_GLOBAL_TEXT_COUNT = 446;

int expected_global_text_count()
{
    return g_platform == NL_PLATFORM_XBOX ? XBOX_GLOBAL_TEXT_COUNT : PC_GLOBAL_TEXT_COUNT;
}

const char **localized_strings(localized_string_table *table)
{
    if (table == nullptr || table->field_0 == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<const char **>(table->field_0);
}

int localized_global_text_count(localized_string_table *table)
{
    if (table == nullptr) {
        return expected_global_text_count();
    }

    const int count = table->field_4 - table->scripttext_number;
    return count > 0 ? count : expected_global_text_count();
}

const char *localized_error_string(localized_string_table *table)
{
    const char **strings = localized_strings(table);
    if (strings != nullptr && table->field_4 > 0 && strings[0] != nullptr) {
        return strings[0];
    }

    return "";
}
}

void localized_string_table::load_localizer()
{
    TRACE("localized_string_table::load_localizer");

    if constexpr (1)
    {
        [[maybe_unused]] auto a3 = os_developer_options::instance->get_string(os_developer_options::strings_t::SKU);
        globalTextLanguage() = 0;

        switch (g_settings()->sub_81D010("Settings\\Language", 0)) {
        case 1:
            globalTextLanguage() = 1;
            break;
        case 2:
            globalTextLanguage() = 2;
            break;
        case 3:
            globalTextLanguage() = 3;
            break;
        case 4:
            globalTextLanguage() = 4;
            break;
        default:
            globalTextLanguage() = 0;
            break;
        }

        auto *my_partition = resource_manager::get_partition_pointer(RESOURCE_PARTITION_LANG);
        assert(my_partition != nullptr);
        assert(my_partition->get_pack_slots().size() == 1);

        resource_pack_slot *my_slot = my_partition->get_pack_slots().front();
        assert(my_slot != nullptr);

        auto *my_streamer = my_partition->get_streamer();
        assert(my_streamer != nullptr);

        static const char *globalTextLangFileNames[] { "globaltext_ENGLISH",
                                                        "globaltext_FRENCH",
                                                        "globaltext_GERMAN",
                                                        "globaltext_SPANISH",
                                                        "globaltext_ITALIAN" };

        const auto *textLangFileName = globalTextLangFileNames[globalTextLanguage()];

        my_streamer->load(textLangFileName, 0, nullptr, nullptr);
        my_streamer->flush(RenderLoadMeter);

        mString v5{textLangFileName};
        v5.append(g_platform == NL_PLATFORM_XBOX ? "_XBOX" : "_PS2");

        resource_key res_key = create_resource_key_from_path(v5.c_str(), RESOURCE_KEY_TYPE_LANGUAGE);
        localized_string_table *string_localizer =
            CAST(string_localizer, my_slot->get_resource(res_key, nullptr, nullptr));
        assert(string_localizer != nullptr);

        string_localizer->sub_60BD30();
        g_game_ptr->field_7C = string_localizer;
    }
    else
    {
        CDECL_CALL(0x0062EF10);
    }
}

void localized_string_table::sub_60BD30() {
    this->field_0 = (internal *) ((char *) this + (unsigned int) this->field_0);
    this->field_8 += (int) this;

    const int global_text_count = this->field_4 - this->scripttext_number;
    const int expected_count = expected_global_text_count();
    if (global_text_count < 0) {
        sp_log(
            "localized strings table has invalid counts: total=%d script=%d global=%d.",
            this->field_4,
            this->scripttext_number,
            global_text_count);
        assert(0);
        return;
    }

    if (global_text_count != expected_count) {
        sp_log(
            "localized strings table global count mismatch: expected=%d actual=%d total=%d script=%d.",
            expected_count,
            global_text_count,
            this->field_4,
            this->scripttext_number);

        if (g_platform != NL_PLATFORM_XBOX) {
            assert(0);
        }
    }

    const char **strings = localized_strings(this);
    assert(strings != nullptr);

    if (this->field_4 > 0) {
        for (int i = 0; i < this->field_4; ++i) {
            char DstBuf[4];
            itoa(i, DstBuf, 10);
            auto *v6 = get_msg(g_fileUSM(), DstBuf);
            if (v6 != nullptr) {
                strings[i] = v6;
            } else if (strings[i] != nullptr) {
                strings[i] += this->field_8;
            }

            auto v7 = (uint8_t *) strings[i];
            if (v7 != nullptr && *v7) {
                do {
                    if (*v7 == 160) {
                        *v7 = ' ';
                    }
                } while (*++v7);
            }
        }
    }
}

const char *localized_string_table::lookup_scripttext_string(int num) {
    if (num < 0 || num >= this->scripttext_number) {
        sp_log("localized scripttext lookup out of range: num=%d script_count=%d.", num, this->scripttext_number);
        assert(g_platform == NL_PLATFORM_XBOX);
        return localized_error_string(this);
    }

    const char **strings = localized_strings(this);
    assert(strings != nullptr);

    const int global_text_count = localized_global_text_count(this);
    auto *result = strings[global_text_count + num];

    //    sp_log("lookup_scripttext_string: %s %d", result, num);

    return result != nullptr ? result : localized_error_string(this);
}

const char *localized_string_table::lookup_localized_string(global_text_enum num)
{
    const int idx = static_cast<int>(num);
    const int global_text_count = localized_global_text_count(this);

    if (idx < 0 || idx >= global_text_count) {
        sp_log("localized global text lookup out of range: num=%d global_count=%d.", idx, global_text_count);
        assert(g_platform == NL_PLATFORM_XBOX);
        return localized_error_string(this);
    }

    const char **strings = localized_strings(this);
    assert(strings != nullptr);

    auto *result = strings[idx];

    return result != nullptr ? result : localized_error_string(this);
}

void localized_string_table_patch() {

    SET_JUMP(0x0062EF10, localized_string_table::load_localizer);

    {
        FUNC_ADDRESS(address, &localized_string_table::lookup_localized_string);
        SET_JUMP(0x0060BDC0, address);
    }

    {
        FUNC_ADDRESS(address, &localized_string_table::lookup_scripttext_string);
        SET_JUMP(0x0060BDD0, address);
    }

    //REDIRECT(0x006732E8, dialog_box_formatting);
}
