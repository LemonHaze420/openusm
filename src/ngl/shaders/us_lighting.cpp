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

void us_lighting_switch_time_of_day(int tod)
{
#if STANDALONE_SYSTEM
    if (tod >= 0 && tod < 4) {
        g_TOD = tod;
        auto &street_color = usl_street_tod_color()[tod];
        comic_panels::set_default_bgcolor(color{street_color[0], street_color[1], street_color[2], 1.0f});
    }

    static const string_hash skydome_names[] = {
        string_hash{"sky_day"},
        string_hash{"sky_night"},
        string_hash{"sky_rainy"},
        string_hash{"sky_sunset"},
    };
    for (int index = 0; index < 4; ++index) {
        auto *skydome = usl_skydomes()[index];
        if (skydome == nullptr) {
            _std::list<region *> regions{};
            skydome = g_world_ptr->ent_mgr.create_and_add_entity_or_subclass(
                skydome_names[index], make_unique_entity_id(), po_identity_matrix, mString{}, 0x10000000, &regions);
            usl_skydomes()[index] = skydome;
            if (skydome != nullptr) {
                skydome->set_ext_flag_recursive(static_cast<entity_ext_flag_t>(0x200000), true);
                g_world_ptr->field_23C.push_back(skydome);
                skydome->set_fade_distance(1000000.0f);
                if (skydome->is_a_conglomerate()) {
                    auto *lights = bit_cast<conglomerate *>(skydome)->field_100;
                    if (lights != nullptr && !lights->empty()) {
                        usl_suns()[index] = lights->front();
                    }
                }
            }
        }

        if (skydome != nullptr) {
            skydome->set_visible(index == g_TOD, false);
        }
    }
#else
    CDECL_CALL(0x00408790, tod);
#endif
}
