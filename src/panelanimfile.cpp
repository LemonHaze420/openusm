#include "panelanimfile.h"

#include "common.h"
#include "fetext.h"
#include "func_wrapper.h"
#include "panelanim.h"
#include "matrix4x4.h"

#include "panelquad.h"

#include <cmath>
VALIDATE_SIZE(PanelAnimFile, 0x30);

void PanelAnimFile::Update(Float a2)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x00628A40, this, a2);
        return;
    }

    if (!field_2D)
        return;

    for (int i = 0; i < field_0.size(); ++i) {
        auto *target = field_0.m_data[i]->field_14;
        if (target != nullptr)
            target->ResetXform();
    }

    const float delta_time = std::min(float(a2), 0.083333001f);
    const float previous_time = bit_cast<float>(field_18);
    float current_time = previous_time + delta_time;
    field_18 = bit_cast<int>(current_time);

    if (current_time > field_20 && field_2C && field_24 == 0) {
        field_24 = 1;
        current_time -= field_20;
        field_18 = bit_cast<int>(current_time);
    }
    if (current_time > field_20 && field_28 == 1) {
        current_time -= field_20;
        field_18 = bit_cast<int>(current_time);
        if (field_2C)
            field_24 = 0;
    }

    float sample_time = current_time;
    if (sample_time > field_20 &&
        (previous_time < field_20 ||
         (std::fpclassify(field_20) == FP_ZERO &&
          std::fpclassify(previous_time) == FP_ZERO))) {
        sample_time = field_20;
    }
    if (sample_time > field_20) {
        Stop();
        return;
    }

    const float step_time = bit_cast<float>(field_1C);
    sample_time = field_24 == 1
        ? field_20 - sample_time + step_time
        : sample_time + step_time;

    for (int i = 0; i < field_0.size(); ++i) {
        auto *animation = field_0.m_data[i];
        auto *target = animation->field_14;
        if (target == nullptr || animation->field_0.empty())
            continue;

        const matrix4x4 transform = animation->GetXFormMatrix(sample_time);
        const float visibility = animation->GetVisibility(sample_time);
        if (animation->field_18)
            static_cast<PanelQuad *>(target)->Animate(transform, visibility);
        else
            reinterpret_cast<FEText *>(target)->Animate(transform, visibility);
    }
}

void PanelAnimFile::Stop()
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x00617260, this);
        return;
    }

    field_2D = false;
    for (int i = 0; i < field_0.size(); ++i) {
        auto *target = field_0.m_data[i]->field_14;
        if (target != nullptr)
            target->StartAnim(false);
    }
}

void PanelAnimFile::PostUnmashFixup(PanelFile *a1)
{
    for (auto i = 0; i < this->field_0.size(); ++i) {
        auto &v3 = this->field_0.m_data[i];
        v3->PostUnmashFixup(a1);
    }
}

void PanelAnimFile_patch()
{
    FUNC_ADDRESS(address, &PanelAnimFile::PostUnmashFixup);
    REDIRECT(0x006289DC, address);
}
