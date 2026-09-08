#include "entity.h"
#include "entity_base_vhandle.h"
#include "variable.h"
#include "wds.h"

#include "decal_morphs.h"

#include "func_wrapper.h"

decal_morphs::decal_morphs() {}

void decal_morphs::frame_advance(Float elapsed)
{
    struct morph_state {
        vhandle_type<entity> handle;
        float remaining;
        float duration;
        bool active;
        bool locked;
        char padding[2];
    };
    static auto &states = var<morph_state[30]>(0x0095AD50);

    for (auto &state : states) {
        if (state.locked) {
            continue;
        }
        if (state.active) {
            state.remaining -= elapsed.value;
        }
        auto *entity_ptr = state.handle.get_volatile_ptr();
        if (state.active && state.remaining < 0.0f) {
            if (entity_ptr != nullptr) {
                g_world_ptr->ent_mgr.release_entity(entity_ptr);
            }
            state.active = false;
        } else if (entity_ptr != nullptr) {
            auto color = entity_ptr->get_render_color();
            const float progress = 1.0f - state.remaining / state.duration;
            const float squared = progress * progress;
            color.field_0[3] = static_cast<uint8_t>(
                (1.0f - squared * squared) * 255.0f + 0.5f);
            entity_ptr->set_render_color(color);
        }
    }
}
