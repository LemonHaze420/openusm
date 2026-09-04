#pragma once

struct interaction_type_enum {
    int value;

    operator int()
    {
        return value;
    }
};
