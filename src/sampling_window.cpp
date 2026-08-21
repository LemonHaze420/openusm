#include "sampling_window.h"

#include "common.h"
#include "func_wrapper.h"

Var<direction_sampling_window> up_sampling_window{0x009588A8};

VALIDATE_SIZE(sampling_window, 0x104);

sampling_window::sampling_window() {}

void sampling_window::push_sample(Float a2, Float a3)
{
    if constexpr (0) {
    } else {
        THISCALL(0x00527040, this, a2, a3);
    }
}

float sampling_window::average(Float duration) const
{
    if (this->field_100) {
        return 0.0f;
    }

    assert(duration >= DURATION_EPSILON);

    auto v9 = duration;
    if (duration > 1.0f) {
        v9 = 1.0;
    }

    auto v3 = 0.0f;
    auto v4 = 0.0f;
    auto *smp = &this->samples[this->current_sample];
    auto duration_left = v9;
    while (duration_left > DURATION_EPSILON) {
        assert(smp->time >= DURATION_EPSILON && smp->time < SAMPLING_PERIOD + EPSILON);

        auto v7 = smp->time;
        if (v7 > duration_left) {
            v7 = duration_left;
        }

        assert(duration_left >= DURATION_EPSILON);

        v4 += (smp->field_0 / smp->time) * v7;
        v3 += v7;
        duration_left = duration_left - v7;
        if (--smp < bit_cast<sample *>(this)) {
            smp = &this->samples[this->end_sample - 1];
        }
    }

    return v4 / v3;
}

vector3d direction_sampling_window::average(Float a4)
{
    vector3d result;
    THISCALL(0x0048B630, this, &result, a4);

    return result;
}
