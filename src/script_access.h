#pragma once

#include "string_hash.h"
#include "variable.h"

struct script_object;
struct script_instance;
struct vm_thread;

namespace script {
inline Var<vm_thread *> thread{0x0096BB54};

script_instance *create_instance(string_hash a1, script_object *a2);
}  // namespace script

extern void script_access_patch();
