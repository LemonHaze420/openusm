#include "proximity_map_construction.h"

#include "func_wrapper.h"

proximity_map *create_static_proximity_map_on_the_stack(stack_allocator &a1,
                                                        _std::vector<proximity_map_construction_leaf> &a2,
                                                        subdivision_node_builder &a3, const vector3d &a4,
                                                        const vector3d &a5, int a6)
{
    return (proximity_map *)CDECL_CALL(0x0054E720, &a1, &a2, &a3, &a4, &a5, a6);
}
