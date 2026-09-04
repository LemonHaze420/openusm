#include "nuge.h"

#include "rigid_body.h"

static Var<math::VecClass<3, 1>> stru_8BFDCC{0x008BFDCC};

static Var<math::VecClass<3, 0>> stru_8BFCA4{0x008BFCA4};

void nuge::get_ballistic_info(rigid_body *const *a1, int a2, math::VecClass<3, 1> *a3, math::VecClass<3, 0> *a4,
                              float *a5)
{
    *a3 = stru_8BFDCC();
    *a4 = stru_8BFCA4();

    *a5 = 0.0;
    for (auto i = 0; i < a2; ++i) {
        auto *v8 = a1[i];
        if (v8 != nullptr) {
            auto a2a = 1.f / v8->field_130;

            math::VecClass<3, 1> a3a;
            a3a[0] = a2a * v8->field_0[3][0];
            a3a[1] = a2a * v8->field_0[3][1];
            a3a[2] = a2a * v8->field_0[3][2];
            a3a[3] = a2a * v8->field_0[3][3];
            *a3 = *a3 + a3a;

            math::VecClass<3, 0> v21;
            v21[0] = a2a * v8->field_D0[0];
            v21[1] = a2a * v8->field_D0[1];
            v21[2] = a2a * v8->field_D0[2];
            v21[3] = a2a * v8->field_D0[3];
            *a4 = *a4 + v21;

            *a5 = a2a + *a5;
        }
    }

    auto v10 = 1.f / *a5;

    *a3 = v10 * (*a3);
}
