#include "web_sounds.h"

#include "actor.h"
#include "fixed_pool.h"
#include "func_wrapper.h"
#include "sound_and_pfx_interface.h"
#include "vector3d.h"

void web_sounds_manager::add_web_sound(actor *act, const vector3d &a2, string_hash a3)
{
    assert(act != nullptr);

    auto *v6 = act->my_sound_and_pfx_interface;
    auto *v7 = v6->get_web_sound_params(a3);
    if (v7 != nullptr) {
        auto *new_sound = new web_sound(v7, act, a2);
        assert(new_sound != nullptr);

        s_web_sounds->push_back(new_sound);
    } else {
        auto *v5 = a3.to_string();
        sp_log("Could not find web sound %s!", v5);
    }
}

web_sound::web_sound(web_sound_params *a2, actor *act, const vector3d &a4)
{
    THISCALL(0x00556D00, this, a2, act, &a4);
}

void *web_sound::operator new(size_t)
{
    return pool.allocate_new_block();
}

void web_sound::operator delete(void *ptr)
{
    pool.remove(ptr);
}
