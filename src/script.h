#pragma once

#include "string_hash.h"
#include "variable.h"

struct script_instance;
struct script_object;
struct vm_thread;

namespace script {

//0x0064E4F0
extern int find_function(string_hash a1, const script_object *a2, bool a3);

#if STANDALONE_SYSTEM
inline script_object *standalone_gso{};
inline script_instance *standalone_gsoi{};
inline script_object *&gso = standalone_gso;
inline script_instance *&gsoi = standalone_gsoi;
#else
inline Var<script_object *> gso{0x0096BB4C};
inline Var<script_instance *> gsoi{0x0096BB50};
#endif

inline script_instance *get_gsoi()
{
#if STANDALONE_SYSTEM
    return script::gsoi;
#else
    return script::gsoi();
#endif
}

inline script_object *get_gso()
{
#if STANDALONE_SYSTEM
    return script::gso;
#else
    return script::gso();
#endif
}

}  // namespace script

extern void script_patch();
