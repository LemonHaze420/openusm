#include "subdivision_node.h"

#include "common.h"
#include "float.hpp"
#include "subdivision_visitor.h"
#include "vector3d.h"

VALIDATE_SIZE(subdivision_node, 0x1);

struct undefined_subdivision_methods {
    std::intptr_t m_vtbl = 0x00889470;

    /* virtual */ int traverse_line_segment(const subdivision_node &, const vector3d &, const vector3d &,
                                            subdivision_visitor &)
    {
        return 3;
    }

    /* virtual */ int traverse_sphere(const subdivision_node &, const vector3d &, Float, subdivision_visitor &)
    {
        return 3;
    }

    /* virtual */ int traverse_point(const subdivision_node &, const vector3d &, subdivision_visitor &)
    {
        return 3;
    }

    /* virtual */ int traverse_all(const subdivision_node &, subdivision_visitor &, bool)
    {
        return 3;
    }

    /* virtual */ void get_extents(const subdivision_node &, vector3d *, vector3d *) {}

    /* virtual */ void convert_endian(subdivision_node &, _nlPlatformEnum) {}
};
