#include "smoke_test.h"

#include "common.h"
#include "func_wrapper.h"
#include "mstring.h"
#include "variable.h"
#include "app.h"
#include "os_developer_options.h"
#include "game.h"

#include <cassert>
#include <cstring>

VALIDATE_SIZE(smoke_test, 800u);

Var<smoke_test *> g_smoke_test{0x00960E0C};

smoke_test::smoke_test(const char **a2, Float a3)
{
    if constexpr (1) {
        auto *v4 = &this->field_10;
        this->field_0 = false;
        this->field_10 = {};
        memset(this->field_20, 0, sizeof(this->field_20));
        this->field_18 = 0;
        this->smoke_lvl_num = 0;
        this->field_0 = false;
        this->field_14 = -1;
        v4->reset();
        this->field_4 = 0.0;
        this->field_8 = 0.0;
        this->field_C = 0.0;

        auto v11 = 0u;
        for (auto i = 0u; a2[i] && a2[i + 1]; i += 2, ++v11) {
            this->field_20[v11].field_0 = new mString{a2[i]};
            this->field_20[v11].field_4 = a3;
        }

        this->smoke_lvl_num = v11;
        assert(smoke_lvl_num < SMOKE_LEVEL_MAX_NUM);

        if (this->smoke_lvl_num == 0) {
            this->field_0 = true;
        }
    } else {
        THISCALL(0x00578F40, this, a2, a3);
    }
}

void smoke_test::frame_advance()
{
    THISCALL(0x0057B740, this);
}

static Var<const char *[]> cheat_heroes{0x00922564};

void smoke_test::sub_57B610()
{
    mString a1 = *this->field_20[this->field_18].field_0;
    this->field_10.reset();
    this->field_C = 0.0;
    this->field_4 = 0.0;
    this->field_8 = 0.0;
    if (app::instance != nullptr) {
        auto *v2 = app::instance->m_game;
        if (strstr(this->field_20[this->field_18].field_0->c_str(), "M0_Arena") != nullptr ||
            os_developer_options::instance->get_int(mString{"SOAK_SMOKE"}) != 0) {
            if (!cheat_heroes()[this->field_14]) {
                this->field_14 = -1;
            }

            ++this->field_14;
            mString v5{cheat_heroes()[this->field_14]};
            os_developer_options::instance->set_string(os_developer_options::strings_t::HERO_NAME, v5);
        }

        if (v2 != nullptr) {
            v2->load_new_level(a1, -1);
        }
    }
}
