#pragma once

#include "float.hpp"
#include "input_mgr.h"
#include "commands.h"

#include <cstdint>

inline constexpr auto GBFLAG_PRESSED = 1;
inline constexpr auto GBFLAG_TRIGGERED = 2;
inline constexpr auto GBFLAG_RELEASED = 4;

class game_button {
    int m_trigger_type;
    device_id_t field_4;
    game_control_t field_8;
    game_button *field_C;
    game_button *field_10;
    float field_14;
    float field_18;

public:
    float field_1C;

private:
    float field_20;
    float field_24;
    float field_28;

public:
    float field_2C;

private:
    int16_t field_30;
    int16_t m_flags;

public:
    //0x0048D9A0
    game_button();

    game_button(game_control_t a2);

    game_button(const game_button &a2);

    game_button(const game_button &a2, const game_button &a3, int a4);

    game_button &operator=(const game_button &a2);

    //0x0048C6F0
    ~game_button();

    void *operator new(size_t size);

    void operator delete(void *ptr, size_t size);

    bool is_flagged(uint32_t a2) const
    {
        return (a2 & this->m_flags) != 0;
    }

    void clear_flags()
    {
        this->m_flags &= 0x20u;
    }

    void set_flag(uint16_t a2, bool a3)
    {
        if (a3) {
            this->m_flags |= a2;
        } else {
            this->m_flags &= ~a2;
        }
    }

    float sub_55ED50() const;

    float sub_55ED30() const;

    //0x0050B640
    void override(Float a2, Float a3, Float a4);

    //0x0051D510
    void update(Float a2);

    void set_id(device_id_t a2);

    void set_trigger_type(int a2);

    //0x0048C770
    void set_control(game_control_t a2);

    //0x0051D310
    void set_primary(const game_button &a2);

    //0x0051D410
    void set_modifier(const game_button &a2);

    //0x0050B610
    void clear();

    bool is_pressed() const;

    bool is_triggered() const;
};

extern void game_button_patch();
