#include "textureinputpack.h"

#include "common.h"
#include "float.hpp"
#include "func_wrapper.h"
#include "input.h"
#include "ngl_dx_texture.h"
#include "utility.h"
#include "variables.h"
#include "vtbl.h"

#include <d3d9.h>

VALIDATE_SIZE(TexturePackBase, 0x6C);

VALIDATE_OFFSET(TextureInputPack, field_9C, 0x9C);
VALIDATE_OFFSET(TextureInputPack, field_3C0, 0x3C0);
VALIDATE_OFFSET(TextureInputPack, field_414, 0x414);
VALIDATE_OFFSET(TextureInputPack, field_5AC, 0x5AC);

TextureInputPack::TextureInputPack() {}

void __fastcall sub_585BE0(void *self, void *, void *a2, int a3, int a4, int a5, int a6, Float a7) {
    THISCALL(0x00585BE0, self, a2, a3, a4, a5, a6, a7);
}

void TextureInputPack::sub_5871D0() {
    if constexpr (0) {
        if (g_distance_clipping_enabled()) {
            g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_FOGENABLE, false);
        }

        nglSetTextureStageState(0, D3DTSS_COLOROP, 2u);
        nglSetTextureStageState(0, D3DTSS_COLORARG1, 2u);
        nglSetTextureStageState(0, D3DTSS_ALPHAOP, 2u);
        nglSetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);

        auto v2 = this->field_4;
        auto v3 = this->field_8;
        auto v4 = this->field_C - v2 + 1;
        auto v49 = this->field_98 - this->field_94;
        [[maybe_unused]] auto v48 = v4;

        int v54[4];

        int v5 = 0;
        auto *v46 = &this->field_3C0;
        do {
            auto *v6 = v46;
            v54[v5++] = (v4 * *v46);
            v46 = v6 + 1;
        } while (v5 < 4);

        int v7 = 0;
        if (this->field_9C + 1 > 0) {
            char *v46 = this->field_A0;
            do {
                auto v8 = this->field_3D0;
                auto v48 = v2 + v54[v7];

                auto v9 = this->field_3E0;
                auto v38 = this->field_3EC;
                auto v10 = v8 + v3 - 1;
                auto *v11 = this->field_1C;

                void (__fastcall *func4)(void *, void *, void *, int, int, int, int, color32, int, Float, Float, int) = CAST(func4, get_vfunc(v11->m_vtbl, 0x4));

                func4(v11,
                        nullptr,
                        v46,
                        v2,
                        v3,
                        v48 - 1,
                        v10,
                        v38,
                        v9,
                        Float{1.0},
                        Float{1.0},
                        0);

                v2 = v48;
                ++v7;

                v46 += 50;
            } while (v7 < (this->field_9C + 1));
        }

        auto v13 = this->field_3D0;
        auto v14 = this->field_94;
        auto *v15 = (int *) this->field_88;
        auto v16 = v13 + v3;
        auto v47 = v16;
        auto *v44 = v15;
        if (v14 > 0) {
            do {
                --v14;
                v44 = (int *) v44[61];
            } while (v14 != 0);
        }

        auto v17 = (this->field_10 - v13 - this->field_8 + 1) / this->field_3D4;

        int v18 = 0;
        int v45 = 0;
        for (auto i = v17; v18 < v17; v45 = v18) {
            if (v18 >= v49) {
                break;
            }

            auto v19 = this->field_9C;
            char *v46 = (char *) this->field_4;
            int v20 = 0;
            int v43 = 0;
            if (v19 + 1 > 0) {
                while (1) {
                    auto v21 = this->field_3D8;
                    auto v22 = v21 + v16;
                    auto v23 = v16 - v21 + this->field_3D4 - 1;
                    auto *v24 = &v46[v21];
                    auto v52 = (int) &v46[v54[v20] - v21 - 1];
                    if (v20 <= 0) {
                        void (__fastcall *func4)(void *, void *, char *, int, int, int, int, color32, int, Float, Float, int) = CAST(func4, get_vfunc(this->field_1C->m_vtbl, 0x4));

                        func4(this->field_1C,
                              nullptr,
                              (char *) v44 + 8,
                              (int) v46 + v21,
                              v22,
                              (int) v52,
                              v23,
                              this->field_3F0,
                              this->field_3E4,
                              Float{1.0},
                              Float{1.0},
                              0);
                    } else if (v45 + this->field_94 == this->field_8C && v20 == this->field_90 + 1) {
                        int v39;
                        color32 v25;
                        if (this->field_410 == v20 && this->field_414 == v45) {
                            v39 = this->field_3E8;
                            v25 = this->field_3FC;
                        } else {
                            v39 = this->field_3E8;
                            v25 = this->field_3F4;
                        }

                        void (__fastcall *func4)(void *, void *, const char *, int, int, int, int, color32, int, Float, Float, int) = CAST(func4, get_vfunc(this->field_1C->m_vtbl, 0x4));
                        func4(this->field_1C,
                              nullptr,
                              "?????",
                              (int) v24,
                              v22,
                              (int) v52,
                              v23,
                              v25,
                              v39,
                              1.0,
                              1.0,
                              0);
                    } else {
                        auto *v26 = &v44[6 * this->field_40C + 3 * this->field_40C + 3 * v20];
                        auto *v27 = this->field_418->get_string((InputType) v26[49], v26[50]);
                        auto v48 = (int) v27;

                        char v42;
                        unsigned int v50;
                        if (v27 != nullptr && (v42 = (int) (v27 + 1), (v50 = strlen(v27)) != 0)) {
                            v42 = ' ';
                            v50 = (unsigned int) (v27 + 1);
                            if (strlen(v27) == 1) {
                                auto v28 = *v27;
                                switch (v28) {
                                case -32:
                                case -28:
                                case -25:
                                case -24:
                                case -23:
                                case -20:
                                case -15:
                                case -14:
                                case -10:
                                case -7:
                                case -4:
                                    v42 = v28 - 32;
                                    break;
                                default:
                                    break;
                                }
                            }

                            int v29 = v44[6 * this->field_40C + 49 + 3 * this->field_40C + 3 * v43];
                            if (v29 != 1 && v29 != 2) {
                                auto v30 = (int) v24;
                                auto v31 = (int) &v24[v23 - v22];
                                [[maybe_unused]] auto *v24 = (char *) (v31 + this->field_3D8);

                                switch (v29) {
                                case 3:
                                    sub_585BE0(this->field_1C,
                                               nullptr,
                                               this->field_78,
                                               v30,
                                               v22,
                                               v31,
                                               v23,
                                               0.2);
                                    break;
                                case 4:
                                    sub_585BE0(this->field_1C,
                                               nullptr,
                                               this->field_7C,
                                               v30,
                                               v22,
                                               v31,
                                               v23,
                                               0.2);
                                    break;
                                case 5:
                                    sub_585BE0(this->field_1C,
                                               nullptr,
                                               this->field_80,
                                               v30,
                                               v22,
                                               v31,
                                               v23,
                                               0.2);
                                    break;
                                case 6:
                                    sub_585BE0(this->field_1C,
                                               nullptr,
                                               this->field_84,
                                               v30,
                                               v22,
                                               v31,
                                               v23,
                                               0.2);
                                    break;
                                default:
                                    break;
                                }
                            }

                            int v40;
                            color32 v32;
                            if (this->field_410 == v43 && this->field_414 == v45 && byte_965C20()) {
                                v40 = this->field_3E8;
                                v32 = this->field_3FC;
                            } else {
                                v40 = this->field_3E8;
                                v32 = this->field_3F4;
                            }

                            auto *v33 = this->field_1C;
                            void (__fastcall *func4)(void *, void *, const char *, int, int, int, int, color32, int, Float, Float, int) = CAST(func4, get_vfunc(v33->m_vtbl, 0x4));
                            if (v42 == ' ') {
                                func4(v33,
                                      nullptr,
                                      (char *) v48,
                                      (int) v24,
                                      v22,
                                      (int) v52,
                                      v23,
                                      v32,
                                      v40,
                                      1.0,
                                      1.0,
                                      0);
                            } else {
                                func4(v33,
                                      nullptr,
                                      (char *) &v42,
                                      (int) v24,
                                      v22,
                                      (int) v52,
                                      v23,
                                      v32,
                                      v40,
                                      1.0,
                                      1.0,
                                      0);
                            }
                        } else {
                            int v41;
                            color32 v35;
                            if (this->field_410 == v43 && this->field_414 == v45 && byte_965C20()) {
                                v41 = this->field_3E8;
                                v35 = this->field_3FC;
                            } else {
                                v41 = this->field_3E8;
                                v35 = this->field_3F8;
                            }

                            void (__fastcall *func4)(void *, void *, const char *, int, int, int, int, color32, int, Float, Float, int) = CAST(func4, get_vfunc(this->field_1C->m_vtbl, 0x4));
                            func4(this->field_1C,
                                  nullptr,
                                  this->field_5AC,
                                  (int) v24,
                                  v22,
                                  (int) v52,
                                  v23,
                                  v35,
                                  v41,
                                  1.0,
                                  1.0,
                                  0);
                        }
                    }

                    v16 = v47;
                    auto v36 = v43 + 1 < this->field_9C + 1;
                    v46 += v54[v43++];
                    if (!v36) {
                        break;
                    }

                    v20 = v43;
                }
                v18 = v45;
                v17 = i;
            }

            auto *v37 = (int *) v44[61];
            v16 += this->field_3D4;
            ++v18;
            v47 = v16;
            v44 = v37;
        }

        if (g_distance_clipping_enabled()) {
            g_Direct3DDevice()->lpVtbl->SetRenderState(g_Direct3DDevice(), D3DRS_FOGENABLE, true);
        }

    } else {
        THISCALL(0x005871D0, this);
    }
}

void __fastcall sub_5B24F0(void *self,
                           void *,
                           const char *a2,
                           int a3,
                           int a4,
                           int a5,
                           int a6,
                           int a7,
                           int a8,
                           Float a9,
                           Float a10,
                           int a11) {
    THISCALL(0x005B24F0, self, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

void TextureInputPack_patch() {
    set_vfunc(0x0088EBB0, sub_5B24F0);

    {
        FUNC_ADDRESS(address, &TextureInputPack::sub_5871D0);
        set_vfunc(0x0088E904, address);
    }
}
