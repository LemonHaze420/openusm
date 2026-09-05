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
    constexpr float three_pi_over_two = 4.71238898038468985769f;
    auto &sine = var<float[181]>(0x0095A310);
    for (int angle = -90; angle <= 90; ++angle)
        sine[angle + 90] = std::sin(angle * 0.01745329251994329577f + three_pi_over_two);
#else
    CDECL_CALL(0x004D0F60);
#endif
}
