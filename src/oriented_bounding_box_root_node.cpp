#include "oriented_bounding_box_root_node.h"

#include "common.h"
#include "debug_render.h"
#include "os_developer_options.h"
#include "game.h"
#include "subdivision.h"
#include "subdivision_visitor.h"
#include "subdivision_obb.h"
#include "camera.h"
#include "trace.h"
#include "vtbl.h"

#include <vector.hpp>

VALIDATE_OFFSET(oriented_bounding_box_root_node, field_5C, 0x5C);

oriented_bounding_box_root_node::oriented_bounding_box_root_node() {}

struct visitor_t : subdivision_visitor {
    _std::vector<subdivision_node *> field_4;

    visitor_t();

    int visit(subdivision_node *a1)
    {
        if (subdivision_node_obb_base::visit_key() != bit_cast<subdivision_node_obb_base *>(a1)->visited) {
            bit_cast<subdivision_node_obb_base *>(this)->visited = subdivision_node_obb_base::visit_key();
            this->field_4.push_back(a1);
        }

        return 0;
    }
};

visitor_t::visitor_t()
{
    auto &func = get_vfunc(m_vtbl, 0x0);
    FUNC_ADDRESS(address, &visitor_t::visit);
    func = bit_cast<void *>(address);
}

void oriented_bounding_box_root_node::set_color([[maybe_unused]] color32 a2)
{
#if 0
    auto *v65 = this;
    if ( debug_render_get_ival(OBBS) == 1 ||
         os_developer_options::instance()->get_flag(mString{"SHOW_OBBS"}) ||
         os_developer_options::instance()->get_flag(mString{"SHOW_DISTRICTS"}) )
      
    {
        visitor_t visitor{};

        ++subdivision_node_obb_base::visit_key();
        vector3d min_extent, max_extent;
        auto *v2 = &v65->field_38;
        v2->get_extents(&min_extent, &max_extent);

        auto *v4 = g_game_ptr()->get_current_view_camera(0);
        auto v5 = v4->get_abs_position();
        auto v61 = v5;
        this->traverse_sphere(v61, 25.0, &visitor);

        for ( auto &v59 : visitor.field_4 )
        {
            sub_67923A(v59, a2, 0, 0.079999998);
            sub_690C2D(v59, 2);
        }

        if ( sub_65BD8E(&stru_158A024) )
        {
            v58 = sub_65BD8E(&stru_158A024);
            v7 = j_game::get_current_view_camera(g_game_ptr, &v28, 0);
            v8 = sub_6688A4(v7);
            v9 = j_entity_base::get_abs_position(&v8->base.base.base.base);
            j_vector3d::_as(&cam_pos, &v9->base);
            v56 = 2500.0;
            v55 = 4.0;
            sub_65F272(&stru_158A024, &begin);
            sub_65F272(&stru_158A840, &v53);
            sub_65F272(&stru_158A810, &v52);
            sub_66974A(&stru_158754C, (int)v51);
            sub_66974A(&stru_1589610, (int)v50);
            sub_65F272(&stru_1589768, &v49);
            sub_678D0D(&stru_158A024, &end);
            sub_678D0D(&stru_158A840, &v47);
            sub_678D0D(&stru_158A810, &v46);
            sub_6734CF(&stru_158754C, (int)v45);
            sub_6734CF(&stru_1589610, (int)v44);
            sub_678D0D(&stru_1589768, &v43);

            for ( j = 0; sub_692CBC(&begin, (int)&end); ++j )
            {
                v10 = sub_68F4C2(&begin);
                v11 = j_vector3d::minus(&v34, v10, &cam_pos);
                v41 = j_vector3d::length2(v11);

                if ( v41 > v55 && v56 > v41 )
                {
                    if ( *sub_69FFB1(v50) )
                    {
                    v22 = *j_color32::color32(&v35, 255, 0, 0, 255);
                    v18 = sub_68F4C2(&v53);
                    v12 = sub_68F4C2(&begin);
                    j_render_beam(v12, v18, v22, 0.050000001, 0);
                    if ( *sub_69FFB1(v51) )
                    {
                      v23 = *j_color32::color32(&v36, 0, 255, 0, 255);
                      v19 = sub_68F4C2(&v52);
                      v13 = sub_68F4C2(&begin);
                      j_render_beam(v13, v19, v23, 0.1, 0);
                    }
                    }
                    else
                    {
                        v26 = *j_color32::color32(&v37, 255, 0, 0, 255);
                        v25 = sub_68F4C2(&v53)->arr[0];
                        v14 = sub_68F4C2(&begin);
                        j_render_debug_hemisphere(v14, v25, v26);
                        if ( *sub_69FFB1(v51) )
                        {
                            v24 = *j_color32::color32(&v38, 0, 255, 0, 255);
                            v15 = sub_68F4C2(&v49);
                            v20 = j_vector3d::mul(&v39, v15, 5.0);
                            v16 = sub_68F4C2(&v52);
                            v21 = j_vector3d::plus_0(&v40, v16, v20);
                            v17 = sub_68F4C2(&v52);
                            render_beam(v17, v21, v24, 0.1, 0);
                        }
                    }

                    if ( j > 25 )
                        break;
                }

                sub_679B81(&begin, &v29, 0);
                sub_679B81(&v53, &v30, 0);
                sub_679B81(&v52, &v31, 0);
                sub_6A8454((int *)v51, (int)v32, 0);
                sub_6A8454((int *)v50, (int)v33, 0);
                sub_6A4674(&v49);
            }

            if ( sub_65BD8E(&stru_158A024) > 100 )
            {
                sub_683339(&stru_158A024);
                sub_683339(&stru_158A840);
                sub_683339(&stru_158A810);
                sub_68D7A8(&stru_158754C);
                sub_68D7A8(&stru_1589610);
                sub_683339(&stru_1589768);
            }

            while ( sub_65BD8E(&stru_158A024) > 10 )
            {
                sub_66A541(&stru_158A024);
                sub_66A541(&stru_158A840);
                sub_66A541(&stru_158A810);
                sub_6785F1(&stru_158754C);
                sub_6785F1(&stru_1589610);
                sub_66A541(&stru_1589768);
            }
        }
    }
#endif
}

void oriented_bounding_box_root_node::traverse_sphere(const vector3d &a2, Float a3, subdivision_visitor *a4)
{
    THISCALL(0x00522E50, this, &a2, a3, a4);
}

void oriented_bounding_box_root_node::un_mash(char *a2, int *image_size_used, region *)
{
    TRACE("oriented_bounding_box_root_node::un_mash");

    auto sub_68CB5A = [](int a1, int alignment) -> int {
        auto result = ~(alignment - 1) & (a1 + alignment - 1);
        assert((result & (alignment - 1)) == 0);

        return result;
    };

    auto v4 = sub_68CB5A((int)a2, 0x40);
    this->field_5C = CAST(this->field_5C, bit_cast<char *>(this->field_5C) + v4);
    this->field_6C += v4;
    this->field_20 += v4;

    auto sub_68D0D2 = [&sub_68CB5A](oriented_bounding_box_root_node *self) -> int {
        assert((uint32_t(self) & (SUBDIVISION_NODE_ALIGNMENT - 1)) == 0);
        return sub_68CB5A((int)&self[1], 4);
    };

    this->field_24 = sub_68D0D2(this);

    assert(image_size_used != nullptr);

    *image_size_used = this->field_60;
    this->field_60 = 0;
    this->field_64 = 0;
}
