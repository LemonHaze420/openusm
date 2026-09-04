#include "ai_tentacle_info.h"

#include "ai_tentacle_engine.h"
#include "common.h"
#include "func_wrapper.h"
#include "local_collision.h"

#include <cmath>

VALIDATE_OFFSET(ai_tentacle_info, tween_positions, 0x48);
VALIDATE_SIZE(ai_tentacle_info, 0xD8u);

po ai_tentacle_info::get_end_po() const
{
    po v6{identity_matrix};
    this->field_78.to_matrix(v6.m);

    v6[3][0] = this->end_pos[0];
    v6[3][1] = this->end_pos[1];
    v6[3][2] = this->end_pos[2];

    return v6;
}

void ai_tentacle_info::set_position(int index, const vector3d &a1)
{
    assert(index >= 0);
    assert(index < get_num_positions());

    auto &v3 = this->positions.at(index);
    v3 = a1;
}

void ai_tentacle_info::set_code_blend(Float a2, Float a3)
{
    auto v3 = (a2 < 1.0f);
    auto v4 = equal<float>(a2, 1.0f);
    this->field_18 = a2;
    if (v3 || v4) {
        if (a2 < 0.0f) {
            this->field_18 = 0.0;
        }
    } else {
        this->field_18 = 1.0;
    }

    if (a3 >= LARGE_EPSILON) {
        this->field_1C = std::abs(this->field_18 - this->field_14) / a3;
    } else {
        this->field_1C = 0.0;
        this->field_14 = this->field_18;
    }

    if (this->field_18 > EPSILON && this->field_14 >= EPSILON) {
        this->init_code_tween(a3);
    }
}

void ai_tentacle_info::init_code_tween(Float a2)
{
    if (this->field_14 >= EPSILON) {
        if (a2 >= LARGE_EPSILON) {
            assert(this->end_pos.is_valid());

            this->init_positions(true);

            assert(this->end_pos.is_valid());

            assert(tween_positions.size() == positions.size());

            this->tween_positions = this->positions;

            this->field_2C = this->end_pos;

            this->field_38 = this->field_78;
            this->tween_amount = 0.0;
        } else {
            this->tween_duration = -1.0;
            this->tween_amount = 1.0;
        }

        this->tween_timer = 0.0;
        this->tween_duration = a2;
    }
}

void ai_tentacle_info::init_positions(bool a2)
{
    THISCALL(0x00476780, this, a2);
}

vector3d ai_tentacle_info::correct_tentacle_pos(line_info &a3, bool &a4, vector3d &a5, vector3d &a6)
{
    if constexpr (0) {
    } else {
        vector3d result;
        THISCALL(0x00465090, this, &result, &a3, &a4, &a5, &a6);
        return result;
    }
}

int ai_tentacle_info::push_engine(ai_tentacle_engine *eng)
{
    assert(eng != nullptr);

    this->engines.push_front(eng);
    return (eng != nullptr ? eng->field_10 : 0);
}
