#include "script_lib.h"

#include "glass_house_manager.h"
#include "mission_manager.h"
#include "script_instance_mashing.h"
#include "script_object.h"
#include "trace.h"
#include "utility.h"
#include "vm_executable.h"
#include "vm_stack.h"
#include "vm_thread.h"


slf__abs_delay__num__t::slf__abs_delay__num__t(const char *a3) : function(a3)
{
    m_vtbl = CAST(m_vtbl, 0x0089A724);
    FUNC_ADDRESS(address, &slf__abs_delay__num__t::operator());
    m_vtbl->__cl = CAST(m_vtbl->__cl, address);
}

bool slf__abs_delay__num__t::operator()(vm_stack &stack,
                                        [[maybe_unused]] script_library_class::function::entry_t entry) const
{
    TRACE("slf__abs_delay__num__t::operator()");

    bool(__fastcall * func)(const void *, void *edx, vm_stack *, entry_t) = CAST(func, 0x00663170);
    return func(this, nullptr, &stack, entry);
}

slf__acos__num__t::slf__acos__num__t(const char *a3) : function(a3)
{
    m_vtbl = CAST(m_vtbl, 0x0089A91C);
    FUNC_ADDRESS(address, &slf__acos__num__t::operator());
    m_vtbl->__cl = CAST(m_vtbl->__cl, address);
}

bool slf__acos__num__t::operator()(vm_stack &stack,
                                   [[maybe_unused]] script_library_class::function::entry_t entry) const
{
    TRACE("slf__acos__num__t::operator()");

    bool(__fastcall * func)(const void *, void *edx, vm_stack *, entry_t) = CAST(func, 0x00663F80);
    return func(this, nullptr, &stack, entry);
}

slf__add_2d_debug_str__vector3d__vector3d__num__str__t::slf__add_2d_debug_str__vector3d__vector3d__num__str__t(
    const char *a3)
    : function(a3)
{
    m_vtbl = CAST(m_vtbl, 0x0089A860);
    FUNC_ADDRESS(address, &slf__add_2d_debug_str__vector3d__vector3d__num__str__t::operator());
    m_vtbl->__cl = CAST(m_vtbl->__cl, address);
}

bool slf__add_2d_debug_str__vector3d__vector3d__num__str__t::operator()(
    vm_stack &stack, [[maybe_unused]] script_library_class::function::entry_t entry) const
{
    TRACE("slf__add_2d_debug_str__vector3d__vector3d__num__str__t::operator()");

    bool(__fastcall * func)(const void *, void *edx, vm_stack *, entry_t) = CAST(func, 0x00663360);
    return func(this, nullptr, &stack, entry);
}

slf__add_2d_debug_str__vector3d__vector3d__num__str__num__t::
    slf__add_2d_debug_str__vector3d__vector3d__num__str__num__t(const char *a3)
    : function(a3)
{
    m_vtbl = CAST(m_vtbl, 0x0089A858);
    FUNC_ADDRESS(address, &slf__add_2d_debug_str__vector3d__vector3d__num__str__num__t::operator());
    m_vtbl->__cl = CAST(m_vtbl->__cl, address);
}

bool slf__add_2d_debug_str__vector3d__vector3d__num__str__num__t::operator()(
    vm_stack &stack, [[maybe_unused]] script_library_class::function::entry_t entry) const
{
    TRACE("slf__add_2d_debug_str__vector3d__vector3d__num__str__num__t::operator()");

    bool(__fastcall * func)(const void *, void *edx, vm_stack *, entry_t) = CAST(func, 0x00663760);
    return func(this, nullptr, &stack, entry);
}

slf__add_3d_debug_str__vector3d__vector3d__num__str__t::slf__add_3d_debug_str__vector3d__vector3d__num__str__t(
    const char *a3)
    : function(a3)
{
    m_vtbl = CAST(m_vtbl, 0x0089A850);
    FUNC_ADDRESS(address, &slf__add_3d_debug_str__vector3d__vector3d__num__str__t::operator());
    m_vtbl->__cl = CAST(m_vtbl->__cl, address);
}


bool slf__add_3d_debug_str__vector3d__vector3d__num__str__t::operator()(vm_stack &stack,
                                                                        script_library_class::function::entry_t)
{
    TRACE("slf__add_3d_debug_str__vector3d__vector3d__num__str__t::operator()");

    SLF_PARMS;

    mString v9{parms->field_1C};
    auto v8 = parms->field_18;
    uint8_t v7 = (parms->field_C[2] * 255.0);
    uint8_t v6 = (parms->field_C[1] * 255.0);
    uint8_t v3 = (parms->field_C[0] * 255.0);
    color32 v4{v3, v6, v7, 255};
    add_3d_debug_str(parms->field_0, v4, v8, v9);

    SLF_DONE;
}

slf__is_point_inside_glass_house__vector3d__t::slf__is_point_inside_glass_house__vector3d__t(const char *a3)
    : function(a3)
{
    m_vtbl = CAST(m_vtbl, 0x0089A540);
    FUNC_ADDRESS(address, &slf__is_point_inside_glass_house__vector3d__t::operator());
    m_vtbl->__cl = CAST(m_vtbl->__cl, address);
}

bool slf__is_point_inside_glass_house__vector3d__t::operator()(
    vm_stack &stack, [[maybe_unused]] script_library_class::function::entry_t entry) const
{
    TRACE("slf__is_point_inside_glass_house__vector3d__t::operator()");

    SLF_PARMS;

    float result = (glass_house_manager::is_point_in_glass_house(parms->v) ? 1.0f : 0.0f);

    SLF_RETURN;
    SLF_DONE;
}


slf__initialize_encounter_object__t::slf__initialize_encounter_object__t(const char *a3) : function(a3)
{
    m_vtbl = CAST(m_vtbl, 0x0089AA0C);
    FUNC_ADDRESS(address, &slf__initialize_encounter_object__t::operator());
    m_vtbl->__cl = CAST(m_vtbl->__cl, address);
}

bool slf__initialize_encounter_object__t::operator()(
    vm_stack &stack, [[maybe_unused]] script_library_class::function::entry_t entry) const
{
    TRACE("slf__initialize_encounter_object__t::operator()");

    if constexpr (0) {
        auto *v2 = stack.get_thread();
        auto *se = v2->inst->get_parent()->get_parent();
        auto *ex = v2->get_running_executable();

        assert(se != nullptr);

        assert(ex != nullptr);

        if (entry != 0) {
            return false;
        }

        auto *so = ex->get_owner();
        auto v6 = so->get_name();
        if (initialize_game_init_instances(se, v6)) {
            return true;
        }

        assert(0 && "USM SHIPPING HACK: Could not initialize encounter object, trying to exit script gracefully.");

        mission_manager::s_inst->prepare_unload_script();

        return false;
    } else {
        bool(__fastcall * func)(const void *, void *edx, vm_stack *, entry_t) = CAST(func, 0x00664490);
        return func(this, nullptr, &stack, entry);
    }
}
