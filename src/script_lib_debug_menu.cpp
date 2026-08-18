#include "script_lib_debug_menu.h"

#include "debug_menu.h"
#include "func_wrapper.h"
#include "mstring.h"
#include "script_executable.h"
#include "script_manager.h"
#include "script_object.h"
#include "trace.h"
#include "utility.h"
#include "vm_executable.h"
#include "vm_thread.h"

extern debug_menu *script_menu;

extern debug_menu *progression_menu;

int vm_debug_menu_entry_garbage_collection_id = -1;

void init_script_debug_menu()
{
    if (script_menu == nullptr) {
        script_menu = create_menu("Script");
        debug_menu::root_menu->add_entry(script_menu);
    }

    if (progression_menu == nullptr) {
        progression_menu = create_menu("Progression");
        debug_menu::root_menu->add_entry(progression_menu);
    }
}

void vm_debug_menu_entry_garbage_collection_callback(script_executable *,
                                                    _std::list<uint32_t> &a2,
                                                    _std::list<mString> &)
{
    for ( auto &v2 : a2 )
    {
        assert(script_menu != nullptr);

        auto *entry = bit_cast<debug_menu_entry *>(v2);
        
        // script_menu->remove_entry(entry);
        remove_debug_menu_entry(entry);
    }
}

void construct_debug_menu_lib()
{
    if ( vm_debug_menu_entry_garbage_collection_id == -1 ) {
#ifdef OPENUSM_XBPACK_V10
        vm_debug_menu_entry_garbage_collection_id = CDECL_CALL(
            0x005AFE40,
            vm_debug_menu_entry_garbage_collection_callback);
#else
        vm_debug_menu_entry_garbage_collection_id = script_manager::register_allocated_stuff_callback(vm_debug_menu_entry_garbage_collection_callback);
#endif
    }
}

slf__create_debug_menu_entry__str__t::slf__create_debug_menu_entry__str__t(const char *a3) : function(a3)
{
    m_vtbl = CAST(m_vtbl, 0x0089C704);
    FUNC_ADDRESS(address, &slf__create_debug_menu_entry__str__t::operator());
    m_vtbl->__cl = CAST(m_vtbl->__cl, address);
}

bool slf__create_debug_menu_entry__str__t::operator()(vm_stack &stack, [[maybe_unused]]script_library_class::function::entry_t entry) const
{
    TRACE("slf__create_debug_menu_entry__str__t::operator()");

#ifdef OPENUSM_XBPACK_V10
    SLF_PARMS;

    init_script_debug_menu();
    assert(script_menu != nullptr);

    mString label {parms->str0};
    auto *result = new debug_menu_entry {label};

    auto *thread = stack.get_thread();
    auto *script = thread->get_executable()->get_owner()->get_parent();
    mString source {};
    THISCALL(
        0x005A34B0,
        script,
        vm_debug_menu_entry_garbage_collection_id,
        int(result),
        &source);
    script_menu->add_entry(result);

    SLF_RETURN;
    SLF_DONE;
#else
    bool (__fastcall *func)(const void *, void *, vm_stack *, entry_t) = CAST(func, 0x0067C1E0);
    return func(this, nullptr, &stack, entry);
#endif
}

slf__create_debug_menu_entry__str__str__t::slf__create_debug_menu_entry__str__str__t(const char *a3) : function(a3)
{
    m_vtbl = CAST(m_vtbl, 0x0089C70C);
    FUNC_ADDRESS(address, &slf__create_debug_menu_entry__str__str__t::operator());
    m_vtbl->__cl = CAST(m_vtbl->__cl, address);
}

bool slf__create_debug_menu_entry__str__str__t::operator()(vm_stack &stack, [[maybe_unused]]script_library_class::function::entry_t entry) const
{
    TRACE("slf__create_debug_menu_entry__str__str__t::operator()");

    if constexpr (1)
    {
        SLF_PARMS;

        init_script_debug_menu();
        assert(script_menu != nullptr);

        mString v14 {parms->str0};
        auto *result = new debug_menu_entry {v14};

        mString v15 {parms->str1};
        auto *nt = stack.get_thread();
        auto *v4 = nt->get_instance();
        result->set_script_handler(v4, v15);
        mString v16 {};
        uint32_t v11 = int(result);
        auto v10 = vm_debug_menu_entry_garbage_collection_id;
        auto *v6 = nt->get_executable();
        auto *so = v6->get_owner();
        auto *v8 = so->get_parent();

#ifdef OPENUSM_XBPACK_V10
        THISCALL(0x005A34B0, v8, v10, v11, &v16);
#else
        v8->add_allocated_stuff(v10, v11, v16);
#endif
        script_menu->add_entry(result);

        SLF_RETURN;
        SLF_DONE;
    }
    else
    {
        bool (__fastcall *func)(const void *, void *edx, vm_stack *, entry_t) = CAST(func, 0x00678210);
        return func(this, nullptr, &stack, entry);
    }
}

slf__create_progression_menu_entry__str__str__t::slf__create_progression_menu_entry__str__str__t(const char *a3) : function(a3)
{
    m_vtbl = CAST(m_vtbl, 0x0089C714);
    FUNC_ADDRESS(address, &slf__create_progression_menu_entry__str__str__t::operator());
    m_vtbl->__cl = CAST(m_vtbl->__cl, address);
}

bool slf__create_progression_menu_entry__str__str__t::operator()(vm_stack &stack, [[maybe_unused]]script_library_class::function::entry_t entry) const
{
    TRACE("slf__create_progression_menu_entry__str__str__t::operator()");

    SLF_PARMS;

    init_script_debug_menu();
    assert(progression_menu != nullptr);

    debug_menu_entry menu_entry {parms->str0};
    menu_entry.set_script_handler(stack.get_thread()->get_instance(), mString {parms->str1});
    progression_menu->add_entry(&menu_entry);

    int result = 0;
    SLF_RETURN;
    SLF_DONE;
}

void script_lib_debug_menu_patch()
{
    REDIRECT(0x0089C710, construct_debug_menu_lib);
}
