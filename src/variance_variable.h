#pragma once

template <typename T>
struct variance_variable {
    T field_0;
    T field_4;

    variance_variable()
    {
        this->field_4 = 0;
        this->field_0 = 0;
    }
};
