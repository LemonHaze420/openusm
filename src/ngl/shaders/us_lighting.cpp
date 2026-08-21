#include "us_lighting.h"

#include "func_wrapper.h"
#include "comic_panels.h"
#include "conglom.h"
#include "oldmath_po.h"
#include "variables.h"
#include "wds.h"
#include "wds_entity_manager.h"

#if 0
static string_hash usl_skydome_names[4] {
                                        int(to_hash("sky_day")),
                                        int(to_hash("sky_night")),
                                        int(to_hash("sky_rainy")),
                                        int(to_hash("sky_sunset")),
                                        };
#endif

void us_lighting_switch_time_of_day(int a1)
{
    if constexpr (0) {
        if (a1 <= 3) {
            g_TOD = a1;
            auto v1 = usl_street_tod_color()[a1][2];
            auto v2 = usl_street_tod_color()[a1][0];

            color v28;
            v28.g = usl_street_tod_color()[a1][1];
            v28.r = v2;
            v28.b = v1;
            v28.a = 1.0;
            comic_panels::set_default_bgcolor(v28);
        }

        for (int i = 0; i < 4; ++i) {
            _std::list<region *> regions{};
            auto *v5 = usl_skydomes()[i];
            if (v5 == nullptr) {
                mString v29{};
                auto *p_a7 = &regions;
                auto v19 = make_unique_entity_id();

                static Var<string_hash[4]> usl_skydome_names{0x00956388};
                string_hash v18 = usl_skydome_names()[i];
                usl_skydomes()[i] = g_world_ptr->ent_mgr.create_and_add_entity_or_subclass(
                    v18, v19, po_identity_matrix, v29, 0x10000000, p_a7);
                if (usl_skydomes()[i] != nullptr) {
                    usl_skydomes()[i]->set_ext_flag_recursive(static_cast<entity_ext_flag_t>(0x200000), true);

                    auto &v9 = g_world_ptr->field_23C;
                    v9.push_back(v5);

                    auto *v11 = v5;
                    v11->set_fade_distance(1000000.0);
                    if (v11->is_a_conglomerate()) {
                        auto *v14 = bit_cast<conglomerate *>(v5)->field_100;
                        if (v14 != nullptr) {
                            if (v14->size() != 0) {
                                usl_suns()[i] = v14->front();
                            }
                        }
                    }
                }
            }

            if (v5 != nullptr) {
                v5->set_visible(i == g_TOD, false);
            }
        }
    } else {
        CDECL_CALL(0x00408790, a1);
    }
}
