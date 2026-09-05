#include "game_button.h"

#include "common.h"
#include "func_wrapper.h"
#include "input_mgr.h"
#include "memory.h"
#include "slab_allocator.h"
#include "trace.h"
#include "utility.h"
#include "vector3d.h"

#include <cassert>

VALIDATE_SIZE(game_button, 0x34u);

game_button::game_button()
{
    if constexpr (STANDALONE_SYSTEM) {
        this->field_4 = input_mgr::instance->field_58;
        this->field_C = nullptr;
        this->field_10 = nullptr;
        this->m_flags = 0;
        this->m_trigger_type = 4;
        this->field_2C = 0.0;
        this->set_control(static_cast<game_control_t>(0));
        this->clear();
    } else {
        void(__fastcall * func)(game_button *) = CAST(func, 0x0048D9A0);
        func(this);
    }
}

game_button::game_button(game_control_t a2)
{
    this->field_4 = input_mgr::instance->field_58;
    this->field_C = nullptr;
    this->field_10 = nullptr;
    this->m_flags = 0;
    this->m_trigger_type = 4;
    this->field_2C = 0.0;
    this->set_control(a2);
    this->clear();
}

game_button::game_button(const game_button &a2)
{
    this->field_4 = input_mgr::instance->field_58;
    this->field_C = nullptr;
    this->field_10 = nullptr;
    (*this) = a2;
}

game_button::game_button(const game_button &a2, const game_button &a3, int a4)
{
    this->field_4 = input_mgr::instance->field_58;
    this->field_8 = static_cast<game_control_t>(0);
    this->field_C = nullptr;
    this->field_10 = nullptr;
    this->m_flags = 0;
    this->m_trigger_type = a4;
    this->field_2C = 0.0;
    this->set_primary(a2);
    this->set_modifier(a3);
    this->clear();
}

game_button &game_button::operator=(const game_button &a2)
{
    if (this->field_C != nullptr) {
        delete this->field_C;
        this->field_C = nullptr;
    }

    if (this->field_10 != nullptr) {
        delete this->field_10;
        this->field_10 = nullptr;
    }

    this->m_trigger_type = a2.m_trigger_type;
    this->field_4 = a2.field_4;
    this->field_8 = a2.field_8;
    this->field_30 = a2.field_30;
    this->m_flags = a2.m_flags;
    this->field_14 = a2.field_14;
    this->field_18 = a2.field_18;
    this->field_1C = a2.field_1C;
    this->field_20 = a2.field_20;
    this->field_24 = a2.field_24;
    this->field_2C = a2.field_2C;
    this->field_28 = a2.field_28;

    if (a2.field_C != nullptr) {
        this->set_primary(*a2.field_C);
    }

    if (auto *v5 = a2.field_10; v5 != nullptr) {
        this->set_modifier(*v5);
    }

    return (*this);
}

void game_button::override(Float a2, Float a3, Float a4)
{
    if constexpr (0) {
        bool v9 = this->m_flags & GBFLAG_PRESSED;
        if (this->field_2C > EPSILON) {
            if (std::abs(a3) < 0.5f) {
                this->field_28 = this->field_2C;
            } else if (this->field_28 > 0.0f) {
                auto v4 = this->field_28 - a2;
                this->field_28 = v4;
                if (v4 > 0.0f) {
                    a3 = 0.0;
                }

                a4 = a3 - this->field_18;
            }
        }

        this->field_18 = a3;
        this->field_14 = a4;

        auto v5 = ((this->m_flags & GBFLAG_PRESSED) != 0 && (this->m_flags & GBFLAG_TRIGGERED) != 0 &&
                   this->field_14 <= -0.5f);

        this->set_flag(0x10, v5);

        this->set_flag(GBFLAG_PRESSED, std::abs(this->field_18) >= 0.5f);
        this->set_flag(GBFLAG_TRIGGERED, this->field_14 >= 0.5f);
        this->set_flag(GBFLAG_RELEASED, this->field_14 < -0.5f);

        if ((this->m_flags & GBFLAG_PRESSED) != 0 && v9) {
            this->field_1C += a2;
        } else if (!v9) {
            this->field_1C = 0.0;
        }

        assert(!(this->m_flags & GBFLAG_PRESSED && this->m_flags & GBFLAG_RELEASED));

        assert(!(this->m_flags & GBFLAG_TRIGGERED && this->m_flags & GBFLAG_RELEASED));

        if (this->field_30 > 0) {
            this->field_20 += a2;
            if (this->field_20 > this->field_24) {
                this->field_30 = 0;
                this->field_20 = 0.0;
            }
        }

        if ((this->m_flags & GBFLAG_PRESSED) != 0 && !v9) {
            if (this->field_20 <= this->field_24) {
                ++this->field_30;
            }

            this->field_20 = 0.0;
        }
    } else {
        THISCALL(0x0050B640, this, a2, a3, a4);
    }
}

float game_button::sub_55ED50() const
{
    if (this->is_flagged(0x20)) {
        return 0.0f;
    }

    return this->field_14;
}

float game_button::sub_55ED30() const
{
    if (this->is_flagged(0x20)) {
        return 0.0f;
    }

    return this->field_18;
}

void game_button::update(Float a2)
{
    TRACE("game_button::update");

    if constexpr (0) {
        auto *v3 = this->field_C;
        if (v3 == nullptr) {
            auto *inst = input_mgr::instance;
            auto v37 = inst->get_control_state(this->field_8, this->field_4);
            auto v35 = inst->get_control_delta(this->field_8, this->field_4);
            this->override(a2, v37, v35);
            return;
        }

        v3->update(a2);

        auto *v6 = this->field_10;
        if (v6 != nullptr) {
            if (this->is_flagged(0x8)) {
                auto *v9 = this->field_C;
                if (v9->is_pressed()) {
                    this->override(a2, 0.0, 0.0);
                    return;
                }

                this->set_flag(0x8, false);
            }

            v6->update(a2);

            switch (this->m_trigger_type) {
            case 0: {
                auto *v10 = this->field_10;
                auto *v11 = this->field_C;
                auto a2c = v10->sub_55ED30();
                auto v12 = v11->sub_55ED30();
                auto *v13 = v10;
                if (v12 >= a2c) {
                    v13 = v11;
                }

                float a2b = v13->sub_55ED30();
                if (!v11->is_pressed() || !v10->is_pressed()) {
                    a2b = 0.0;
                }

                this->override(a2, a2b, a2b - this->field_18);
                break;
            }
            case 1: {
                auto *v14 = this->field_10;
                auto *v15 = this->field_C;
                auto a2d = v14->sub_55ED30();
                auto v16 = v15->sub_55ED30();
                auto *v17 = v14;
                if (v16 >= a2d) {
                    v17 = v15;
                }

                float a2b = v17->sub_55ED30();
                if (!v15->is_pressed() && !v14->is_pressed()) {
                    a2b = 0.0;
                }

                this->override(a2, a2b, a2b - this->field_18);
                break;
            }
            case 2: {
                auto *v18 = this->field_10;
                auto *v19 = this->field_C;
                auto a2e = v18->sub_55ED30();
                auto v20 = v19->sub_55ED30();
                auto *v21 = v18;
                if (v20 >= a2e) {
                    v21 = v19;
                }

                float a2b = v21->sub_55ED30();
                if ((v19->is_pressed() && v18->is_pressed()) || (!v19->is_pressed() && !v18->is_pressed())) {
                    a2b = 0.0;
                }

                this->override(a2, a2b, a2b - this->field_18);
                break;
            }
            case 3: {
                auto *v22 = this->field_10;
                auto *v23 = this->field_C;
                auto a2f = v22->sub_55ED30();
                auto v24 = v23->sub_55ED30();
                auto *v25 = v22;
                if (v24 <= a2f) {
                    v25 = v23;
                }

                float a2b = 0.f - v25->sub_55ED30();
                if (v23->is_pressed() || v22->is_pressed()) {
                    a2b = 0.0;
                }

                this->override(a2, a2b, a2b - this->field_18);
                break;
            }
            case 4: {
                auto *v27 = this->field_10;
                if (!v27->is_pressed()) {
                    auto v33 = -this->field_18;
                    this->override(a2, 0.0, v33);
                    return;
                }

                game_button *v28;

                if (v27->is_triggered() && (v28 = this->field_C, v28->is_pressed()) && !v28->is_triggered()) {
                    this->set_flag(0x8, true);
                    this->override(a2, 0.0, 0.0);
                } else {
                    auto *v29 = this->field_C;
                    auto v34 = v29->sub_55ED50();
                    auto v30 = v29->sub_55ED30();
                    this->override(a2, v30, v34);
                }
                break;
            }
            case 5: {
                if (this->field_10->is_pressed()) {
                    auto v33 = -this->field_18;
                    this->override(a2, 0.0, v33);
                } else {
                    auto v26 = this->field_C->sub_55ED30();

                    auto v32 = v26 - this->field_18;
                    this->override(a2, v26, v32);
                }

                break;
            }
            default:
                assert(0 && "Invalid trigger type.");

                return;
            }
        } else {
            auto *v7 = this->field_C;

            float a2a = (!v7->is_flagged(0x20) ? v7->field_14 : 0.0f);
            float v36 = (!v7->is_flagged(0x20) ? v7->field_18 : 0.0f);

            this->override(a2, v36, a2a);
        }
    } else {
        THISCALL(0x0051D510, this, a2);
    }
}

game_button::~game_button()
{
    game_button *v2 = this->field_C;
    if (v2 != nullptr) {
        delete this->field_C;
        this->field_C = nullptr;
    }

    game_button *v3 = this->field_10;
    if (v3 != nullptr) {
        delete this->field_10;
        this->field_10 = nullptr;
    }
}

void *game_button::operator new(size_t size)
{
    return mem_alloc(size);
}

void game_button::operator delete(void *ptr, size_t size)
{
    mem_dealloc(ptr, size);
}

void game_button::clear()
{
    this->field_18 = 0.0;
    this->field_14 = 0.0;
    this->field_1C = 0.0;
    this->field_30 = 0;
    this->field_20 = 0.0;
    this->field_24 = 0.40000001f;
    this->field_28 = this->field_2C;
}

void game_button::set_trigger_type(int a2)
{
    this->m_trigger_type = a2;
    this->clear();
}

void game_button::set_control(game_control_t a2)
{
    game_button *v3 = this->field_C;
    if (v3 != nullptr) {
        delete this->field_C;
        this->field_C = nullptr;
    }

    game_button *v4 = this->field_10;
    if (v4 != nullptr) {
        delete this->field_10;
        this->field_10 = nullptr;
    }

    this->field_8 = a2;
    this->clear();
}

void game_button::set_id(device_id_t a2)
{
    this->field_4 = a2;
    this->clear();
}

void game_button::set_primary(const game_button &a2)
{
    auto *v3 = this->field_C;
    if (v3 != nullptr) {
        delete this->field_C;
        this->field_C = nullptr;
    }

    this->field_C = new game_button{a2};

    this->field_C->set_id(this->field_4);
    this->clear();
}

void game_button::set_modifier(const game_button &a2)
{
    if (this->field_10 != nullptr) {
        delete this->field_10;
        this->field_10 = nullptr;
    }

    if (this->field_C == nullptr) {
        this->set_primary(*this);
    }

    this->field_10 = new game_button{a2};
    this->field_10->set_id(this->field_4);
    this->clear();
}


bool game_button::is_pressed() const
{
    if (this->is_flagged(0x20)) {
        return false;
    }

    return (this->m_flags & GBFLAG_PRESSED);
}

bool game_button::is_triggered() const
{
    bool v5 = (!this->is_flagged(0x20) && (this->m_flags & GBFLAG_TRIGGERED) != 0);
    return v5;
}


void game_button_patch()
{
    FUNC_ADDRESS(address, &game_button::update);
    REDIRECT(0x004B6100, address);
}
