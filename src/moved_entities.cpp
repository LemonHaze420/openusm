#include "moved_entities.h"

#include "intraframe_trajectory.h"

#include "conglom.h"
#include "func_wrapper.h"
#include "oldmath_po.h"
#include "osassert.h"
#include "trace.h"
#include "utility.h"

#include <cassert>

void moved_entities::reset_all_moved()
{
    TRACE("moved_entities::reset_all_moved");

#if STANDALONE_SYSTEM
    for (int i = 0; i < moved_count; ++i) {
        auto *ent = moved_list[i].get_volatile_ptr();
        if (ent == nullptr) {
            continue;
        }

        ent->field_8 &= ~static_cast<uint32_t>(EXTFLAG_ALREADY_MOVED);
        if (ent->is_an_actor()) {
            auto *moved_actor = static_cast<actor *>(ent);
            moved_actor->invalidate_frame_delta();
            moved_actor->update_colgeom(nullptr);
        }
    }
    moved_count = 0;
#else
    CDECL_CALL(0x005125D0);
#endif
}

void moved_entities::add_moved(vhandle_type<entity> e_arg)
{
    TRACE("moved_entities::add_moved");
    
    if constexpr (1) {
        [[maybe_unused]] static vhandle_type<entity> INVALID_VHANDLE{};
        //assert(e_arg != INVALID_VHANDLE);

        auto *e = e_arg.get_volatile_ptr();
        assert(e != nullptr);

        assert(e->get_abs_po().is_valid());

        if (e->is_conglom_member()) {
            e = (entity *) e->get_conglom_owner();
            assert(e != nullptr && "Failed to obtain conglom owner in add_moved");
        }

        if (!e->is_flagged_in_the_moved_list()) {
            e->set_ext_flag_recursive_internal(static_cast<entity_ext_flag_t>(0x40), true);
            e->set_ext_flag_recursive_internal(static_cast<entity_ext_flag_t>(0x800000), true);

            if (moved_count >= 600) {
                if (moved_count == 600) {
                    for (auto i = 0; i < moved_count; ++i) {
                        auto v20 = moved_list[i];
                        if (v20.get_volatile_ptr() != nullptr) {
                            entity *v2 = v20.get_volatile_ptr();
                            auto v3 = v2->get_id();
                            auto *v4 = v3.to_string();

                            sp_log("Entity at slot %d: %s 0x%x\n", i, v4, v2);
                        } else {
                            sp_log("Destroyed entity at slot %d\n", i);
                        }
                    }
                } else {
                    error("Too many moved_entities this frame.");
                }
            } else {
                moved_list[moved_count++].field_0 = e->my_handle.field_0;

                assert("Cloned conglomerates should not be added to the moved list!" &&
                       !(e->is_a_conglomerate() && ((conglomerate *)e)->is_cloned_conglomerate()));
            }

            if (e->empty_adopted_children()) {
                auto *adopted_children = e->get_adopted_children();
                assert(adopted_children != nullptr);

                for (auto &child : (*adopted_children)) {
                    if ( child->is_an_actor() || child->is_a_pfx_entity() )
                        moved_entities::add_moved(vhandle_type<entity>{child->my_handle});
            }
        }
    }
    } else {
        CDECL_CALL(0x00533D00, e_arg);
    }
}

intraframe_trajectory_t *moved_entities::get_all_trajectories(
    Float frame_time, const moved_entities::trajectory_filter_t &)
{
    TRACE("moved_entities::get_all_trajectories");

    intraframe_trajectory_t *trajectories = nullptr;
    for (int index = 0; index < moved_count; ++index) {
        auto *ent = moved_list[index].get_volatile_ptr();
        if (ent == nullptr || !ent->is_an_actor() || ent->is_in_limbo() ||
            !ent->are_collisions_active() || ent->get_colgeom() == nullptr) {
            continue;
        }

        auto *storage = intraframe_trajectory_t::pool().allocate_new_block();
        auto *trajectory = new (storage) intraframe_trajectory_t(
            static_cast<actor *>(ent), frame_time, ent->get_abs_po(), nullptr);
        trajectory->field_15C = trajectories;
        trajectories = trajectory;
    }

    return trajectories;
}

void moved_entities_patch()
{
    SET_JUMP(0x00533D00, &moved_entities::add_moved);
}
