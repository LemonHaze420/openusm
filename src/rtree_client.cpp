#include "rtree_client.h"

#include "variables.h"
#include "vector3d.h"

#include <cassert>

bool rtree_query_size_error_handler(const vector3d &a1, const vector3d &a2) {
    auto v2 = a1 - a2;
    auto v8 = (v2.xz_length2() < EPSILON);

    auto v3 = a1 - a2;
    auto v7 = v3.length2();
    if ((!v8 || v7 <= 1000000.0) && (v8 || v7 < 10000.0)) {
        return true;
    }

    sp_log(
        "rtree query size is too large. Please inspect the callstack and fix/reengineer the "
        "calling code. Probably line check"
        " needs to be clamped or split into shorter line checks and distributed over multiple "
        "frames. Thank you.");
    if (g_is_the_packer) {
        return true;
    }

    return false;
}
