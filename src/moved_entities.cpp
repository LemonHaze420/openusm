#include "moved_entities.h"

#include "func_wrapper.h"
#include "oldmath_po.h"
#include "conglom.h"
#include "trace.h"
#include "utility.h"

#include <cassert>

void moved_entities::reset_all_moved() {
    TRACE("moved_entities::reset_all_moved");

    CDECL_CALL(0x005125D0);
}

void moved_entities::add_moved(vhandle_type<entity> e_arg) {

    sp_log("add_moved");
    
    if constexpr(1)
    {
        vhandle_type<entity> INVALID_VHANDLE{};
        //assert(e_arg != INVALID_VHANDLE);

        auto *e = e_arg.get_volatile_ptr();
        assert(e != nullptr);

        assert(e->get_abs_po().is_valid());

        if ( !e->is_conglom_member() )
        {
            goto LABEL_44;
        }

        e = (entity *) e->get_conglom_owner();
        assert(e != nullptr && "Failed to obtain conglom owner in add_moved");

LABEL_44:
        if ( !e->is_flagged_in_the_moved_list() )
        {
            e->field_8 |= 0x40;
            e->field_8 |= 0x800000;

            if (moved_count() >= 600)
            {
                if (moved_count() == 600)  
                {
                    for ( auto i = 0; i < moved_count(); ++i )
                    {
                        auto v20 = moved_list()[i];
                        if ( v20.get_volatile_ptr() != nullptr )
                        {
                            entity *v2 = v20.get_volatile_ptr();
                            auto v3 = v2->get_id();
                            auto *v4 = v3.to_string();

                            sp_log("Entity at slot %d: %s 0x%x\n", i, v4, v2);
                        }
                        else
                        {
                            sp_log("Destroyed entity at slot %d\n", i);
                        }
                    }
                }
                else
                {
                    error("Too many moved_entities this frame.");
                }
            }
            else
            {
                moved_list()[moved_count()++].field_0 = e->my_handle.field_0;

                assert("Cloned conglomerates should not be added to the moved list!" && !(e->is_a_conglomerate() && ((conglomerate *) e)->is_cloned_conglomerate()));
            }

            if ( e->empty_adopted_children() )
            {
                auto *adopted_children = e->get_adopted_children();
                assert(adopted_children != nullptr);

                for (auto &child : (*adopted_children))
                {
                    if ( child->is_an_actor() || child->is_a_pfx_entity() )
                        moved_entities::add_moved(vhandle_type<entity>{child->my_handle});

                }

            }
        }
    }
    else
    {
        CDECL_CALL(0x00533D00, e_arg);
    }
}

intraframe_trajectory_t *moved_entities::get_all_trajectories(
    Float a1, const moved_entities::trajectory_filter_t *filter)
{
    TRACE("moved_entities::get_all_trajectories");

    return (intraframe_trajectory_t *) CDECL_CALL(0x0053F2A0, a1, filter);
}

void moved_entities_patch()
{
    SET_JUMP(0x00533D00, &moved_entities::add_moved);
    
}
