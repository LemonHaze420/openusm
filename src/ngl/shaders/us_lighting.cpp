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
    if (tod < 0 || tod > 3) {
        return;
    }

    g_TOD = tod;
    auto &street_color = usl_street_tod_color()[tod];
    comic_panels::set_default_bgcolor(
        color{street_color[0], street_color[1], street_color[2], 1.0f});

    for (int i = 0; i < 4; ++i) {
        if (auto *skydome = usl_skydomes()[i]; skydome != nullptr) {
            skydome->set_visible(i == g_TOD, false);
        }
    }
#else
    CDECL_CALL(0x00408790, tod);
#endif
}
