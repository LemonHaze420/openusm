#include "als_animation_logic_system_interface.h"

#include "als_animation_logic_system.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

namespace als {

_std::list<animation_logic_system_interface::value_t> &animation_logic_system_interface::the_als_list =
    var<_std::list<animation_logic_system_interface::value_t>>(0x009597C0);

state_machine *animation_logic_system_interface::get_als_layer(layer_types a2)
{
    base_state_machine *(__fastcall * func)(void *, void *edx, layer_types a2) =
        CAST(func, get_vfunc(this->m_vtbl, 0x0));
    return func(this, nullptr, a2);
}

void animation_logic_system_interface::kill_all_domains(uint32_t a2)
{
    void(__fastcall * func)(void *, void *edx, uint32_t) = CAST(func, get_vfunc(this->m_vtbl, 0x4));
    func(this, nullptr, a2);
}

void animation_logic_system_interface::suspend_logic_system(bool a2)
{
    void(__fastcall * func)(void *, void *edx, bool) = CAST(func, get_vfunc(this->m_vtbl, 0x8));
    func(this, nullptr, a2);
}

void animation_logic_system_interface::create_instance_data(animation_logic_system_shared *system_shared)
{
    void(__fastcall * func)(void *, void *edx, animation_logic_system_shared *) =
        CAST(func, get_vfunc(this->m_vtbl, 0xC));
    func(this, nullptr, system_shared);
}

void animation_logic_system_interface::delete_instance_data()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(this->m_vtbl, 0x10));
    func(this);
}

void animation_logic_system_interface::reset_animation_player()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(this->m_vtbl, 0x14));
    func(this);
}

bool animation_logic_system_interface::frame_advance_should_do_frame_advance(Float a2)
{
    bool(__fastcall * func)(void *, void *edx, Float) = CAST(func, get_vfunc(this->m_vtbl, 0x18));
    return func(this, nullptr, a2);
}

void animation_logic_system_interface::frame_advance_main_als_advance(Float a2)
{
    void(__fastcall * func)(void *, void *edx, Float) = CAST(func, get_vfunc(this->m_vtbl, 0x1C));
    func(this, nullptr, a2);
}

void animation_logic_system_interface::frame_advance_post_request_processing(Float a2)
{
    void(__fastcall * func)(void *, void *edx, Float) = CAST(func, get_vfunc(this->m_vtbl, 0x20));
    func(this, nullptr, a2);
}

void animation_logic_system_interface::frame_advance_on_layer_trans(Float a2)
{
    void(__fastcall * func)(void *, void *edx, Float) = CAST(func, get_vfunc(this->m_vtbl, 0x24));
    func(this, nullptr, a2);
}

void animation_logic_system_interface::frame_advance_post_logic_processing(Float a2)
{
    void(__fastcall * func)(void *, void *edx, Float) = CAST(func, get_vfunc(this->m_vtbl, 0x28));
    func(this, nullptr, a2);
}

void animation_logic_system_interface::frame_advance_play_new_animations(Float a2)
{
    void(__fastcall * func)(void *, void *edx, Float) = CAST(func, get_vfunc(this->m_vtbl, 0x2C));
    func(this, nullptr, a2);
}

void animation_logic_system_interface::frame_advance_update_pending_params(Float a2)
{
    void(__fastcall * func)(void *, void *edx, Float) = CAST(func, get_vfunc(this->m_vtbl, 0x30));
    func(this, nullptr, a2);
}

void animation_logic_system_interface::frame_advance_change_mocomp(Float a2)
{
    void(__fastcall * func)(void *, void *edx, Float) = CAST(func, get_vfunc(this->m_vtbl, 0x34));
    func(this, nullptr, a2);
}

void animation_logic_system_interface::frame_advance_run_mocomp_pre_anim(Float a2)
{
    void(__fastcall * func)(void *, void *edx, Float) = CAST(func, get_vfunc(this->m_vtbl, 0x38));
    func(this, nullptr, a2);
}

void animation_logic_system_interface::frame_advance_controller(Float a2)
{
    void(__fastcall * func)(void *, void *edx, Float) = CAST(func, get_vfunc(this->m_vtbl, 0x3C));
    func(this, nullptr, a2);
}

void animation_logic_system_interface::frame_advance_post_controller(Float a2)
{
    void(__fastcall * func)(void *, void *edx, Float) = CAST(func, get_vfunc(this->m_vtbl, 0x40));
    func(this, nullptr, a2);
}

bool animation_logic_system_interface::sub_4933E0()
{
    bool(__fastcall * func)(void *) = CAST(func, get_vfunc(this->m_vtbl, 0x44));
    return func(this);
}

void animation_logic_system_interface::change_mocomp()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(this->m_vtbl, 0x48));
    func(this);
}

void animation_logic_system_interface::frame_advance_pre_controller_all_alses(Float a1)
{
    TRACE("als::animation_logic_system_interface::frame_advance_pre_controller_all_alses");

    if constexpr (0) {
        for (auto &v : the_als_list) {
            v.field_4 = v.field_0->frame_advance_should_do_frame_advance(a1);
        }

        for (auto &v : the_als_list) {
            if (v.field_4) {
                v.field_0->frame_advance_main_als_advance(a1);
            }
        }

        for (auto &v : the_als_list) {
            if (v.field_4) {
                v.field_0->frame_advance_post_request_processing(a1);
            }
        }

        for (auto &v : the_als_list) {
            if (v.field_4) {
                v.field_0->frame_advance_on_layer_trans(a1);
            }
        }

        for (auto &v : the_als_list) {
            if (v.field_4) {
                v.field_0->frame_advance_post_logic_processing(a1);
            }
        }

        for (auto &v : the_als_list) {
            if (v.field_4) {
                v.field_0->frame_advance_play_new_animations(a1);
            }
        }

        for (auto &v : the_als_list) {
            if (v.field_4) {
                v.field_0->frame_advance_update_pending_params(a1);
            }
        }

        for (auto &v : the_als_list) {
            if (v.field_4) {
                v.field_0->frame_advance_change_mocomp(a1);
            }
        }

        for (auto &v : the_als_list) {
            if (v.field_4) {
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

void animation_logic_system_interface::remove_from_als_list(als::animation_logic_system_interface *a1)
{
    for (auto it = the_als_list.begin(), end = the_als_list.end(); it != end; ++it) {
        if (it->field_0 == a1) {
            the_als_list.erase(it);
            return;
        }
    }
}

}  // namespace als

void als_animation_logic_system_interface_patch()
{
    REDIRECT(0x00537181, als::animation_logic_system_interface::frame_advance_pre_controller_all_alses);

    {
        void (als::animation_logic_system_interface::*func)(Float) =
            &als::animation_logic_system_interface::force_update;
        FUNC_ADDRESS(address, func);
        REDIRECT(0x00498D05, address);
        REDIRECT(0x00625FA1, address);
        REDIRECT(0x00642267, address);
    }
}
