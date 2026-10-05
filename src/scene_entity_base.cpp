#include "scene_entity_base.h"

#include "oldmath_po.h"
#include "variable.h"

#include <cmath>

#include "func_wrapper.h"

scene_entity_base::scene_entity_base() {}

void scene_entity_base::initialize()
{
#if STANDALONE_SYSTEM
    auto &identity = var<po>(0x00920048);
    identity = po_identity_matrix;
    var<po *>(0x0095A29C) = &identity;

    auto &sin_indices = var<unsigned char[449]>(0x0095A0D8);
    for (int angle = 0; angle < 90; ++angle)
        sin_indices[angle] = static_cast<unsigned char>(angle + 90);
    for (int angle = 90; angle < 270; ++angle)
        sin_indices[angle] = static_cast<unsigned char>(14 - angle);
    for (int angle = 270; angle < 449; ++angle)
        sin_indices[angle] = static_cast<unsigned char>(angle - 270);
    auto &sine = var<float[181]>(0x0095A310);
    for (int angle = -90; angle <= 90; ++angle) {
        const float radians = static_cast<float>(double(angle) * 0.01745329238474369f + 4.71238899230957f);
        const float phase = -std::fabs(radians) * 0.15915493667125702f;
        const float t = std::fabs(std::ceil(phase) - phase - 0.5f) - 0.25f;
        const float t2 = t * t;
        const float t3 = t2 * t;
        const float t4 = t2 * t2;
        const float t5 = t4 * t;
        float result = t5 * t4 * 39.71065902709961f;
        result += t3 * t4 * -76.57495880126953f;
        result += t5 * 81.60222625732422f;
        result += t3 * -41.3416748046875f;
        sine[angle + 90] = result + t * 6.283185005187988f;
    }
#else
    CDECL_CALL(0x004D0F60);
#endif
}
