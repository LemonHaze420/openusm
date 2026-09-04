#include "script_lib_anim.h"

#include "memory.h"

slc_anim_t *slc_anim = nullptr;

struct slf__anim__kill_anim__t : script_library_class::function {
    slf__anim__kill_anim__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AAE8;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__anim__pause__t : script_library_class::function {
    slf__anim__pause__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AAB8;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__anim__play__t : script_library_class::function {
    slf__anim__play__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AAD8;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__anim__set_fade_time__num__t : script_library_class::function {
    slf__anim__set_fade_time__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AAC8;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__anim__set_time__num__t : script_library_class::function {
    slf__anim__set_time__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AAD0;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__anim__set_timescale__num__t : script_library_class::function {
    slf__anim__set_timescale__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AAC0;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__anim__wait_finished__t : script_library_class::function {
    slf__anim__wait_finished__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AAE0;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

void register_anim_lib()
{
#define BUILD_SLF_NAME(klass, type) slf__##klass##__##type##__t

#define CREATE_SLF(KLASS, TYPE, NAME) \
    new (mem_alloc(sizeof(BUILD_SLF_NAME(KLASS, TYPE)))) BUILD_SLF_NAME(KLASS, TYPE){slc_anim, NAME}

    CREATE_SLF(anim, kill_anim, "kill_anim()");
    CREATE_SLF(anim, pause, "pause()");
    CREATE_SLF(anim, play, "play()");
    CREATE_SLF(anim, set_fade_time__num, "set_fade_time(num)");
    CREATE_SLF(anim, set_time__num, "set_time(num)");
    CREATE_SLF(anim, set_timescale__num, "set_timescale(num)");
    CREATE_SLF(anim, wait_finished, "wait_finished()");

#undef CREATE_SLF
#undef BUILD_SLF_NAME
}
