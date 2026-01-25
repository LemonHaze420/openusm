#include "als_animation_logic_system_interface.h"

#include "als_animation_logic_system.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

namespace als {

void animation_logic_system_interface::frame_advance_pre_controller_all_alses(Float a1)
{
    TRACE("als::animation_logic_system_interface::frame_advance_pre_controller_all_alses");

    if constexpr (0) {
        for ( auto &v : the_als_list())
        {
            v.field_4 = v.field_0->frame_advance_should_do_frame_advance(a1);
        }

        for ( auto &v : the_als_list())
        {
            if ( v.field_4 )
            {
                v.field_0->frame_advance_main_als_advance(a1);
            }
        }

        for ( auto &v : the_als_list())
        {
            if ( v.field_4 )
            {
                v.field_0->frame_advance_post_request_processing(a1);
            }
        }

        for ( auto &v: the_als_list())
        {
            if ( v.field_4 )
            {
                v.field_0->frame_advance_on_layer_trans(a1);
            }
        }

        for ( auto &v : the_als_list() )
        {
            if ( v.field_4 )
            {
                v.field_0->frame_advance_post_logic_processing(a1);
            }
        }

        for ( auto &v : the_als_list() )
        {
            if ( v.field_4 )
            {
                v.field_0->frame_advance_play_new_animations(a1);
            }
        }

        for ( auto &v : the_als_list() )
        {
            if ( v.field_4 )
            {
                v.field_0->frame_advance_update_pending_params(a1);
            }
        }

        for ( auto &v : the_als_list() )
        {
            if ( v.field_4 )
            {
                v.field_0->frame_advance_change_mocomp(a1);
            }
        }

        for ( auto &v : the_als_list() )
        {
            if ( v.field_4 )
            {
                v.field_0->frame_advance_run_mocomp_pre_anim(a1);
            }
        }
    } else {
        CDECL_CALL(0x0049ED90, a1);
    }
}

void animation_logic_system_interface::frame_advance_controller_all_als(Float a1)
{
    void (*func)(Float) = CAST(func, 0x0049EED0);
    func(a1);
}

void animation_logic_system_interface::frame_advance_post_controller_all_alses(Float a1)
{
    void (*func)(Float) = CAST(func, 0x0049EF00);
    func(a1);
}


void animation_logic_system_interface::force_update(Float a2)
{
    TRACE("animation_logic_system_interface::force_update");

    THISCALL(0x00492FC0, this, a2);
}

void animation_logic_system_interface::force_update()
{
    TRACE("animation_logic_system_interface::force_update");

    this->force_update(0.000099999997);
}


}

void als_animation_logic_system_interface_patch() {
    REDIRECT(0x00537181, als::animation_logic_system_interface::frame_advance_pre_controller_all_alses);

    {
        void (als::animation_logic_system_interface::*func)(Float) = &als::animation_logic_system_interface::force_update;
        FUNC_ADDRESS(address, func);
        REDIRECT(0x00498D05, address);
        REDIRECT(0x00625FA1, address);
        REDIRECT(0x00642267, address);
    }
}

