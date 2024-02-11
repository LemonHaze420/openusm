#pragma once

#include "string_hash.h"
#include "variable.h"

struct script_object;
struct vm_thread;

namespace script {

//0x0064E4F0
extern int find_function(string_hash a1, const script_object *a2, bool a3);

inline Var<script_object *> gso {0x0096BB4C};

inline Var<script_object *> gsoi {0x0096BB50};

} // namespace script

extern void script_patch();
