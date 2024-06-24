#include "multilinestring.h"

#include "common.h"
#include "femanager.h"
#include "func_wrapper.h"
#include "log.h"
#include "ngl.h"
#include "utility.h"
#include "trace.h"
#include "variables.h"

#include <cassert>

VALIDATE_SIZE(MultiLineString, 0x28);

MultiLineString::MultiLineString() {
    THISCALL(0x00609AA0, this);
}

void MultiLineString::Set(MultiLineString::string a2, font_index a7, Float a8, Float a9) {
    TRACE("MultiLineString::Set");

    if constexpr (0) {
        this->field_10 = *bit_cast<mString *>(&a2);
        auto *v6 = &this->field_10;

        this->m_font_index = a7;

        int v8 = 0;

        this->field_4 = {0, 0};
        this->field_C = 0.0;
        this->button_array_size = 0;
        if (!this->field_10.empty()) {
            do {
                if (v8 < 0) {
                    break;
                }
                v8 = v6->find({v8}, '~');
                if (v8 >= 0) {
                    this->button_array_size += 2;
                    ++v8;
                }

            } while (this->field_10.m_size);
        }

        if (this->button_array != nullptr) {
            operator delete[](this->button_array);
            this->button_array = nullptr;
        }

        auto v9 = this->button_array_size;
        if (v9 <= 0) {
            auto *v12 = this->field_10.c_str();

            mString v13{v12};

            MultiLineString::string temp;
            std::memcpy(&temp, &v13, sizeof(v13));

            this->field_C = MultiLineString::GetWidth(temp, a8, this->m_font_index);
        } else {
            auto *array = new button_t[v9];

            this->button_array = array;
            this->ParseForButtons(a8, a9);
        }
    } else {
        THISCALL(0x0062E5E0, this, a2, a7, a8, a9);
    }
}

bool sub_609B80(const char *a1, const char *a2, const char **a3, const char *a4, int *a5) {
    int v5 = strlen(a2);
    auto v6 = 0;
    if (v5 > 0)
    {
        auto *v7 = a1;
        do {
            auto v8 = *v7;
            auto v9 = v7[a2 - a1];
            if (*v7 == v9) {
                if (!v8 && !v9) {
                    break;
                }

            } else if (v8 < 'A' || v8 > 'Z' || v9 != v8 + 32) {
                return false;
            }

            ++v6;
            ++v7;
        } while (v6 < v5);
    }

    *a3 = a4;
    *a5 = v5;
    return true;
}

int MultiLineString::ConvertStringToButtonCode(const char *a1, const char **a2, const mString &a3) {
    if (a1[0] != '~') {
        return 0;
    }

    *a2 = "";

    const char *v6;
    const char *v5;

    switch (globalTextLanguage()) {
    case 1:
        v6 = ":";
        v5 = "9";
        break;
    case 3:
        v6 = "<";
        v5 = ";";
        break;
    case 4:
        v6 = ">";
        v5 = "=";
        break;
    default:
        v6 = "&";
        v5 = "!";
        break;
    }

    int len;

    if (sub_609B80(a1, "~cross", a2, "\"", &len))
        return len;
    if (sub_609B80(a1, "~circle", a2, "%", &len))
        return len;
    if (sub_609B80(a1, "~square", a2, "#", &len))
        return len;
    if (sub_609B80(a1, "~triangle", a2, "$", &len))
        return len;
    if (sub_609B80(a1, "~attack", a2, "#", &len))
        return len;
    if (sub_609B80(a1, "~jump", a2, "\"", &len))
        return len;
    if (sub_609B80(a1, "~web", a2, "$", &len))
        return len;
    if (sub_609B80(a1, "~counter", a2, "%", &len))
        return len;
    if (sub_609B80(a1, "~gc_z", a2, "(", &len))
        return len;
    if (sub_609B80(a1, "~l1", a2, "(", &len))
        return len;
    if (sub_609B80(a1, "~r1", a2, "'", &len))
        return len;
    if (sub_609B80(a1, "~r2", a2, v5, &len))
        return len;
    if (sub_609B80(a1, "~l2", a2, v6, &len))
        return len;

    if (sub_609B80(a1, "~swing", a2, "!", &len)) {
        return len;
    }

    if (sub_609B80(a1, "~modifier", a2, "&", &len)) {
        return len;
    }

    if (sub_609B80(a1, "~reflexes", a2, "(", &len))
        return len;
    if (sub_609B80(a1, "~recenter", a2, "'", &len))
        return len;
    if (sub_609B80(a1, "~right", a2, "*", &len))
        return len;
    if (sub_609B80(a1, "~back", a2, ",", &len))
        return len;
    if (sub_609B80(a1, "~forward", a2, ")", &len))
        return len;
    if (sub_609B80(a1, "~left", a2, "+", &len))
        return len;
    if (sub_609B80(a1, "~start", a2, "-", &len))
        return len;
    if (sub_609B80(a1, "~select", a2, "1", &len))
        return len;
    if (sub_609B80(a1, "~updown", a2, "/", &len))
        return len;
    if (sub_609B80(a1, "~both_lr", a2, ".", &len))
        return len;
    if (sub_609B80(a1, "~navig_all", a2, "0", &len))
        return len;
    if (sub_609B80(a1, "~stick_left", a2, "2", &len))
        return len;
    if (sub_609B80(a1, "~stick_right", a2, "3", &len))
        return len;
    if (sub_609B80(a1, "~click_left_stick", a2, "4", &len))
        return len;
    if (sub_609B80(a1, "~click_right_stick", a2, "5", &len))
        return len;
    if (sub_609B80(a1, "~registered", a2, "6", &len))
        return len;
    if (sub_609B80(a1, "~copy_right", a2, "7", &len))
        return len;

    if (!sub_609B80(a1, "~at_character", a2, "8", &len)) {
        auto *v4 = a3.c_str();
        sp_log("Incorrect button code %s found in %s\n", a1, v4);

        assert(0);
    }

    return len;
}

void MultiLineString::Draw(Float a2, int a3, int a4, Float a5, Float a6, Float a7, Float a8)
{
    //sp_log("%s", this->field_10.c_str());

    if constexpr (1)
    {
        int v9 = 0;
        if (this->button_array_size <= 0) {
            auto *v26 = this->field_10.c_str();

            nglFont *v27 = g_femanager.GetFont(this->m_font_index);

            nglListAddString(v27, v26, this->field_4[0], this->field_4[1], a2, a3, a5, a6);
        } else {
            auto *v10 = this->field_10.slice(0, this->button_array[0].field_0).c_str();

            nglFont *v13 = g_femanager.GetFont(this->m_font_index);

            nglListAddString(v13, v10, this->field_4[0], this->field_4[1], a2, a3, a5, a6);

            auto v15 = this->button_array_size;
            if ((v15 + 1) / 2 > 0) {
                int a5a = 0;
                do {
                    int v16;
                    if (a5a == v15 - 1) {
                        v16 = this->field_10.size();
                    } else {
                        v16 = this->button_array[2 * v9 + 1].field_0;
                    }

                    auto v17 = this->field_10.slice(this->button_array[2 * v9].field_0, v16);
                    auto v18 = this->field_4[1] - a8;
                    auto *v19 = v17.c_str();
                    auto v31 = this->button_array[2 * v9].field_2;

                    auto a4a = v18;
                    auto a3a = (double) v31 + this->field_4[0];

                    auto *font = g_femanager.GetFont(static_cast<font_index>(3));

                    nglListAddString(font, v19, a3a, a4a, a2, a4, a7, a7);

                    auto v20 = this->button_array_size;
                    if (a5a < v20 - 1) {
                        int v21;
                        if (a5a == v20 - 2) {
                            v21 = this->field_10.size();
                        } else {
                            v21 = this->button_array[2 * v9 + 2].field_0;
                        }

                        auto *v22 = this->field_10
                                        .slice(this->button_array[2 * v9 + 1].field_0, v21)
                                        .c_str();

                        font = g_femanager.GetFont(this->m_font_index);

                        auto a3b = (double) this->button_array[2 * v9 + 1].field_2 +
                            this->field_4[0];
                        nglListAddString(font, v22, a3b, this->field_4[1], a2, a3, a5, a6);
                    }

                    v15 = this->button_array_size;
                    a5a += 2;
                    ++v9;
                } while (v9 < (v15 + 1) / 2);
            }
        }
    } else {
        THISCALL(0x00617960, this, a2, a3, a4, a5, a6, a7, a8);
    }
}

double MultiLineString::GetWidth(MultiLineString::string a1, Float a5, font_index a3) {
    TRACE("MultiLineString::GetWidth", a1.guts);

    if constexpr (0)
    {
        int v3 = a3;
        nglFont *v4 = nullptr;

        float result;

        if (v3 == 6) {
            result = 0.0f;
        } else {
            if (v3 != 5) {
                v4 = g_femanager.field_4[v3];
            }

            auto str = *bit_cast<mString *>(&a1);

            int tmp;
            uint32_t a4;
            nglGetStringDimensions(v4, bit_cast<char *>(str.c_str()), (uint32_t *) &tmp, &a4, a5, a5);

            if (v3 == 3 && (str == "6" || str == "7" || str == "8")) {
                float v5 = (double) tmp;
                if (tmp < 0) {
                    v5 += 4.2949673e9;
                }

                tmp = v5 * 0.60000002f;
            }

            float v6 = (double) tmp;
            if (tmp < 0) {
                v6 += 4.2949673e9;
            }

            result = v6;
        }
        return result;

    } else {

        float (*func)(string, Float, font_index) = CAST(func, 0x00617BF0);
        float result = func(a1, a5, a3);
        return result;
    }
}

int MultiLineString::ParseForButtons(Float a3, Float a4) {
    TRACE("MultiLineString::ParseForButtons");

    assert(button_array != nullptr && button_array_size > 0);

    return THISCALL(0x00628ED0, this, a3, a4);
}

void MultiLineString_patch() {
    FUNC_ADDRESS(address, &MultiLineString::Set);
    REDIRECT(0x0062E809, address);
    //REDIRECT(0x0062EA41, address);
    //REDIRECT(0x0062ECFC, address);
    //REDIRECT(0x0062ED98, address);

    {
        FUNC_ADDRESS(address, &MultiLineString::ParseForButtons);
        REDIRECT(0x0062E6B0, address);
    }

    REDIRECT(0x0062E6D1, MultiLineString::GetWidth);
}
