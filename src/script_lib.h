#pragma once

#include "script_library_class.h"

#include "color32.h"
#include "debug_string.h"
#include "script_lib_vector3d.h"
#include "vector3d.h"

#include <cstdint>

struct vm_stack;

struct slf__abs_delay__num__t : script_library_class::function {
    slf__abs_delay__num__t(const char *a3);

    bool operator()(vm_stack &stack, [[maybe_unused]] script_library_class::function::entry_t entry) const;
};

struct slf__acos__num__t : script_library_class::function {
    slf__acos__num__t(const char *a3);

    bool operator()(vm_stack &stack, [[maybe_unused]] script_library_class::function::entry_t entry) const;
};


struct slf__add_2d_debug_str__vector3d__vector3d__num__str__t : script_library_class::function {
    slf__add_2d_debug_str__vector3d__vector3d__num__str__t(const char *a3);

    bool operator()(vm_stack &stack, [[maybe_unused]] script_library_class::function::entry_t entry) const;
};

struct slf__add_2d_debug_str__vector3d__vector3d__num__str__num__t : script_library_class::function {
    slf__add_2d_debug_str__vector3d__vector3d__num__str__num__t(const char *a3);

    bool operator()(vm_stack &stack, [[maybe_unused]] script_library_class::function::entry_t entry) const;
};


struct slf__add_3d_debug_str__vector3d__vector3d__num__str__t : script_library_class::function {
    slf__add_3d_debug_str__vector3d__vector3d__num__str__t(const char *a3);

    struct parms_t {
        using vm_num_t = float;
        using vm_str_t = const char *;
        using vm_vector3d_t = vector3d;

        vm_vector3d_t field_0;
        vm_vector3d_t field_C;
        vm_num_t field_18;
        vm_str_t field_1C;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t);
};

struct slf__is_point_inside_glass_house__vector3d__t : script_library_class::function {
    slf__is_point_inside_glass_house__vector3d__t(const char *a3);

    struct parms_t {
        vector3d v;
    };

    //0x00661F70
    bool operator()(vm_stack &a1, script_library_class::function::entry_t) const;
};

struct slf__initialize_encounter_object__t : script_library_class::function {
    slf__initialize_encounter_object__t(const char *a3);

    bool operator()(vm_stack &stack, [[maybe_unused]] script_library_class::function::entry_t entry) const;
};
