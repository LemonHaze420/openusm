#include "sampling_window.h"

#include "common.h"
#include "func_wrapper.h"

Var<direction_sampling_window> up_sampling_window{0x009588A8};

VALIDATE_SIZE(sampling_window, 0x104);

sampling_window::sampling_window()
{
    for (auto &sample : this->samples) {
        sample.field_0 = 0.0f;
        sample.time = 0.033333335f;
    }
    this->current_sample = 0;
    this->end_sample = 30;
    this->field_100 = true;
    this->samples[0].time = 0.0f;
}

void sampling_window::push_sample(Float elapsed, Float value)
{
    constexpr float period = 0.033333335f;
    if (this->field_100) {
        this->field_100 = false;
        for (int i = 0; i < 30; ++i) {
            this->samples[i].field_0 = static_cast<float>(value) * period;
            this->samples[i].time = period;
        }
    }
    for (float remaining = elapsed; remaining >= DURATION_EPSILON;) {
        float available = period - this->samples[this->current_sample].time;
        if (available <= 0.0f) {
            if (++this->current_sample == this->end_sample) {
                this->current_sample = 0;
            }
            this->samples[this->current_sample].time = 0.0f;
            this->samples[this->current_sample].field_0 = 0.0f;
            available = period;
        }
        const float taken = remaining < available ? remaining : available;
        auto &sample = this->samples[this->current_sample];
        sample.field_0 += static_cast<float>(value) * taken;
        sample.time += taken;
        remaining -= taken;
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
    if constexpr (STANDALONE_SYSTEM) {
        const float z = field_0[2].average(a4);
        const float y = field_0[1].average(a4);
        const float x = field_0[0].average(a4);
        if (!(x * x + y * y + z * z <= EPSILON))
            field_30C = vector3d{x, y, z};
        return field_30C;
    } else {
        vector3d result;
        THISCALL(0x0048B630, this, &result, a4);
        return result;
    }
}
