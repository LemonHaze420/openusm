#pragma once

#include "color32.h"
#include "float.hpp"
#include "mstring.h"

#include <vector.hpp>

struct message_board {
    struct internal {
        char field_0[100];
        float field_64;
        color32 field_68;

        internal()
        {
            field_68 = {};
        }
    };

    _std::vector<internal> field_0;

    message_board() = default;

    struct string {
        int field_0;
        int field_4;
        char *guts;
        void *field_C;
    };

    //0x00515EB0
    void post(string a1, Float a2, color32 a3);

    void frame_advance(float a2);

    void render();
};

extern void message_board_patch();
