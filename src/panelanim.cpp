#include "panelanim.h"

#include "common.h"
#include "matrix4x4.h"
#include "panelfile.h"
#include "panelanimkeyframe.h"
#include "trace.h"

#include "vector3d.h"

#include <cmath>
VALIDATE_SIZE(PanelAnim, 0x1C);

PanelAnim::PanelAnim() {}

namespace {
float key_value(int value)
{
    return bit_cast<float>(value);
}

const PanelAnimKeyframe *key_at(const mVector<PanelAnimKeyframe> &keys, int index)
{
    assert(index >= 0 && index < keys.size());
    return keys.m_data[index];
}

int key_interval(const mVector<PanelAnimKeyframe> &keys, float time)
{
    for (int i = 0; i + 1 < keys.size(); ++i) {
        if (time <= key_value(key_at(keys, i + 1)->field_14))
            return i;
    }
    return keys.size() - 2;
}
}

float PanelAnim::GetVisibility(Float time) const
{
    assert(field_0.size() > 0);
    const auto *first = key_at(field_0, 0);
    const auto *last = key_at(field_0, field_0.size() - 1);
    if (time <= key_value(first->field_14))
        return key_value(first->field_18);
    if (time >= key_value(last->field_14))
        return key_value(last->field_18);

    const int index = key_interval(field_0, time);
    const auto *from = key_at(field_0, index);
    const auto *to = key_at(field_0, index + 1);
    const float from_time = key_value(from->field_14);
    const float duration = key_value(to->field_14) - from_time;
    const float factor = duration > 0.0f ? (time - from_time) / duration : 0.0f;
    return key_value(from->field_18) +
        (key_value(to->field_18) - key_value(from->field_18)) * factor;
}

matrix4x4 PanelAnim::GetXFormMatrix(Float time) const
{
    assert(field_0.size() > 0);
    const PanelAnimKeyframe *from = key_at(field_0, 0);
    const PanelAnimKeyframe *to = from;
    float factor = 0.0f;

    const auto *last = key_at(field_0, field_0.size() - 1);
    if (time >= key_value(last->field_14)) {
        from = last;
        to = last;
    } else if (time > key_value(from->field_14)) {
        const int index = key_interval(field_0, time);
        from = key_at(field_0, index);
        to = key_at(field_0, index + 1);
        const float from_time = key_value(from->field_14);
        const float duration = key_value(to->field_14) - from_time;
        if (duration > 0.0f)
            factor = (time - from_time) / duration;
    }

    const auto interpolate = [&](int from_value, int to_value) {
        const float start = key_value(from_value);
        return start + (key_value(to_value) - start) * factor;
    };

    const float x = interpolate(from->field_0, to->field_0);
    const float y = interpolate(from->field_4, to->field_4);
    const float scale_x = interpolate(from->field_8, to->field_8);
    const float scale_y = interpolate(from->field_C, to->field_C);
    const float start_rotation = key_value(from->field_10);
    const float rotation_delta = std::remainder(
        key_value(to->field_10) - start_rotation, 6.2831853071795864769f);
    const float rotation = start_rotation + rotation_delta * factor;

    matrix4x4 scale;
    scale.make_scale(vector3d {scale_x, scale_y, 1.0f});
    matrix4x4 rotate;
    rotate.make_rotate(vector3d {0.0f, 0.0f, 1.0f}, rotation);
    matrix4x4 result = scale * rotate;
    result[3] = vector4d {x, y, 0.0f, 1.0f};
    return result;
}

void PanelAnim::PostUnmashFixup(PanelFile *pf)
{
    TRACE("PanelAnim::PostUnmashFixup");

    auto the_index = (int)this->field_14;
    assert(the_index >= 0);

    if (this->field_18) {
        assert(the_index < pf->pquads.size());
        this->field_14 = pf->pquads.m_data[the_index];
    } else {
        assert(the_index < pf->ptext.size());
        this->field_14 = CAST(this->field_14, pf->ptext.m_data[the_index]);
    }
}
