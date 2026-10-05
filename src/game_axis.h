#pragma once

#include "float.hpp"

struct game_axis {
    int field_0;
    int field_4;
    float field_8;
    float field_C;
    float field_10;
    float m_threshold;
    float field_18;
    float field_1C;
    float m_timeout;
    int field_24;
    int field_28;
    bool field_2C;
    bool field_2D;

    game_axis();

    void clear();

    void set_id(int a2)
    {
        this->field_0 = a2;
        this->clear();
    }

    void set_control(int control)
    {
        this->field_4 = control;
        this->clear();
    }

    //0x0051D900
    void update(Float a2);


    void override(Float dt, float value, float delta);


    void update_multitap(Float dt);

    static inline float default_timeout = 0.40000001f;

    static inline float default_threshold = 0.40000001f;
};
