#pragma once

#include "variable.h"
#include "vector3d.h"

struct sampling_window {
    struct sample {
        float field_0;
        float time;
    };

    sample samples[31];
    int current_sample;
    int end_sample;
    bool field_100;
    char tmp[3];

    sampling_window();

    void push_sample(Float a2, Float a3);

    float average(Float duration) const;
};

struct direction_sampling_window {
    sampling_window field_0[3];
    vector3d field_30C;

    //0x0048B630
    vector3d average(Float a4);
};

extern Var<direction_sampling_window> up_sampling_window;

inline Var<direction_sampling_window> heading_sampling_window{0x00958E70};

inline Var<sampling_window> heading_change_sampling_window{0x00958610};
