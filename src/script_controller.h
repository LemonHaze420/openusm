#pragma once

#include "signaller.h"

struct script_controller : signaller {
    //0x0064E300
    script_controller();

    //0x0065F8A0
    //virtual;
    void update();

    bool is_button_pressed(int a1) const;

    float get_axis_position(int a1) const;

    bool is_left_pressed() const {
        return this->get_axis_position(2) <= -1.0 * 0.7;
    }

    bool is_right_pressed() const {
        return this->get_axis_position(2) >= 1.0 * 0.7;
    }

};

extern script_controller *& script_pad;

extern void script_controller_patch();
