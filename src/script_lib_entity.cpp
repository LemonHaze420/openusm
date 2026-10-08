#include "script_lib_entity.h"

#include "actor.h"
#include "ai_voice_box_inode.h"
#include "ai_std_combat_target.h"
#include "als_animation_logic_system.h"
#include "conglom.h"
#include "damage_inode.h"
#include "damage_interface.h"
#include "signaller.h"
#include "state_machine.h"

#include "base_ai_core.h"
#include "base_ai_state_machine.h"
#include "entity_base.h"
#include "entity_base_vhandle.h"
#include "entity_handle_manager.h"
#include "mash_config.h"
#include "memory.h"
#include "oldmath_po.h"
#include "physical_interface.h"
#include "osassert.h"
#include "slc_manager.h"
#include "trace.h"
#include "utility.h"
#include "vm_stack.h"
#include "xbpack.h"
#include "marky_camera.h"
#include "vm_thread.h"
#include "wds.h"
#include "variant_interface.h"
#include "script_manager.h"
#include "time_interface.h"
#include "vtbl.h"

#include <cmath>
#include <type_traits>

template <typename T>
void bind_standalone_entity_slf(T *function)
{
    static std::decay_t<decltype(*function->m_vtbl)> native_vtable{};
    FUNC_ADDRESS(address, &T::operator());
    native_vtable.__cl = CAST(native_vtable.__cl, address);
    function->m_vtbl = &native_vtable;
}

ai::ai_core *get_ai_core_from_vhandle(entity_base_vhandle a1)
{
    auto *ent = a1.get_volatile_ptr();
    assert(ent != nullptr && "This entity is invalid!");

    if (ent == nullptr) {
        return nullptr;
    }

    assert(ent->is_an_actor() && "This entity is not an actor!");

    if (!ent->is_an_actor()) {
        return nullptr;
    }

    assert(bit_cast<actor *>(ent)->get_ai_core() && "This entity does not have an AI!");
    return ent->get_ai_core();
}

string_hash get_ai_param_hash_and_core(entity_base_vhandle a2, const char *a3, ai::ai_core **a4, bool a5)
{
    *a4 = get_ai_core_from_vhandle(a2);
    string_hash a1{a3};
    if (*a4) {
        if (a5) {
            auto *pb = (*a4)->get_param_block();
            if (!pb->does_parameter_exist(a1)) {
                auto *v8 = (*a4)->get_actor(0);
                auto id = v8->get_id();
                auto v10 = id.to_string();
                mString v16{0, "Unknown AI parameter %s requested by entity %s, from script", a3, v10};
                auto *v11 = v16.c_str();
                error(v11);
            }
        }
    }

    return a1;
}

struct slf__entity__abs_snap_to__entity__t : script_library_class::function {
    slf__entity__abs_snap_to__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE34;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__add_collision_ignorance__entity__t : script_library_class::function {
    slf__entity__add_collision_ignorance__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AF94;
    }

    struct parms_t {
        entity_base_vhandle me;
        entity_base_vhandle it;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        TRACE("slf__entity__add_collision_ignorance__entity__t::operator()");

        SLF_PARMS;

        auto *eb_parms_me = bit_cast<actor *>(parms->me.get_volatile_ptr());
        if (eb_parms_me != nullptr) {
            auto *eb_parms_it = parms->it.get_volatile_ptr();
            if (eb_parms_it != nullptr) {
                if (eb_parms_me->is_an_actor() && eb_parms_it->is_an_actor()) {
                    eb_parms_me->add_collision_ignorance(eb_parms_it->my_handle);
                }
            }
        }

        SLF_DONE;
    }
};

struct slf__entity__add_exclusive_interactor__string_hash__interactable_interface__t : script_library_class::function {
    slf__entity__add_exclusive_interactor__string_hash__interactable_interface__t(script_library_class *slc,
                                                                                  const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B5C4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__add_item__entity__t : script_library_class::function {
    slf__entity__add_item__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AF44;
    }

    struct parms_t {
        entity_base_vhandle me;
        entity_base_vhandle it;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        TRACE("slf__entity__add_item__entity__t::operator");

        SLF_PARMS;

        auto *eb_parms_me = parms->me.get_volatile_ptr();
        auto *eb_parms_it = parms->it.get_volatile_ptr();
        if (!eb_parms_me || !eb_parms_it) {
            SLF_DONE;
        }

        assert(eb_parms_it->get_flavor() == ENTITY_ITEM);

        if (eb_parms_me->is_an_actor()) {
            if (eb_parms_me->is_hero()) {
                sp_log("This doesn't actually give the item to the hero. Talk to a coder to find out the new way to do "
                       "this");
            } else {
                bit_cast<actor *>(eb_parms_me)->add_item(eb_parms_it->my_handle.field_0, 1);
            }
        }

        SLF_DONE;
    }
};

struct slf__entity__add_selectable_target__entity__t : script_library_class::function {
    slf__entity__add_selectable_target__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B17C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__add_vehicle_to_traffic_system__num__t : script_library_class::function {
    slf__entity__add_vehicle_to_traffic_system__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B474;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_get_viseme_morph_set__str__str__t : script_library_class::function {
    slf__entity__ai_get_viseme_morph_set__str__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B3F4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_get_viseme_stream__str__t : script_library_class::function {
    slf__entity__ai_get_viseme_stream__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B3EC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_is_speaking__t : script_library_class::function {
    slf__entity__ai_is_speaking__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B40C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_run_lip_sync__str__t : script_library_class::function {
    slf__entity__ai_run_lip_sync__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B3E4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_say_file__str__num__num__t : script_library_class::function {
    slf__entity__ai_say_file__str__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089B3CC;
    }

    struct parms_t {
        entity_base_vhandle owner;
        const char *sound;
        vm_num_t behavior;
        vm_num_t priority;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        float result = 0.0f;
        auto *owner = parms->owner.get_volatile_ptr();
        if (owner != nullptr && owner->is_an_actor()) {
            if (auto *core = owner->get_ai_core()) {
                auto *voice =
                    static_cast<ai::voice_box_inode *>(core->get_info_node(ai::voice_box_inode::default_id, false));
                if (voice != nullptr)
                    result = voice->say_file(string_hash{parms->sound},
                                             static_cast<int>(parms->behavior),
                                             static_cast<int>(parms->priority),
                                             nullptr)
                                 ? 1.0f
                                 : 0.0f;
            }
        }
        SLF_RETURN;
        return true;
    }
};

struct slf__entity__ai_say_gab__str__num__num__t : script_library_class::function {
    slf__entity__ai_say_gab__str__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B424;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_say_sound_group__str__num__num__t : script_library_class::function {
    slf__entity__ai_say_sound_group__str__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B414;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_shut_up__t : script_library_class::function {
    slf__entity__ai_shut_up__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089B404;
    }

    struct parms_t {
        entity_base_vhandle owner;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *owner = parms->owner.get_volatile_ptr();
        if (owner != nullptr && owner->is_an_actor()) {
            if (auto *core = owner->get_ai_core()) {
                auto *voice =
                    static_cast<ai::voice_box_inode *>(core->get_info_node(ai::voice_box_inode::default_id, false));
                if (voice != nullptr)
                    voice->shut_up();
            }
        }
        return true;
    }
};

struct slf__entity__ai_traffic_come_on_camera__vector3d__num__t : script_library_class::function {
    slf__entity__ai_traffic_come_on_camera__vector3d__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B2EC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_traffic_come_on_camera__vector3d__vector3d__num__t : script_library_class::function {
    slf__entity__ai_traffic_come_on_camera__vector3d__vector3d__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B2F4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_traffic_follow_entity__entity__num__t : script_library_class::function {
    slf__entity__ai_traffic_follow_entity__entity__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B2FC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_traffic_follow_vehicle__entity__t : script_library_class::function {
    slf__entity__ai_traffic_follow_vehicle__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B304;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_traffic_get_value__num__t : script_library_class::function {
    slf__entity__ai_traffic_get_value__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B2BC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_traffic_get_value__num__num__t : script_library_class::function {
    slf__entity__ai_traffic_get_value__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B2B4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_traffic_goto__vector3d__num__num__num__t : script_library_class::function {
    slf__entity__ai_traffic_goto__vector3d__num__num__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B2CC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_traffic_set_value__num__num__t : script_library_class::function {
    slf__entity__ai_traffic_set_value__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B2C4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_traffic_spawn_away_from__vector3d__num__t : script_library_class::function {
    slf__entity__ai_traffic_spawn_away_from__vector3d__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B2DC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_traffic_spawn_behind__entity__t : script_library_class::function {
    slf__entity__ai_traffic_spawn_behind__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B2E4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_traffic_spawn_near__vector3d__t : script_library_class::function {
    slf__entity__ai_traffic_spawn_near__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B2D4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_voice_box_set_team_respect__string_hash__num__t : script_library_class::function {
    slf__entity__ai_voice_box_set_team_respect__string_hash__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B3C4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_wait_say_file__str__num__num__t : script_library_class::function {
    slf__entity__ai_wait_say_file__str__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B3D4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_wait_say_gab__str__num__num__t : script_library_class::function {
    slf__entity__ai_wait_say_gab__str__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B42C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_wait_say_preregistered_file__str__num__num__t : script_library_class::function {
    slf__entity__ai_wait_say_preregistered_file__str__num__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B3DC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ai_wait_say_sound_group__str__num__num__t : script_library_class::function {
    slf__entity__ai_wait_say_sound_group__str__num__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089B41C;
    }

    struct parms_t {
        entity_base_vhandle owner;
        const char *sound;
        vm_num_t behavior;
        vm_num_t priority;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t entry) const
    {
        SLF_PARMS;
        float result = 0.0f;
        auto *owner = parms->owner.get_volatile_ptr();
        if (owner != nullptr && owner->is_an_actor()) {
            if (auto *core = owner->get_ai_core()) {
                auto *voice =
                    static_cast<ai::voice_box_inode *>(core->get_info_node(ai::voice_box_inode::default_id, false));
                if (voice != nullptr) {
                    auto *recall = reinterpret_cast<float *>(parms + 1);
                    if (entry == FIRST_ENTRY) {
                        *recall = voice->say_sound_group(string_hash{parms->sound},
                                                         static_cast<int>(parms->behavior),
                                                         static_cast<int>(parms->priority),
                                                         nullptr)
                                      ? 1.0f
                                      : 0.0f;
                        return false;
                    }
                    if (voice->is_speaking())
                        return false;
                    result = *recall;
                }
            }
        }
        SLF_RETURN;
        return true;
    }
};

struct slf__entity__anim_finished__t : script_library_class::function {
    slf__entity__anim_finished__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AEC4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__anim_finished__num__t : script_library_class::function {
    slf__entity__anim_finished__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AECC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__apply_continuous_rotation__vector3d__num__num__t : script_library_class::function {
    slf__entity__apply_continuous_rotation__vector3d__num__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089ACA4;
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
        vector3d axis;
        float speed;
        float detached;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        physical_interface::apply_continuous_rotation(parms->entity, parms->axis, parms->speed);
        if (!(parms->detached > 0.f))
            parms->entity.get_volatile_ptr();
        return true;
    }
};

struct slf__entity__apply_damage__num__t : script_library_class::function {
    slf__entity__apply_damage__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = CAST(m_vtbl, 0x0089ADBC);
#endif
    }

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        stack.pop(sizeof(entity_base_vhandle) + sizeof(vm_num_t));
        const auto *parameters = reinterpret_cast<const entity_base_vhandle *>(stack.get_SP());
        (void)parameters[0].get_volatile_ptr();
        return true;
    }
};

struct slf__entity__apply_directed_damage__num__vector3d__t : script_library_class::function {
    slf__entity__apply_directed_damage__num__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ADD4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__apply_directed_damage_cat__num__vector3d__str__num__t : script_library_class::function {
    slf__entity__apply_directed_damage_cat__num__vector3d__str__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089ADDC;
    }

    struct parms_t {
        entity_base_vhandle owner;
        vm_num_t damage;
        vector3d direction;
        const char *category;
        vm_num_t force_reaction;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *owner = parms->owner.get_volatile_ptr();
        if (owner != nullptr) {
            const string_hash category{parms->category};
            if (owner->is_an_actor()) {
                if (auto *core = owner->get_ai_core()) {
                    auto *damage =
                        static_cast<ai::damage_inode *>(core->get_info_node(ai::damage_inode::default_id, true));
                    damage->apply_forced_damage(static_cast<int>(parms->damage),
                                                parms->direction,
                                                category,
                                                std::not_equal_to<float>{}(parms->force_reaction, 0.0f));
                    return true;
                }
            }
            if (owner->has_damage_ifc()) {
                owner->damage_ifc()->apply_damage(nullptr,
                                                  parms->damage,
                                                  2,
                                                  ZEROVEC,
                                                  parms->direction,
                                                  0,
                                                  category,
                                                  string_hash{0},
                                                  string_hash{0},
                                                  false,
                                                  ZEROVEC,
                                                  17,
                                                  false);
            }
        }
        return true;
    }
};

struct slf__entity__apply_explosive_damage__num__vector3d__t : script_library_class::function {
    slf__entity__apply_explosive_damage__num__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089ADE4;
    }

    struct parms_t {
        entity_base_vhandle owner;
        vm_num_t damage;
        vector3d origin;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *owner = parms->owner.get_volatile_ptr();
        if (owner != nullptr && owner->has_damage_ifc()) {
            vector3d direction = owner->get_abs_position() - parms->origin;
            direction.normalize();
            static const string_hash category{"Enter_Prop_Physics"};
            owner->damage_ifc()->apply_damage(nullptr,
                                              parms->damage,
                                              6,
                                              parms->origin,
                                              direction,
                                              0,
                                              category,
                                              string_hash{0},
                                              string_hash{0},
                                              false,
                                              ZEROVEC,
                                              17,
                                              false);
        }
        return true;
    }
};

struct slf__entity__apply_explosive_damage__num__vector3d__vector3d__t : script_library_class::function {
    slf__entity__apply_explosive_damage__num__vector3d__vector3d__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ADEC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__apply_subdue__num__t : script_library_class::function {
    slf__entity__apply_subdue__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = CAST(m_vtbl, 0x0089ADC4);
#endif
    }

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        stack.pop(sizeof(entity_base_vhandle) + sizeof(vm_num_t));
        const auto *parameters = reinterpret_cast<const entity_base_vhandle *>(stack.get_SP());
        (void)parameters[0].get_volatile_ptr();
        return true;
    }
};

struct slf__entity__camera_get_target__t : script_library_class::function {
    slf__entity__camera_get_target__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE44;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__camera_orbit__vector3d__num__num__num__t : script_library_class::function {
    slf__entity__camera_orbit__vector3d__num__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE74;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__camera_set_collide_with_world__num__t : script_library_class::function {
    slf__entity__camera_set_collide_with_world__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE5C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__camera_set_roll__num__t : script_library_class::function {
    slf__entity__camera_set_roll__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE4C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__camera_set_target__vector3d__t : script_library_class::function {
    slf__entity__camera_set_target__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE3C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__camera_slide_to__vector3d__vector3d__num__num__t : script_library_class::function {
    slf__entity__camera_slide_to__vector3d__vector3d__num__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE64;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__camera_slide_to_orbit__vector3d__num__num__num__num__t : script_library_class::function {
    slf__entity__camera_slide_to_orbit__vector3d__num__num__num__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE6C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__cancel_tether__t : script_library_class::function {
    slf__entity__cancel_tether__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B1D4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__car_random_body_and_color__t : script_library_class::function {
    slf__entity__car_random_body_and_color__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B1C4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__change_ai_base_machine__str__t : script_library_class::function {
    slf__entity__change_ai_base_machine__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089B144;
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
        const char *graph;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t entry) const
    {
        SLF_PARMS;
        auto *entity_ptr = parms->entity.get_volatile_ptr();
        if (entity_ptr == nullptr) {
            stack.push(0.0f);
            return true;
        }
        if (!entity_ptr->is_an_actor())
            return true;
        auto *core = entity_ptr->get_ai_core();
        if (core == nullptr)
            return true;
        const resource_key graph{string_hash{parms->graph}, RESOURCE_KEY_TYPE_AI_STATE_GRAPH};
        if (entry == FIRST_ENTRY) {
            auto *attempts = reinterpret_cast<int *>(parms + 1);
            *attempts = 0;
            if (core->change_base_machine(graph, 6, string_hash{0}))
                return false;
            ++*attempts;
            if (core->my_base_machine == nullptr || core->my_base_machine->get_name() != graph) {
                if (*attempts > 20)
                    core->change_base_machine(graph, 6, string_hash{0});
                return false;
            }
        } else if (core->my_base_machine == nullptr || core->my_base_machine->get_name() != graph) {
            return false;
        }
        stack.push(1.0f);
        return true;
    }
};

struct slf__entity__collisions_enabled__t : script_library_class::function {
    slf__entity__collisions_enabled__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B43C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__compute_sector__t : script_library_class::function {
    slf__entity__compute_sector__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089ABE4;
    }

    struct parms_t {
        entity_base_vhandle owner;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *owner = parms->owner.get_volatile_ptr();
        if (owner != nullptr && owner->is_an_entity())
            static_cast<entity *>(owner)->compute_sector(g_world_ptr->the_terrain, false, nullptr);
        return true;
    }
};

struct slf__entity__create_damage_interface__t : script_library_class::function {
    slf__entity__create_damage_interface__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B34C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__create_interactable_ifc__t : script_library_class::function {
    slf__entity__create_interactable_ifc__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B5B4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__create_physical_interface__t : script_library_class::function {
    slf__entity__create_physical_interface__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B344;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__create_script_data_interface__t : script_library_class::function {
    slf__entity__create_script_data_interface__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B334;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__create_web_ifc__str__num__num__t : script_library_class::function {
    slf__entity__create_web_ifc__str__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B534;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__disable_as_target__t : script_library_class::function {
    slf__entity__disable_as_target__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B14C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__disable_collisions__t : script_library_class::function {
    slf__entity__disable_collisions__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AF84;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__disable_fading__t : script_library_class::function {
    slf__entity__disable_fading__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ADB4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__disgorge_items__t : script_library_class::function {
    slf__entity__disgorge_items__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AF74;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__enable_as_target__t : script_library_class::function {
    slf__entity__enable_as_target__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B154;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__enable_collisions__t : script_library_class::function {
    slf__entity__enable_collisions__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AF8C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__enable_collisions__num__t : script_library_class::function {
    slf__entity__enable_collisions__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B434;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__eye_check_ent__entity__t : script_library_class::function {
    slf__entity__eye_check_ent__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B054;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__eye_check_pos__vector3d__t : script_library_class::function {
    slf__entity__eye_check_pos__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B04C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__force_activate_interaction__string_hash__interactable_interface__t
    : script_library_class::function {
    slf__entity__force_activate_interaction__string_hash__interactable_interface__t(script_library_class *slc,
                                                                                    const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B62C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__force_current_region__t : script_library_class::function {
    slf__entity__force_current_region__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AD24;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__force_region__entity__t : script_library_class::function {
    slf__entity__force_region__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AD1C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_abs_position__t : script_library_class::function {
    slf__entity__get_abs_position__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = CAST(m_vtbl, 0x0089ABCC);
#endif
    }

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        stack.pop(sizeof(entity_base_vhandle));
        const auto handle = *reinterpret_cast<const entity_base_vhandle *>(stack.get_SP());
        vector3d result{};
        if (auto *entity_ptr = handle.get_volatile_ptr(); entity_ptr != nullptr) {
            result = entity_ptr->get_abs_position();
        }
        SLF_RETURN;
        return true;
    }
};

struct slf__entity__get_ai_base_machine_name__t : script_library_class::function {
    slf__entity__get_ai_base_machine_name__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B16C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_ai_param_float__str__t : script_library_class::function {
    slf__entity__get_ai_param_float__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B074;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_ai_param_hash__str__t : script_library_class::function {
    slf__entity__get_ai_param_hash__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B08C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_ai_param_int__str__t : script_library_class::function {
    slf__entity__get_ai_param_int__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B07C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_ai_param_str__str__t : script_library_class::function {
    slf__entity__get_ai_param_str__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B084;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_ai_param_vector3d__str__t : script_library_class::function {
    slf__entity__get_ai_param_vector3d__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B094;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_ai_signaller__t : script_library_class::function {
    slf__entity__get_ai_signaller__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AF6C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_anchor_point__t : script_library_class::function {
    slf__entity__get_anchor_point__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AC1C;
    }

    struct parms_t {
        entity_base_vhandle me;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;

        vector3d result{0, 0, 0};
        if (parms->me.get_volatile_ptr() != nullptr) {
            assert(0 && "This function has been disabled (for now?)");
        }

        SLF_RETURN;
        SLF_DONE;
    }
};

struct slf__entity__get_carry_slave__t : script_library_class::function {
    slf__entity__get_carry_slave__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B19C;
    }

    struct parms_t {
        entity_base_vhandle me;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;

        SLF_DONE;
    }
};

struct slf__entity__get_current_animation_name__t : script_library_class::function {
    slf__entity__get_current_animation_name__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B044;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_damage_force__t : script_library_class::function {
    slf__entity__get_damage_force__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ABD4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_debug_name__t : script_library_class::function {
    slf__entity__get_debug_name__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B444;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_detonate_position__t : script_library_class::function {
    slf__entity__get_detonate_position__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ABDC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_district__t : script_library_class::function {
    slf__entity__get_district__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B024;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_facing__t : script_library_class::function {
    slf__entity__get_facing__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089ABEC;
#endif
    }

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        stack.pop(sizeof(entity_base_vhandle));
        const auto handle = *reinterpret_cast<const entity_base_vhandle *>(stack.get_SP());
        vector3d result{};
        if (auto *entity_ptr = handle.get_volatile_ptr(); entity_ptr != nullptr) {
            const auto &transform = entity_ptr->get_abs_po();
            result = entity_ptr->get_flavor() == 10 ? transform.get_y_facing() : transform.get_z_facing();
        }
        SLF_RETURN;
        return true;
    }
};

struct slf__entity__get_fade_timer__t : script_library_class::function {
    slf__entity__get_fade_timer__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B1AC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_first_child__t : script_library_class::function {
    slf__entity__get_first_child__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AC4C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_hash_name__t : script_library_class::function {
    slf__entity__get_hash_name__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B44C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_hidey_pos__vector3d__num__t : script_library_class::function {
    slf__entity__get_hidey_pos__vector3d__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AC2C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_ifc_num__str__t : script_library_class::function {
    slf__entity__get_ifc_num__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089B284;
#endif
    }

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        struct parms_t {
            entity_base_vhandle owner;
            const char *path;
        };
        SLF_PARMS;
        float result = 0.0f;
        if (auto *owner = parms->owner.get_volatile_ptr()) {
            const auto key = create_resource_key_from_path(parms->path, RESOURCE_KEY_TYPE_IFC_ATTRIBUTE);
            owner->get_ifc_num(key, result, false);
        }
        SLF_RETURN;
        return true;
    }
};

struct slf__entity__get_ifc_str__str__t : script_library_class::function {
    slf__entity__get_ifc_str__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B2A4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_ifc_vec__str__t : script_library_class::function {
    slf__entity__get_ifc_vec__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B294;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_ifl_frame__t : script_library_class::function {
    slf__entity__get_ifl_frame__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B03C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_inode_param_float__str__str__t : script_library_class::function {
    slf__entity__get_inode_param_float__str__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B0DC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_inode_param_hash__str__str__t : script_library_class::function {
    slf__entity__get_inode_param_hash__str__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B0F4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_inode_param_int__str__str__t : script_library_class::function {
    slf__entity__get_inode_param_int__str__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B0E4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_inode_param_str__str__str__t : script_library_class::function {
    slf__entity__get_inode_param_str__str__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B0EC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_inode_param_vector3d__str__str__t : script_library_class::function {
    slf__entity__get_inode_param_vector3d__str__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B0FC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_interactable_ifc__t : script_library_class::function {
    slf__entity__get_interactable_ifc__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B5BC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_item__num__t : script_library_class::function {
    slf__entity__get_item__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AF54;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_item_by_name__str__t : script_library_class::function {
    slf__entity__get_item_by_name__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AF5C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_item_quantity__num__t : script_library_class::function {
    slf__entity__get_item_quantity__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AF64;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_last_anchor__t : script_library_class::function {
    slf__entity__get_last_anchor__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AC0C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_last_attacker__t : script_library_class::function {
    slf__entity__get_last_attacker__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AC14;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_last_item_used__t : script_library_class::function {
    slf__entity__get_last_item_used__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B20C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_member__str__t : script_library_class::function {
    slf__entity__get_member__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089AE8C;
    }

    struct parms_t {
        entity_base_vhandle owner;
        const char *member;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        entity_base_vhandle result{0};
        auto *owner = parms->owner.get_volatile_ptr();
        if (owner != nullptr && (owner->field_4 & 4) != 0) {
            if (auto *member = static_cast<conglomerate *>(owner)->get_member(string_hash{parms->member}, true))
                result = member->my_handle;
        }
        SLF_RETURN;
        return true;
    }
};

struct slf__entity__get_next_sibling__t : script_library_class::function {
    slf__entity__get_next_sibling__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AC54;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_num_items__t : script_library_class::function {
    slf__entity__get_num_items__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AF4C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_parent__t : script_library_class::function {
    slf__entity__get_parent__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AC44;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_pendulum_length__t : script_library_class::function {
    slf__entity__get_pendulum_length__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B3B4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_rel_position__t : script_library_class::function {
    slf__entity__get_rel_position__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AC24;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_rel_velocity__entity__t : script_library_class::function {
    slf__entity__get_rel_velocity__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AD04;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_render_alpha__t : script_library_class::function {
    slf__entity__get_render_alpha__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089B254;
#endif
    }

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        stack.pop(sizeof(entity_base_vhandle));
        const auto handle = *reinterpret_cast<const entity_base_vhandle *>(stack.get_SP());
        float result = 1.0f;
        if (auto *entity_ptr = handle.get_volatile_ptr(); entity_ptr != nullptr && entity_ptr->is_an_entity()) {
            const auto color = static_cast<entity *>(entity_ptr)->get_render_color();
            result = static_cast<float>(static_cast<unsigned char>(color.field_0[3]) * 0.0039215689);
        }
        SLF_RETURN;
        return true;
    }
};

struct slf__entity__get_render_color__t : script_library_class::function {
    slf__entity__get_render_color__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B24C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_scripted_target__t : script_library_class::function {
    slf__entity__get_scripted_target__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B194;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_sector_name__t : script_library_class::function {
    slf__entity__get_sector_name__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AD14;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_state__num__t : script_library_class::function {
    slf__entity__get_state__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B56C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_time_dilation__t : script_library_class::function {
    slf__entity__get_time_dilation__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B314;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_time_mode__t : script_library_class::function {
    slf__entity__get_time_mode__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B324;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_x_facing__t : script_library_class::function {
    slf__entity__get_x_facing__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ABF4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_y_facing__t : script_library_class::function {
    slf__entity__get_y_facing__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ABFC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__get_z_facing__t : script_library_class::function {
    slf__entity__get_z_facing__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AC04;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__has_carry_slave__t : script_library_class::function {
    slf__entity__has_carry_slave__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B1A4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__has_member__str__t : script_library_class::function {
    slf__entity__has_member__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE94;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__has_script_data_interface__t : script_library_class::function {
    slf__entity__has_script_data_interface__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B33C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__hates__entity__t : script_library_class::function {
    slf__entity__hates__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B51C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ifl_damage_lock__num__t : script_library_class::function {
    slf__entity__ifl_damage_lock__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B004;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ifl_lock__num__t : script_library_class::function {
    slf__entity__ifl_lock__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089B00C;
    }

    struct parms_t {
        entity_base_vhandle owner;
        vm_num_t frame;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *owner = parms->owner.get_volatile_ptr();
        if (owner != nullptr && owner->is_an_actor())
            static_cast<actor *>(owner)->ifl_lock(static_cast<int>(parms->frame));
        return true;
    }
};

struct slf__entity__ifl_pause__t : script_library_class::function {
    slf__entity__ifl_pause__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B014;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__ifl_play__t : script_library_class::function {
    slf__entity__ifl_play__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089B01C;
#endif
    }

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        stack.pop(sizeof(entity_base_vhandle));
        const auto handle = *reinterpret_cast<const entity_base_vhandle *>(stack.get_SP());
        auto *value = handle.get_volatile_ptr();
        if (value != nullptr && value->is_an_actor())
            static_cast<actor *>(value)->ifl_play();
        return true;
    }
};

struct slf__entity__in_sector__vector3d__vector3d__num__t : script_library_class::function {
    slf__entity__in_sector__vector3d__vector3d__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AD0C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__inhibit_universal_soldier_ability__str__num__t : script_library_class::function {
    slf__entity__inhibit_universal_soldier_ability__str__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B174;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__invoke_facial_expression__num__num__num__num__t : script_library_class::function {
    slf__entity__invoke_facial_expression__num__num__num__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B3FC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__is_a_car__t : script_library_class::function {
    slf__entity__is_a_car__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        static std::decay_t<decltype(*m_vtbl)> native_vtable{};
        FUNC_ADDRESS(address, &slf__entity__is_a_car__t::operator());
        native_vtable.__cl = CAST(native_vtable.__cl, address);
        m_vtbl = &native_vtable;
#else
        m_vtbl = CAST(m_vtbl, 0x0089AD5C);
#endif
    }

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        stack.pop(sizeof(entity_base_vhandle));
        const auto entity_handle = *reinterpret_cast<const entity_base_vhandle *>(stack.get_SP());
        const auto *entity_ptr = entity_handle.get_volatile_ptr();
        float result = entity_ptr != nullptr && entity_ptr->is_flagged(0x800u) ? 1.0f : 0.0f;
        SLF_RETURN;
        return true;
    }
};

struct slf__entity__is_picked_up__t : script_library_class::function {
    slf__entity__is_picked_up__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE1C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__is_suspended__t : script_library_class::function {
    slf__entity__is_suspended__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B45C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__is_throwable__t : script_library_class::function {
    slf__entity__is_throwable__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AD64;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__is_valid__t : script_library_class::function {
    slf__entity__is_valid__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089B464;
#endif
    }

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        stack.pop(sizeof(entity_base_vhandle));
        const auto handle = *reinterpret_cast<const entity_base_vhandle *>(stack.get_SP());
        float result = handle.get_volatile_ptr() != nullptr ? 1.0f : 0.0f;
        SLF_RETURN;
        return true;
    }
};

struct slf__entity__is_visible__t : script_library_class::function {
    slf__entity__is_visible__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089AD74;
    }

    struct parms_t {
        entity_base_vhandle owner;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *owner = parms->owner.get_volatile_ptr();
        float result = owner != nullptr && (owner->field_4 & 0x200) != 0 ? 1.0f : 0.0f;
        SLF_RETURN;
        return true;
    }
};

struct slf__entity__kill_anim_in_slot__num__t : script_library_class::function {
    slf__entity__kill_anim_in_slot__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AEBC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__likes__entity__t : script_library_class::function {
    slf__entity__likes__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B524;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__look_at__vector3d__t : script_library_class::function {
    slf__entity__look_at__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AC7C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__motion_blur_off__t : script_library_class::function {
    slf__entity__motion_blur_off__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ADFC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__motion_blur_on__num__num__num__t : script_library_class::function {
    slf__entity__motion_blur_on__num__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ADF4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__motion_trail_off__t : script_library_class::function {
    slf__entity__motion_trail_off__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE14;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__motion_trail_on__entity__entity__vector3d__num__num__num__num__t : script_library_class::function {
    slf__entity__motion_trail_on__entity__entity__vector3d__num__num__num__num__t(script_library_class *slc,
                                                                                  const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE04;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__motion_trail_on__entity__str__num__num__vector3d__num__num__num__num__t
    : script_library_class::function {
    slf__entity__motion_trail_on__entity__str__num__num__vector3d__num__num__num__num__t(script_library_class *slc,
                                                                                         const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE0C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__neutral__entity__t : script_library_class::function {
    slf__entity__neutral__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B52C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__operator_not_equals__entity__t : script_library_class::function {
    slf__entity__operator_not_equals__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = CAST(m_vtbl, 0x0089AD3C);
#endif
    }

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        stack.pop(2 * sizeof(entity_base_vhandle));
        const auto *handles = reinterpret_cast<const entity_base_vhandle *>(stack.get_SP());
        float result = handles[0] != handles[1] ? 1.0f : 0.0f;
        SLF_RETURN;
        return true;
    }
};

struct slf__entity__operator_equals_equals__entity__t : script_library_class::function {
    slf__entity__operator_equals_equals__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = CAST(m_vtbl, 0x0089AD34);
#endif
    }

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        stack.pop(2 * sizeof(entity_base_vhandle));
        const auto *handles = reinterpret_cast<const entity_base_vhandle *>(stack.get_SP());
        float result = handles[0] == handles[1] ? 1.0f : 0.0f;
        SLF_RETURN;
        return true;
    }
};

struct slf__entity__physical_ifc_add_particle__str__t : script_library_class::function {
    slf__entity__physical_ifc_add_particle__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B4A4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__physical_ifc_apply_force__vector3d__num__t : script_library_class::function {
    slf__entity__physical_ifc_apply_force__vector3d__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B354;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__physical_ifc_cancel_all_velocity__t : script_library_class::function {
    slf__entity__physical_ifc_cancel_all_velocity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = CAST(m_vtbl, 0x0089B36C);
#endif
    }

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        stack.pop(sizeof(entity_base_vhandle));
        const auto handle = *reinterpret_cast<const entity_base_vhandle *>(stack.get_SP());
        if (auto *entity_ptr = handle.get_volatile_ptr(); entity_ptr != nullptr && entity_ptr->has_physical_ifc()) {
            entity_ptr->physical_ifc()->set_velocity(ZEROVEC, false);
        }
        return true;
    }
};

struct slf__entity__physical_ifc_clear_pendulum__t : script_library_class::function {
    slf__entity__physical_ifc_clear_pendulum__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B3A4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__physical_ifc_get_attached_particle_name__t : script_library_class::function {
    slf__entity__physical_ifc_get_attached_particle_name__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B4BC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__physical_ifc_get_bounce_particle_name__t : script_library_class::function {
    slf__entity__physical_ifc_get_bounce_particle_name__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B4AC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__physical_ifc_is_biped_physics_running__t : script_library_class::function {
    slf__entity__physical_ifc_is_biped_physics_running__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089B374;
    }

    struct parms_t {
        entity_base_vhandle owner;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *owner = parms->owner.get_volatile_ptr();
        float result = owner != nullptr && owner->has_physical_ifc() && (owner->physical_ifc()->field_C & 0x80000) != 0
                           ? 1.0f
                           : 0.0f;
        SLF_RETURN;
        return true;
    }
};

struct slf__entity__physical_ifc_is_effectively_standing__t : script_library_class::function {
    slf__entity__physical_ifc_is_effectively_standing__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B35C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__physical_ifc_is_prop_physics_at_rest__t : script_library_class::function {
    slf__entity__physical_ifc_is_prop_physics_at_rest__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B4CC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__physical_ifc_is_prop_physics_running__t : script_library_class::function {
    slf__entity__physical_ifc_is_prop_physics_running__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B4D4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__physical_ifc_manage_standing__num__t : script_library_class::function {
    slf__entity__physical_ifc_manage_standing__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B364;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__physical_ifc_set_allow_biped_physics__num__t : script_library_class::function {
    slf__entity__physical_ifc_set_allow_biped_physics__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B38C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__physical_ifc_set_attached_particle_name__str__t : script_library_class::function {
    slf__entity__physical_ifc_set_attached_particle_name__str__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B4C4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__physical_ifc_set_bounce_particle_name__str__t : script_library_class::function {
    slf__entity__physical_ifc_set_bounce_particle_name__str__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B4B4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__physical_ifc_set_pendulum__entity__num__t : script_library_class::function {
    slf__entity__physical_ifc_set_pendulum__entity__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B39C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__physical_ifc_set_pendulum__vector3d__num__t : script_library_class::function {
    slf__entity__physical_ifc_set_pendulum__vector3d__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B394;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__physical_ifc_start_biped_physics__t : script_library_class::function {
    slf__entity__physical_ifc_start_biped_physics__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B37C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__physical_ifc_start_prop_physics__vector3d__num__t : script_library_class::function {
    slf__entity__physical_ifc_start_prop_physics__vector3d__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B494;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__physical_ifc_stop_biped_physics__t : script_library_class::function {
    slf__entity__physical_ifc_stop_biped_physics__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089B384;
    }

    struct parms_t {
        entity_base_vhandle owner;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *owner = parms->owner.get_volatile_ptr();
        if (owner != nullptr && owner->has_physical_ifc() && (owner->physical_ifc()->field_C & 0x80000) != 0 &&
            (owner->field_4 & 4) != 0) {
            if (auto *system = static_cast<conglomerate *>(owner)->get_my_als()) {
                if (auto *layer = system->get_als_layer(static_cast<als::layer_types>(0))) {
                    layer->force_als_state(string_hash{"Idle_No_Blend"}, static_cast<int>(0xDEADBEEF));
                    system->force_update();
                    owner->physical_ifc()->manage_standing(true);
                    if (auto *core = get_ai_core_from_vhandle(parms->owner))
                        core->reset_base_machine(string_hash{0});
                }
            }
        }
        return true;
    }
};

struct slf__entity__physical_ifc_stop_prop_physics__t : script_library_class::function {
    slf__entity__physical_ifc_stop_prop_physics__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B49C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__play_anim__str__t : script_library_class::function {
    slf__entity__play_anim__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AEA4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__play_anim__str__num__num__t : script_library_class::function {
    slf__entity__play_anim__str__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AEAC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__poison__num__num__t : script_library_class::function {
    slf__entity__poison__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B1DC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__pop_ai_base_machine__t : script_library_class::function {
    slf__entity__pop_ai_base_machine__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B164;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__pre_roll__num__t : script_library_class::function {
    slf__entity__pre_roll__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B55C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__push_ai_base_machine__str__t : script_library_class::function {
    slf__entity__push_ai_base_machine__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B15C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__randomize_position__vector3d__num__num__num__t : script_library_class::function {
    slf__entity__randomize_position__vector3d__num__num__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B4DC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__regenerate__num__num__t : script_library_class::function {
    slf__entity__regenerate__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B1E4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__rel_angle__vector3d__t : script_library_class::function {
    slf__entity__rel_angle__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AC34;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__remove_collision_ignorance__entity__t : script_library_class::function {
    slf__entity__remove_collision_ignorance__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AF9C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__remove_exclusive_interactor__string_hash__interactable_interface__t
    : script_library_class::function {
    slf__entity__remove_exclusive_interactor__string_hash__interactable_interface__t(script_library_class *slc,
                                                                                     const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B5CC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__remove_selectable_target__entity__t : script_library_class::function {
    slf__entity__remove_selectable_target__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B184;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__remove_vehicle_from_traffic_system__t : script_library_class::function {
    slf__entity__remove_vehicle_from_traffic_system__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B47C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__render_name__num__t : script_library_class::function {
    slf__entity__render_name__num__t(script_library_class *slc, const char *name) : function(slc, name)
    {
        m_vtbl = CAST(m_vtbl, 0x0089B47C);
        auto local_vtbl = CAST(m_vtbl, mem_alloc(sizeof(*m_vtbl)));
        *local_vtbl = *m_vtbl;
        FUNC_ADDRESS(address, &slf__entity__render_name__num__t::operator());
        local_vtbl->__cl = CAST(local_vtbl->__cl, address);
        m_vtbl = local_vtbl;
    }

    struct parms_t {
        entity_base_vhandle me;
        vm_num_t enabled;
    };

    bool operator()(vm_stack &stack, entry_t) const
    {
        SLF_PARMS;

        auto *entity = parms->me.get_volatile_ptr();
        if (entity != nullptr && std::fpclassify(parms->enabled) != FP_ZERO) {
            (void)entity->get_id().to_string();
        }

        return true;
    }
};

struct slf__entity__reset_ai__t : script_library_class::function {
    slf__entity__reset_ai__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM) {
            bind_standalone_entity_slf(this);
        } else {
            m_vtbl = CAST(m_vtbl, 0x0089B06C);
        }
    }

    struct parms_t {
        entity_base_vhandle me;
    };

    bool operator()(vm_stack &stack, entry_t) const
    {
        SLF_PARMS;
        auto *entity = parms->me.get_volatile_ptr();
        if (entity != nullptr && entity->is_an_actor()) {
            if (auto *core = entity->get_ai_core(); core != nullptr)
                core->reset_base_machine(string_hash{0});
        }
        return true;
    }
};

struct slf__entity__restart__t : script_library_class::function {
    slf__entity__restart__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B564;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__seriously_kill__t : script_library_class::function {
    slf__entity__seriously_kill__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ADCC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_abs_xz_facing__vector3d__t : script_library_class::function {
    slf__entity__set_abs_xz_facing__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089AC94;
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
        vector3d target;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *entity_ptr = parms->entity.get_volatile_ptr();
        if (entity_ptr == nullptr)
            return true;
        auto *marky_camera = g_world_ptr->field_28.field_44;
        if (entity_ptr == marky_camera &&
            std::not_equal_to<float>{}(stack.get_thread()->field_1E0, marky_camera->field_1D8))
            return true;
        const vector3d position = entity_ptr->get_abs_position();
        vector3d facing = parms->target - position;
        facing.y = 0.0f;
        facing.normalize();
        vector3d right = vector3d::cross(YVEC, facing);
        right.normalize();
        po transform;
        transform.set_po(right, YVEC, facing, entity_ptr->get_abs_position());
        if (auto *parent = entity_ptr->get_parent(); parent != nullptr) {
            po relative;
            const ptr_to_po source{&transform.m, &parent->get_abs_po().inverse()->m};
            relative.set_from_ptr_to_po_world(source);
            entity_ptr->set_abs_po(relative);
        } else {
            entity_ptr->set_abs_po(transform);
        }
        return true;
    }
};

struct slf__entity__set_active__num__t : script_library_class::function {
    slf__entity__set_active__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AD9C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_ai_param_float__str__num__t : script_library_class::function {
    slf__entity__set_ai_param_float__str__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089B09C;
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
        const char *name;
        vm_num_t value;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        ai::ai_core *core;
        const auto name = get_ai_param_hash_and_core(parms->entity, parms->name, &core, false);
        if (core != nullptr)
            core->field_50.set_pb_float(name, parms->value, true);
        return true;
    }
};

struct slf__entity__set_ai_param_float_variance__str__num__num__t : script_library_class::function {
    slf__entity__set_ai_param_float_variance__str__num__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B0A4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_ai_param_hash__str__num__t : script_library_class::function {
    slf__entity__set_ai_param_hash__str__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B0C4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_ai_param_hash__str__str__t : script_library_class::function {
    slf__entity__set_ai_param_hash__str__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B0BC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_ai_param_hash__str__string_hash__t : script_library_class::function {
    slf__entity__set_ai_param_hash__str__string_hash__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B0CC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_ai_param_int__str__num__t : script_library_class::function {
    slf__entity__set_ai_param_int__str__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089B0AC;
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
        const char *name;
        vm_num_t value;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        ai::ai_core *core;
        const auto name = get_ai_param_hash_and_core(parms->entity, parms->name, &core, false);
        if (core != nullptr)
            core->field_50.set_pb_int(name, static_cast<int>(parms->value), true);
        return true;
    }
};

struct slf__entity__set_ai_param_str__str__str__t : script_library_class::function {
    slf__entity__set_ai_param_str__str__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B0B4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_ai_param_vector3d__str__vector3d__t : script_library_class::function {
    slf__entity__set_ai_param_vector3d__str__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B0D4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_ambient_factor__vector3d__t : script_library_class::function {
    slf__entity__set_ambient_factor__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B32C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_anchor_activated__num__t : script_library_class::function {
    slf__entity__set_anchor_activated__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AFA4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_car_combat_info__num__num__num__num__num__num__num__num__t : script_library_class::function {
    slf__entity__set_car_combat_info__num__num__num__num__num__num__num__num__t(script_library_class *slc,
                                                                                const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B1BC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_crawlable__num__t : script_library_class::function {
    slf__entity__set_crawlable__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AD84;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_default_variant__t : script_library_class::function {
    slf__entity__set_default_variant__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089B544;
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *entity_ptr = parms->entity.get_volatile_ptr();
        entity_ptr->variant_ifc()->apply_variant(string_hash{"__default"});
        return true;
    }
};

struct slf__entity__set_distance_clip__num__t : script_library_class::function {
    slf__entity__set_distance_clip__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089ADAC;
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
        float distance;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        if (auto *owner = parms->entity.get_volatile_ptr()) {
            owner->set_fade_distance(parms->distance);
        }
        return true;
    }
};

struct slf__entity__set_entity_blur__num__t : script_library_class::function {
    slf__entity__set_entity_blur__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089B224;
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
        float blur;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        (void)parms->entity.get_volatile_ptr();
        return true;
    }
};

struct slf__entity__set_facing__vector3d__vector3d__t : script_library_class::function {
    slf__entity__set_facing__vector3d__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        bind_standalone_entity_slf(this);
    }

    struct parms_t {
        entity_base_vhandle entity;
        vector3d facing;
        vector3d up;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        if (auto *entity_ptr = parms->entity.get_volatile_ptr(); entity_ptr != nullptr) {
            auto transform = entity_ptr->get_rel_po();
            transform.set_po(parms->facing, parms->up, transform.get_position());
            entity_ptr->set_abs_po(transform);
        }
        return true;
    }
};

struct slf__entity__set_fade_timer__num__t : script_library_class::function {
    slf__entity__set_fade_timer__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B1B4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_hires_shadow__num__t : script_library_class::function {
    slf__entity__set_hires_shadow__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AD4C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_ifc_num__str__num__t : script_library_class::function {
    slf__entity__set_ifc_num__str__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089B28C;
#endif
    }

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        struct parms_t {
            entity_base_vhandle owner;
            const char *path;
            float value;
        };
        SLF_PARMS;
        if (auto *owner = parms->owner.get_volatile_ptr()) {
            const auto key = create_resource_key_from_path(parms->path, RESOURCE_KEY_TYPE_IFC_ATTRIBUTE);
            owner->set_ifc_num(key, parms->value, false);
        }
        return true;
    }
};

struct slf__entity__set_ifc_str__str__str__t : script_library_class::function {
    slf__entity__set_ifc_str__str__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B2AC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_ifc_vec__str__vector3d__t : script_library_class::function {
    slf__entity__set_ifc_vec__str__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B29C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_ignore_limbo__num__t : script_library_class::function {
    slf__entity__set_ignore_limbo__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089B4E4;
#endif
    }

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        struct parms_t {
            entity_base_vhandle owner;
            float ignore_limbo;
        };
        SLF_PARMS;
        auto *owner = parms->owner.get_volatile_ptr();
        if (equal(parms->ignore_limbo, 0.0f))
            owner->field_4 &= ~0x8u;
        else
            owner->field_4 |= 0x8u;
        return true;
    }
};

struct slf__entity__set_immobile__num__t : script_library_class::function {
    slf__entity__set_immobile__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089B484;
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
        vm_num_t immobile;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        if (auto *entity_ptr = parms->entity.get_volatile_ptr();
            entity_ptr != nullptr && entity_ptr->has_physical_ifc()) {
            auto *physical = entity_ptr->physical_ifc();
            if (std::not_equal_to<float>{}(parms->immobile, 0.0f))
                physical->field_C |= 0x100;
            else
                physical->field_C &= ~0x100;
        }
        return true;
    }
};

struct slf__entity__set_inode_param_entity__str__str__entity__t : script_library_class::function {
    slf__entity__set_inode_param_entity__str__str__entity__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B11C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_inode_param_float__str__str__num__t : script_library_class::function {
    slf__entity__set_inode_param_float__str__str__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B104;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_inode_param_float_variance__str__str__num__num__t : script_library_class::function {
    slf__entity__set_inode_param_float_variance__str__str__num__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B10C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_inode_param_hash__str__str__num__t : script_library_class::function {
    slf__entity__set_inode_param_hash__str__str__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B134;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_inode_param_hash__str__str__str__t : script_library_class::function {
    slf__entity__set_inode_param_hash__str__str__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B12C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_inode_param_int__str__str__num__t : script_library_class::function {
    slf__entity__set_inode_param_int__str__str__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B114;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_inode_param_str__str__str__str__t : script_library_class::function {
    slf__entity__set_inode_param_str__str__str__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B124;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_inode_param_vector3d__str__str__vector3d__t : script_library_class::function {
    slf__entity__set_inode_param_vector3d__str__str__vector3d__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B13C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_invulnerable__num__t : script_library_class::function {
    slf__entity__set_invulnerable__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089AD44;
    }

    struct parms_t {
        entity_base_vhandle owner;
        vm_num_t invulnerable;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        if (auto *owner = parms->owner.get_volatile_ptr()) {
            if (std::not_equal_to<float>{}(parms->invulnerable, 0.0f))
                owner->field_8 |= 0x4000;
            else
                owner->field_8 &= ~0x4000;
        }
        return true;
    }
};

struct slf__entity__set_kill_ent_on_destroy__num__t : script_library_class::function {
    slf__entity__set_kill_ent_on_destroy__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B48C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_member_hidden__num__t : script_library_class::function {
    slf__entity__set_member_hidden__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE9C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_parent__entity__t : script_library_class::function {
    slf__entity__set_parent__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089AC3C;
    }

    struct parms_t {
        entity_base_vhandle owner;
        entity_base_vhandle parent;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *owner = parms->owner.get_volatile_ptr();
        auto *parent = parms->parent.get_volatile_ptr();
        if (owner == nullptr || (owner->field_4 & 0x8000) != 0)
            return true;
        auto *camera = g_world_ptr->field_28.field_44;
        if (owner == camera && std::not_equal_to<float>{}(stack.get_thread()->field_1E0, camera->field_1D8))
            return true;
        if (parent != nullptr) {
            owner->set_parent(parent);
            owner->set_abs_position(ZEROVEC);
        } else {
            owner->clear_parent(true);
        }
        if (owner->is_an_entity()) {
            auto *entity = static_cast<::entity *>(owner);
            entity->compute_sector(g_world_ptr->the_terrain, false, nullptr);
            entity->update_proximity_maps();
        }
        return true;
    }
};

struct slf__entity__set_parent_rel__entity__t : script_library_class::function {
    slf__entity__set_parent_rel__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AC5C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_path_graph__str__t : script_library_class::function {
    slf__entity__set_path_graph__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B214;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_path_graph_start_node__num__t : script_library_class::function {
    slf__entity__set_path_graph_start_node__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B21C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_pendulum_attach_limb__num__t : script_library_class::function {
    slf__entity__set_pendulum_attach_limb__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B3BC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_pendulum_length__num__t : script_library_class::function {
    slf__entity__set_pendulum_length__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = CAST(m_vtbl, 0x0089B3AC);
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
        vm_num_t length;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        if (auto *entity_ptr = parms->entity.get_volatile_ptr();
            entity_ptr != nullptr && entity_ptr->has_physical_ifc()) {
            entity_ptr->physical_ifc()->field_140 = bit_cast<int>(parms->length);
        }
        return true;
    }
};

struct slf__entity__set_physical__num__t : script_library_class::function {
    slf__entity__set_physical__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ADA4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_po_facing__vector3d__t : script_library_class::function {
    slf__entity__set_po_facing__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = CAST(m_vtbl, 0x0089AC9C);
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
        vector3d facing;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        if (auto *entity_ptr = parms->entity.get_volatile_ptr(); entity_ptr != nullptr) {
            auto *camera = g_world_ptr->field_28.field_44;
            if (entity_ptr == camera && std::not_equal_to<float>{}(stack.get_thread()->field_1E0, camera->field_1D8))
                return true;
            po transform;
            auto facing = parms->facing;
            facing.normalize();
            transform.set_facing(facing);
            transform.set_position(entity_ptr->get_abs_position());
            if (auto *parent = entity_ptr->get_parent()) {
                const ptr_to_po source{&transform.m, &parent->get_abs_po().inverse()->m};
                transform.set_from_ptr_to_po_world(source);
            }
            entity_ptr->set_abs_po(transform);
        }
        return true;
    }
};

struct slf__entity__set_rel_position__vector3d__t : script_library_class::function {
    slf__entity__set_rel_position__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089AC64;
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
        vector3d position;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *entity_ptr = parms->entity.get_volatile_ptr();
        if (entity_ptr == nullptr) {
            return true;
        }

        auto *marky_camera = g_world_ptr->field_28.field_44;
        if (entity_ptr == marky_camera &&
            std::not_equal_to<float>{}(stack.get_thread()->field_1E0, marky_camera->field_1D8)) {
            return true;
        }

        physical_interface *physical = nullptr;
        if (entity_ptr->has_physical_ifc()) {
            physical = entity_ptr->physical_ifc();
            if (entity_ptr->get_ai_core() != nullptr) {
                physical->set_control_parent(nullptr);
            }
        }

        if (physical != nullptr && (physical->field_C & 0x80000) != 0) {
            const vector3d translation = parms->position - entity_ptr->get_rel_position();
            (void)translation;
            (void)physical->get_biped_system();
            // 0x0059F400 biped_system::translate_bones is a retail no-op.
        } else {
            entity_ptr->set_abs_position(parms->position);
        }

        if (entity_ptr->is_an_actor()) {
            auto *actor_ptr = static_cast<actor *>(entity_ptr);
            actor_ptr->set_allow_tunnelling_into_next_frame(true);
            actor_ptr->invalidate_frame_delta();
        }

        if (entity_ptr->is_an_entity() && !entity_ptr->is_flagged(0x8000)) {
            auto *entity = static_cast<::entity *>(entity_ptr);
            entity->compute_sector(g_world_ptr->the_terrain, false, nullptr);
            entity->update_proximity_maps();
        }
        return true;
    }
};

struct slf__entity__set_render_alpha__num__t : script_library_class::function {
    slf__entity__set_render_alpha__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089B244;
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
        float alpha;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *owner = parms->entity.get_volatile_ptr();
        if (owner != nullptr && owner->is_an_entity()) {
            if (parms->alpha < 0.0f)
                parms->alpha = 0.0f;
            if (parms->alpha > 1.0f)
                parms->alpha = 1.0f;
            auto *entity_ptr = static_cast<entity *>(owner);
            auto color = entity_ptr->get_render_color();
            color.field_0[3] = static_cast<uint8_t>(parms->alpha * 255.0);
            entity_ptr->set_render_color(color);
        }
        return true;
    }
};

struct slf__entity__set_render_color__vector3d__t : script_library_class::function {
    slf__entity__set_render_color__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089B23C;
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
        vector3d color;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;

        auto *owner = parms->entity.get_volatile_ptr();
        if (owner != nullptr && owner->is_an_entity()) {
            for (int i = 0; i < 3; ++i) {
                if (parms->color[i] < 0.f)
                    parms->color[i] = 0.f;
                if (parms->color[i] > 1.f)
                    parms->color[i] = 1.f;
            }
            auto *entity_ptr = static_cast<entity *>(owner);
            auto color = entity_ptr->get_render_color();
            color.field_0[2] = static_cast<uint8_t>(parms->color.x * 255.0);
            color.field_0[1] = static_cast<uint8_t>(parms->color.y * 255.0);
            color.field_0[0] = static_cast<uint8_t>(parms->color.z * 255.0);
            entity_ptr->set_render_color(color);
        }
        return true;
    }
};

struct slf__entity__set_render_scale__vector3d__t : script_library_class::function {
    slf__entity__set_render_scale__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = CAST(m_vtbl, 0x0089B22C);
    }

    struct parms_t {
        entity_base_vhandle entity;
        vector3d scale;
    };

    bool operator()(vm_stack &stack, entry_t) const
    {
        SLF_PARMS;
        auto *owner = parms->entity.get_volatile_ptr();
        if (owner != nullptr && owner->is_an_entity()) {
            using set_scale_t = void(__fastcall *)(entity_base *, void *, const vector3d &);
            set_scale_t set_scale = CAST(set_scale, get_vfunc(owner->m_vtbl, 0x1D0));
            set_scale(owner, nullptr, parms->scale);
        }
        return true;
    }
};

struct slf__entity__set_scale__num__t : script_library_class::function {
    slf__entity__set_scale__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089B274;
    }

    struct parms_t {
        entity_base_vhandle owner;
        vm_num_t scale;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *owner = parms->owner.get_volatile_ptr();
        if (owner != nullptr && parms->scale > 0.0f) {
            const float scale = parms->scale / owner->get_rel_po().get_x_facing().length();
            po scaling;
            scaling.set_scale(vector3d{scale, scale, scale});
            po transform = owner->get_rel_po();
            const vector3d position = transform.get_position();
            transform.set_position(ZEROVEC);
            const ptr_to_po source{&transform.m, &scaling.m};
            transform.set_from_ptr_to_po_world(source);
            transform.set_position(position);
            owner->set_abs_po(transform);
        }
        return true;
    }
};

struct slf__entity__set_scripted_target__entity__t : script_library_class::function {
    slf__entity__set_scripted_target__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089B18C;
    }

    struct parms_t {
        entity_base_vhandle owner;
        entity_base_vhandle target;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *owner = parms->owner.get_volatile_ptr();
        if (owner != nullptr && owner->is_an_actor()) {
            if (auto *core = owner->get_ai_core()) {
                auto *target = static_cast<ai::combat_target_inode *>(
                    core->get_info_node(ai::combat_target_inode::default_id, false));
                if (target != nullptr)
                    target->field_30 = parms->target.field_0;
            }
        }
        return true;
    }
};

struct slf__entity__set_see_thru__num__t : script_library_class::function {
    slf__entity__set_see_thru__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AD94;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_special_target__num__t : script_library_class::function {
    slf__entity__set_special_target__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B454;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_state__num__num__t : script_library_class::function {
    slf__entity__set_state__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089B574;
#endif
    }

    struct parms_t {
        entity_base_vhandle me;
        float state;
        float value;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *target = parms->me.get_volatile_ptr();
        if (target != nullptr && target->get_flavor() == 10) {
            auto *function = reinterpret_cast<void(__fastcall *)(entity_base *, void *, int, float)>(
                get_vfunc(target->m_vtbl, 0x21C));
            function(target, nullptr, static_cast<int>(parms->state), parms->value);
        }
        SLF_DONE;
    }
};

struct slf__entity__set_targetting__num__t : script_library_class::function {
    slf__entity__set_targetting__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AD54;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_throwable__num__t : script_library_class::function {
    slf__entity__set_throwable__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AD6C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_time_dilation__num__t : script_library_class::function {
    slf__entity__set_time_dilation__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B30C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_time_mode__num__t : script_library_class::function {
    slf__entity__set_time_mode__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B31C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_variant__string_hash__t : script_library_class::function {
    slf__entity__set_variant__string_hash__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B53C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_velocity__vector3d__t : script_library_class::function {
    slf__entity__set_velocity__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = CAST(m_vtbl, 0x0089AC74);
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
        vector3d velocity;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        if (auto *entity_ptr = parms->entity.get_volatile_ptr();
            entity_ptr != nullptr && entity_ptr->has_physical_ifc()) {
            const auto velocity = entity_ptr->get_abs_po().non_affine_slow_xform(parms->velocity);
            entity_ptr->physical_ifc()->set_velocity(velocity, false);
        }
        return true;
    }
};

struct slf__entity__set_visible__num__t : script_library_class::function {
    slf__entity__set_visible__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089AD7C;
#endif
    }

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        struct parms_t {
            entity_base_vhandle owner;
            float visible;
        };
        SLF_PARMS;
        if (auto *owner = parms->owner.get_volatile_ptr(); owner != nullptr)
            owner->set_visible(!equal(parms->visible, 0.0f), false);
        return true;
    }
};

struct slf__entity__set_visible_and_disable_fading__num__t : script_library_class::function {
    slf__entity__set_visible_and_disable_fading__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AD8C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__set_xz_facing__vector3d__t : script_library_class::function {
    slf__entity__set_xz_facing__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089AC84;
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
        vector3d facing;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *entity_ptr = parms->entity.get_volatile_ptr();
        if (entity_ptr == nullptr)
            return true;
        auto *marky_camera = g_world_ptr->field_28.field_44;
        if (entity_ptr == marky_camera &&
            std::not_equal_to<float>{}(stack.get_thread()->field_1E0, marky_camera->field_1D8))
            return true;
        vector3d facing = parms->facing;
        facing.normalize();
        if (facing.y < 0.95f) {
            facing.y = 0.0f;
            facing.normalize();
            vector3d right = vector3d::cross(YVEC, facing);
            right.normalize();
            po transform;
            transform.set_po(right, YVEC, facing, entity_ptr->get_rel_po().get_position());
            entity_ptr->set_abs_po(transform);
        }
        return true;
    }
};

struct slf__entity__setup_tether__vector3d__num__t : script_library_class::function {
    slf__entity__setup_tether__vector3d__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B1CC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__snap_to__entity__t : script_library_class::function {
    slf__entity__snap_to__entity__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE2C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__suspend__t : script_library_class::function {
    slf__entity__suspend__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089AF7C;
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *owner = parms->entity.get_volatile_ptr();
        if (owner != nullptr && owner->is_an_entity() && !owner->is_flagged(entity_flag_t::EFLAG_SUSPENDED)) {
            static_cast<entity *>(owner)->suspend(true);
        }
        return true;
    }
};

struct slf__entity__teleport_to_point__vector3d__t : script_library_class::function {
    slf__entity__teleport_to_point__vector3d__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089AC6C;
    }

    struct parms_t {
        entity_base_vhandle owner;
        vector3d position;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        if (auto *owner = parms->owner.get_volatile_ptr())
            entity_teleport_abs_position(owner, parms->position, false);
        return true;
    }
};

struct slf__entity__unforce_regions__t : script_library_class::function {
    slf__entity__unforce_regions__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AD2C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__unsuspend__t : script_library_class::function {
    slf__entity__unsuspend__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089AFAC;
#endif
    }

    struct parms_t {
        entity_base_vhandle entity;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        auto *owner = parms->entity.get_volatile_ptr();
        if (owner != nullptr && owner->is_an_entity()) {
            static_cast<entity *>(owner)->unsuspend(true);
        }
        return true;
    }
};

struct slf__entity__use_item__str__t : script_library_class::function {
    slf__entity__use_item__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B1FC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__use_item_by_id__str__t : script_library_class::function {
    slf__entity__use_item_by_id__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B204;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_camera_set_roll__num__num__t : script_library_class::function {
    slf__entity__wait_camera_set_roll__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE54;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_change_color__vector3d__vector3d__num__t : script_library_class::function {
    slf__entity__wait_change_color__vector3d__vector3d__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE7C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_change_range__num__num__num__t : script_library_class::function {
    slf__entity__wait_change_range__num__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AE84;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};


struct slf__entity__wait_change_render_color__vector3d__num__num__t : script_library_class::function {
    slf__entity__wait_change_render_color__vector3d__num__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
#if STANDALONE_SYSTEM
        bind_standalone_entity_slf(this);
#else
        m_vtbl = (decltype(m_vtbl))0x0089B25C;
#endif
    }

    struct parms_t {
        entity_base_vhandle owner;
        vector3d color;
        float alpha;
        float duration;
    };

    struct recall_t {
        float red;
        float green;
        float blue;
        float alpha;
        float elapsed;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t entry) const
    {
        SLF_PARMS;
        auto *owner = parms->owner.get_volatile_ptr();
        if (owner == nullptr)
            return true;
        auto *recall = reinterpret_cast<recall_t *>(parms + 1);
        if (entry == FIRST_ENTRY) {
            const auto color =
                owner->is_an_entity() ? static_cast<entity *>(owner)->get_render_color() : color32{0xFFFFFFFF};
            recall->red = static_cast<unsigned char>(color.field_0[2]) * 0.0039215689f;
            recall->green = static_cast<unsigned char>(color.field_0[1]) * 0.0039215689f;
            recall->blue = static_cast<unsigned char>(color.field_0[0]) * 0.0039215689f;
            recall->alpha = static_cast<unsigned char>(color.field_0[3]) * 0.0039215689f;
            recall->elapsed = 0.0f;
            return false;
        }
        const float time_scale =
            owner->has_time_ifc() ? owner->time_ifc()->sub_4ADE50() : g_world_ptr->time_manager.field_0;
        recall->elapsed += script_manager::get_time_inc() * time_scale;
        if (recall->elapsed > parms->duration)
            recall->elapsed = parms->duration;
        const bool immediate = equal(parms->duration, 0.0f);
        const double amount = immediate ? 1.0 : recall->elapsed / double(parms->duration);
        const auto interpolate = [immediate, amount](float initial, float target) {
            return static_cast<uint8_t>((immediate ? target : initial + (target - initial) * amount) * 255);
        };
        if (owner->is_an_entity()) {
            color32 color;
            color.field_0[2] = interpolate(recall->red, parms->color.x);
            color.field_0[1] = interpolate(recall->green, parms->color.y);
            color.field_0[0] = interpolate(recall->blue, parms->color.z);
            color.field_0[3] = interpolate(recall->alpha, parms->alpha);
            static_cast<entity *>(owner)->set_render_color(color);
        }
        return !(recall->elapsed < parms->duration);
    }
};

struct slf__entity__wait_change_render_scale__vector3d__num__t : script_library_class::function {
    slf__entity__wait_change_render_scale__vector3d__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B234;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_for_not_sector__str__t : script_library_class::function {
    slf__entity__wait_for_not_sector__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AFEC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_for_pickup__t : script_library_class::function {
    slf__entity__wait_for_pickup__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AFB4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_for_sector__str__t : script_library_class::function {
    slf__entity__wait_for_sector__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AFE4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_lookat__vector3d__num__t : script_library_class::function {
    slf__entity__wait_lookat__vector3d__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B26C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_lookat2__entity__entity__vector3d__num__t : script_library_class::function {
    slf__entity__wait_lookat2__entity__entity__vector3d__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B264;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_looping_anim__str__num__num__t : script_library_class::function {
    slf__entity__wait_looping_anim__str__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B1F4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_play_anim__str__num__num__num__t : script_library_class::function {
    slf__entity__wait_play_anim__str__num__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AEB4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_prox__entity__num__t : script_library_class::function {
    slf__entity__wait_prox__entity__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AFBC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_prox__vector3d__num__t : script_library_class::function {
    slf__entity__wait_prox__vector3d__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AFC4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_prox__vector3d__num__vector3d__num__t : script_library_class::function {
    slf__entity__wait_prox__vector3d__num__vector3d__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AFCC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_prox_maxY__vector3d__num__num__t : script_library_class::function {
    slf__entity__wait_prox_maxY__vector3d__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AFDC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_prox_minY__vector3d__num__num__t : script_library_class::function {
    slf__entity__wait_prox_minY__vector3d__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AFD4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_prox_sector__vector3d__num__str__t : script_library_class::function {
    slf__entity__wait_prox_sector__vector3d__num__str__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089AFF4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_rotate__vector3d__num__num__t : script_library_class::function {
    slf__entity__wait_rotate__vector3d__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089ACAC;
    }

    struct parms_t {
        entity_base_vhandle owner;
        vector3d axis;
        vm_num_t angle;
        vm_num_t duration;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t entry) const
    {
        SLF_PARMS;
        auto *owner = parms->owner.get_volatile_ptr();
        if (owner == nullptr)
            return true;
        float elapsed = 1.0f;
        float angle = parms->angle;
        float remaining = 0.0f;
        const bool immediate = std::equal_to<float>{}(parms->duration, 0.0f);
        if (!immediate) {
            if (entry == FIRST_ENTRY)
                return false;
            const float time_scale =
                owner->has_time_ifc() ? owner->time_ifc()->sub_4ADE50() : g_world_ptr->time_manager.field_0;
            elapsed = script_manager::get_time_inc() * time_scale;
            remaining = parms->duration - elapsed;
            if (remaining < 0.0f) {
                elapsed += remaining;
                remaining = 0.0f;
            }
            angle = 0.0f;
        }
        auto *camera = g_world_ptr->field_28.field_44;
        if (owner != camera || std::equal_to<float>{}(stack.get_thread()->field_1E0, camera->field_1D8)) {
            if (!immediate)
                angle = parms->angle / parms->duration * elapsed;
            po rotation;
            rotation.set_rot(parms->axis, angle);
            if (owner->is_an_actor()) {
                auto *actor = static_cast<::actor *>(owner);
                if (immediate)
                    actor->set_frame_delta(rotation, elapsed);
                else
                    actor->set_frame_delta_no_update(rotation, elapsed);
            }
            const ptr_to_po source{&rotation.m, &owner->get_rel_po().m};
            rotation.set_from_ptr_to_po_world(source);
            owner->set_abs_po(rotation);
        }
        if (immediate || remaining <= EPSILON)
            return true;
        parms->angle -= angle;
        parms->duration = remaining;
        return false;
    }
};

struct slf__entity__wait_rotate_WCS__vector3d__vector3d__num__num__t : script_library_class::function {
    slf__entity__wait_rotate_WCS__vector3d__vector3d__num__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ACBC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_rotate_WCS_cosmetic__vector3d__vector3d__num__num__t : script_library_class::function {
    slf__entity__wait_rotate_WCS_cosmetic__vector3d__vector3d__num__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ACCC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_rotate_WCS_with_compute_sector__vector3d__vector3d__num__num__t
    : script_library_class::function {
    slf__entity__wait_rotate_WCS_with_compute_sector__vector3d__vector3d__num__num__t(script_library_class *slc,
                                                                                      const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ACC4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_rotate_cosmetic__vector3d__num__num__t : script_library_class::function {
    slf__entity__wait_rotate_cosmetic__vector3d__num__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ACB4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_set_scale__num__num__t : script_library_class::function {
    slf__entity__wait_set_scale__num__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089B27C;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_translate__vector3d__num__t : script_library_class::function {
    slf__entity__wait_translate__vector3d__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ACD4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_translate_WCS__vector3d__num__t : script_library_class::function {
    slf__entity__wait_translate_WCS__vector3d__num__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ACEC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_translate_WCS_cosmetic__vector3d__num__t : script_library_class::function {
    slf__entity__wait_translate_WCS_cosmetic__vector3d__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ACFC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_translate_WCS_with_compute_sector__vector3d__num__t : script_library_class::function {
    slf__entity__wait_translate_WCS_with_compute_sector__vector3d__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ACF4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_translate_cosmetic__vector3d__num__t : script_library_class::function {
    slf__entity__wait_translate_cosmetic__vector3d__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ACE4;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__wait_translate_with_compute_sector__vector3d__num__t : script_library_class::function {
    slf__entity__wait_translate_with_compute_sector__vector3d__num__t(script_library_class *slc, const char *a3)
        : function(slc, a3)
    {
        m_vtbl = (decltype(m_vtbl))0x0089ACDC;
    }

    bool operator()(vm_stack &, script_library_class::function::entry_t) const
    {
        return true;
    }
};

struct slf__entity__was_occluded_last_frame__t : script_library_class::function {
    slf__entity__was_occluded_last_frame__t(script_library_class *slc, const char *a3) : function(slc, a3)
    {
        if constexpr (STANDALONE_SYSTEM)
            bind_standalone_entity_slf(this);
        else
            m_vtbl = (decltype(m_vtbl))0x0089AE24;
    }

    struct parms_t {
        entity_base_vhandle owner;
    };

    bool operator()(vm_stack &stack, script_library_class::function::entry_t) const
    {
        SLF_PARMS;
        float result = 0.0f;
        if (auto *owner = parms->owner.get_volatile_ptr()) {
            using predicate_t = bool(__fastcall *)(entity_base *, void *);
            auto is_signaller = reinterpret_cast<predicate_t>(get_vfunc(owner->m_vtbl, 0x5C));
            if (is_signaller(owner, nullptr))
                result = static_cast<signaller *>(owner)->get_occluded_last_frame() ? 1.0f : 0.0f;
        }
        SLF_RETURN;
        return true;
    }
};

slc_entity_t *slc_entity = nullptr;

void register_entity_lib()
{
#define BUILD_SLF_NAME(_KLASS, _TYPE) slf__##_KLASS##__##_TYPE##__t

#define CREATE_SLF(KLASS, TYPE, NAME)                                                                       \
    do {                                                                                                    \
        new (mem_alloc(sizeof(BUILD_SLF_NAME(KLASS, TYPE)))) BUILD_SLF_NAME(KLASS, TYPE){slc_entity, NAME}; \
    } while (false)

    CREATE_SLF(entity, abs_snap_to__entity, "abs_snap_to(entity)");
    CREATE_SLF(entity, add_collision_ignorance__entity, "add_collision_ignorance(entity)");
    CREATE_SLF(entity,
               add_exclusive_interactor__string_hash__interactable_interface,
               "add_exclusive_interactor(string_hash,interactable_interface)");
    CREATE_SLF(entity, add_item__entity, "add_item(entity)");
    CREATE_SLF(entity, add_selectable_target__entity, "add_selectable_target(entity)");
    CREATE_SLF(entity, add_vehicle_to_traffic_system__num, "add_vehicle_to_traffic_system(num)");
    CREATE_SLF(entity, ai_get_viseme_morph_set__str__str, "ai_get_viseme_morph_set(str,str)");
    CREATE_SLF(entity, ai_get_viseme_stream__str, "ai_get_viseme_stream(str)");
    CREATE_SLF(entity, ai_is_speaking, "ai_is_speaking()");
    CREATE_SLF(entity, ai_run_lip_sync__str, "ai_run_lip_sync(str)");
    CREATE_SLF(entity, ai_say_file__str__num__num, "ai_say_file(str,num,num)");
    CREATE_SLF(entity, ai_say_gab__str__num__num, "ai_say_gab(str,num,num)");
    CREATE_SLF(entity, ai_say_sound_group__str__num__num, "ai_say_sound_group(str,num,num)");
    CREATE_SLF(entity, ai_shut_up, "ai_shut_up()");
    CREATE_SLF(entity, ai_traffic_come_on_camera__vector3d__num, "ai_traffic_come_on_camera(vector3d,num)");
    CREATE_SLF(
        entity, ai_traffic_come_on_camera__vector3d__vector3d__num, "ai_traffic_come_on_camera(vector3d,vector3d,num)");
    CREATE_SLF(entity, ai_traffic_follow_entity__entity__num, "ai_traffic_follow_entity(entity,num)");
    CREATE_SLF(entity, ai_traffic_follow_vehicle__entity, "ai_traffic_follow_vehicle(entity)");
    CREATE_SLF(entity, ai_traffic_get_value__num, "ai_traffic_get_value(num)");
    CREATE_SLF(entity, ai_traffic_get_value__num__num, "ai_traffic_get_value(num,num)");
    CREATE_SLF(entity, ai_traffic_goto__vector3d__num__num__num, "ai_traffic_goto(vector3d,num,num,num)");
    CREATE_SLF(entity, ai_traffic_set_value__num__num, "ai_traffic_set_value(num,num)");
    CREATE_SLF(entity, ai_traffic_spawn_away_from__vector3d__num, "ai_traffic_spawn_away_from(vector3d,num)");
    CREATE_SLF(entity, ai_traffic_spawn_behind__entity, "ai_traffic_spawn_behind(entity)");
    CREATE_SLF(entity, ai_traffic_spawn_near__vector3d, "ai_traffic_spawn_near(vector3d)");
    CREATE_SLF(
        entity, ai_voice_box_set_team_respect__string_hash__num, "ai_voice_box_set_team_respect(string_hash,num)");
    CREATE_SLF(entity, ai_wait_say_file__str__num__num, "ai_wait_say_file(str,num,num)");
    CREATE_SLF(entity, ai_wait_say_gab__str__num__num, "ai_wait_say_gab(str,num,num)");
    CREATE_SLF(entity, ai_wait_say_preregistered_file__str__num__num, "ai_wait_say_preregistered_file(str,num,num)");
    CREATE_SLF(entity, ai_wait_say_sound_group__str__num__num, "ai_wait_say_sound_group(str,num,num)");
    CREATE_SLF(entity, anim_finished, "anim_finished()");
    CREATE_SLF(entity, anim_finished__num, "anim_finished(num)");
    CREATE_SLF(entity, apply_continuous_rotation__vector3d__num__num, "apply_continuous_rotation(vector3d,num,num)");
    CREATE_SLF(entity, apply_damage__num, "apply_damage(num)");
    CREATE_SLF(entity, apply_directed_damage__num__vector3d, "apply_directed_damage(num,vector3d)");
    CREATE_SLF(
        entity, apply_directed_damage_cat__num__vector3d__str__num, "apply_directed_damage_cat(num,vector3d,str,num)");
    CREATE_SLF(entity, apply_explosive_damage__num__vector3d, "apply_explosive_damage(num,vector3d)");
    if constexpr (!xbpack::v10) {
        CREATE_SLF(
            entity, apply_explosive_damage__num__vector3d__vector3d, "apply_explosive_damage(num,vector3d,vector3d)");
    }
    CREATE_SLF(entity, apply_subdue__num, "apply_subdue(num)");
    CREATE_SLF(entity, camera_get_target, "camera_get_target()");
    CREATE_SLF(entity, camera_orbit__vector3d__num__num__num, "camera_orbit(vector3d,num,num,num)");
    CREATE_SLF(entity, camera_set_collide_with_world__num, "camera_set_collide_with_world(num)");
    CREATE_SLF(entity, camera_set_roll__num, "camera_set_roll(num)");
    CREATE_SLF(entity, camera_set_target__vector3d, "camera_set_target(vector3d)");
    CREATE_SLF(entity, camera_slide_to__vector3d__vector3d__num__num, "camera_slide_to(vector3d,vector3d,num,num)");
    CREATE_SLF(
        entity, camera_slide_to_orbit__vector3d__num__num__num__num, "camera_slide_to_orbit(vector3d,num,num,num,num)");
    CREATE_SLF(entity, cancel_tether, "cancel_tether()");
    CREATE_SLF(entity, car_random_body_and_color, "car_random_body_and_color()");
    CREATE_SLF(entity, change_ai_base_machine__str, "change_ai_base_machine(str)");
    CREATE_SLF(entity, collisions_enabled, "collisions_enabled()");
    CREATE_SLF(entity, compute_sector, "compute_sector()");
    CREATE_SLF(entity, create_damage_interface, "create_damage_interface()");
    CREATE_SLF(entity, create_interactable_ifc, "create_interactable_ifc()");
    CREATE_SLF(entity, create_physical_interface, "create_physical_interface()");
    CREATE_SLF(entity, create_script_data_interface, "create_script_data_interface()");
    CREATE_SLF(entity, create_web_ifc__str__num__num, "create_web_ifc(str,num,num)");
    CREATE_SLF(entity, disable_as_target, "disable_as_target()");
    CREATE_SLF(entity, disable_collisions, "disable_collisions()");
    CREATE_SLF(entity, disable_fading, "disable_fading()");
    CREATE_SLF(entity, disgorge_items, "disgorge_items()");
    CREATE_SLF(entity, enable_as_target, "enable_as_target()");
    CREATE_SLF(entity, enable_collisions, "enable_collisions()");
    CREATE_SLF(entity, enable_collisions__num, "enable_collisions(num)");
    CREATE_SLF(entity, eye_check_ent__entity, "eye_check_ent(entity)");
    CREATE_SLF(entity, eye_check_pos__vector3d, "eye_check_pos(vector3d)");
    CREATE_SLF(entity,
               force_activate_interaction__string_hash__interactable_interface,
               "force_activate_interaction(string_hash,interactable_interface)");
    CREATE_SLF(entity, force_current_region, "force_current_region()");
    CREATE_SLF(entity, force_region__entity, "force_region(entity)");
    CREATE_SLF(entity, get_abs_position, "get_abs_position()");
    CREATE_SLF(entity, get_ai_base_machine_name, "get_ai_base_machine_name()");
    CREATE_SLF(entity, get_ai_param_float__str, "get_ai_param_float(str)");
    CREATE_SLF(entity, get_ai_param_hash__str, "get_ai_param_hash(str)");
    CREATE_SLF(entity, get_ai_param_int__str, "get_ai_param_int(str)");
    CREATE_SLF(entity, get_ai_param_str__str, "get_ai_param_str(str)");
    CREATE_SLF(entity, get_ai_param_vector3d__str, "get_ai_param_vector3d(str)");
    CREATE_SLF(entity, get_ai_signaller, "get_ai_signaller()");
    CREATE_SLF(entity, get_anchor_point, "get_anchor_point()");
    CREATE_SLF(entity, get_carry_slave, "get_carry_slave()");
    CREATE_SLF(entity, get_current_animation_name, "get_current_animation_name()");
    CREATE_SLF(entity, get_damage_force, "get_damage_force()");
    CREATE_SLF(entity, get_debug_name, "get_debug_name()");
    if constexpr (!xbpack::v10) {
        CREATE_SLF(entity, get_detonate_position, "get_detonate_position()");
    }
    CREATE_SLF(entity, get_district, "get_district()");
    CREATE_SLF(entity, get_facing, "get_facing()");
    CREATE_SLF(entity, get_fade_timer, "get_fade_timer()");
    CREATE_SLF(entity, get_first_child, "get_first_child()");
    if constexpr (!xbpack::v10) {
        CREATE_SLF(entity, get_hash_name, "get_hash_name()");
    }
    CREATE_SLF(entity, get_hidey_pos__vector3d__num, "get_hidey_pos(vector3d,num)");
    CREATE_SLF(entity, get_ifc_num__str, "get_ifc_num(str)");
    CREATE_SLF(entity, get_ifc_str__str, "get_ifc_str(str)");
    CREATE_SLF(entity, get_ifc_vec__str, "get_ifc_vec(str)");
    CREATE_SLF(entity, get_ifl_frame, "get_ifl_frame()");
    CREATE_SLF(entity, get_inode_param_float__str__str, "get_inode_param_float(str,str)");
    CREATE_SLF(entity, get_inode_param_hash__str__str, "get_inode_param_hash(str,str)");
    CREATE_SLF(entity, get_inode_param_int__str__str, "get_inode_param_int(str,str)");
    CREATE_SLF(entity, get_inode_param_str__str__str, "get_inode_param_str(str,str)");
    CREATE_SLF(entity, get_inode_param_vector3d__str__str, "get_inode_param_vector3d(str,str)");
    CREATE_SLF(entity, get_interactable_ifc, "get_interactable_ifc()");
    CREATE_SLF(entity, get_item__num, "get_item(num)");
    CREATE_SLF(entity, get_item_by_name__str, "get_item_by_name(str)");
    CREATE_SLF(entity, get_item_quantity__num, "get_item_quantity(num)");
    CREATE_SLF(entity, get_last_anchor, "get_last_anchor()");
    CREATE_SLF(entity, get_last_attacker, "get_last_attacker()");
    CREATE_SLF(entity, get_last_item_used, "get_last_item_used()");
    CREATE_SLF(entity, get_member__str, "get_member(str)");
    if constexpr (xbpack::v10) {
        CREATE_SLF(entity, get_debug_name, "get_name()");
    }
    CREATE_SLF(entity, get_next_sibling, "get_next_sibling()");
    CREATE_SLF(entity, get_num_items, "get_num_items()");
    CREATE_SLF(entity, get_parent, "get_parent()");
    CREATE_SLF(entity, get_pendulum_length, "get_pendulum_length()");
    CREATE_SLF(entity, get_rel_position, "get_rel_position()");
    CREATE_SLF(entity, get_rel_velocity__entity, "get_rel_velocity(entity)");
    CREATE_SLF(entity, get_render_alpha, "get_render_alpha()");
    CREATE_SLF(entity, get_render_color, "get_render_color()");
    CREATE_SLF(entity, get_scripted_target, "get_scripted_target()");
    CREATE_SLF(entity, get_sector_name, "get_sector_name()");
    CREATE_SLF(entity, get_state__num, "get_state(num)");
    CREATE_SLF(entity, get_time_dilation, "get_time_dilation()");
    CREATE_SLF(entity, get_time_mode, "get_time_mode()");
    CREATE_SLF(entity, get_x_facing, "get_x_facing()");
    CREATE_SLF(entity, get_y_facing, "get_y_facing()");
    CREATE_SLF(entity, get_z_facing, "get_z_facing()");
    CREATE_SLF(entity, has_carry_slave, "has_carry_slave()");
    CREATE_SLF(entity, has_member__str, "has_member(str)");
    CREATE_SLF(entity, has_script_data_interface, "has_script_data_interface()");
    CREATE_SLF(entity, hates__entity, "hates(entity)");
    CREATE_SLF(entity, ifl_damage_lock__num, "ifl_damage_lock(num)");
    CREATE_SLF(entity, ifl_lock__num, "ifl_lock(num)");
    CREATE_SLF(entity, ifl_pause, "ifl_pause()");
    CREATE_SLF(entity, ifl_play, "ifl_play()");
    CREATE_SLF(entity, in_sector__vector3d__vector3d__num, "in_sector(vector3d,vector3d,num)");
    CREATE_SLF(entity, inhibit_universal_soldier_ability__str__num, "inhibit_universal_soldier_ability(str,num)");
    CREATE_SLF(entity, invoke_facial_expression__num__num__num__num, "invoke_facial_expression(num,num,num,num)");
    CREATE_SLF(entity, is_a_car, "is_a_car()");
    CREATE_SLF(entity, is_picked_up, "is_picked_up()");
    CREATE_SLF(entity, is_suspended, "is_suspended()");
    CREATE_SLF(entity, is_throwable, "is_throwable()");
    CREATE_SLF(entity, is_valid, "is_valid()");
    CREATE_SLF(entity, is_visible, "is_visible()");
    CREATE_SLF(entity, kill_anim_in_slot__num, "kill_anim_in_slot(num)");
    CREATE_SLF(entity, likes__entity, "likes(entity)");
    CREATE_SLF(entity, look_at__vector3d, "look_at(vector3d)");
    CREATE_SLF(entity, motion_blur_off, "motion_blur_off()");
    CREATE_SLF(entity, motion_blur_on__num__num__num, "motion_blur_on(num,num,num)");
    CREATE_SLF(entity, motion_trail_off, "motion_trail_off()");
    CREATE_SLF(entity,
               motion_trail_on__entity__entity__vector3d__num__num__num__num,
               "motion_trail_on(entity,entity,vector3d,num,num,num,num)");
    CREATE_SLF(entity,
               motion_trail_on__entity__str__num__num__vector3d__num__num__num__num,
               "motion_trail_on(entity,str,num,num,vector3d,num,num,num,num)");
    CREATE_SLF(entity, neutral__entity, "neutral(entity)");
    CREATE_SLF(entity, operator_not_equals__entity, "operator!=(entity)");
    CREATE_SLF(entity, operator_equals_equals__entity, "operator==(entity)");
    CREATE_SLF(entity, physical_ifc_add_particle__str, "physical_ifc_add_particle(str)");
    CREATE_SLF(entity, physical_ifc_apply_force__vector3d__num, "physical_ifc_apply_force(vector3d,num)");
    CREATE_SLF(entity, physical_ifc_cancel_all_velocity, "physical_ifc_cancel_all_velocity()");
    CREATE_SLF(entity, physical_ifc_clear_pendulum, "physical_ifc_clear_pendulum()");
    CREATE_SLF(entity, physical_ifc_get_attached_particle_name, "physical_ifc_get_attached_particle_name()");
    CREATE_SLF(entity, physical_ifc_get_bounce_particle_name, "physical_ifc_get_bounce_particle_name()");
    CREATE_SLF(entity, physical_ifc_is_biped_physics_running, "physical_ifc_is_biped_physics_running()");
    CREATE_SLF(entity, physical_ifc_is_effectively_standing, "physical_ifc_is_effectively_standing()");
    CREATE_SLF(entity, physical_ifc_is_prop_physics_at_rest, "physical_ifc_is_prop_physics_at_rest()");
    CREATE_SLF(entity, physical_ifc_is_prop_physics_running, "physical_ifc_is_prop_physics_running()");
    CREATE_SLF(entity, physical_ifc_manage_standing__num, "physical_ifc_manage_standing(num)");
    CREATE_SLF(entity, physical_ifc_set_allow_biped_physics__num, "physical_ifc_set_allow_biped_physics(num)");
    CREATE_SLF(entity, physical_ifc_set_attached_particle_name__str, "physical_ifc_set_attached_particle_name(str)");
    CREATE_SLF(entity, physical_ifc_set_bounce_particle_name__str, "physical_ifc_set_bounce_particle_name(str)");
    CREATE_SLF(entity, physical_ifc_set_pendulum__entity__num, "physical_ifc_set_pendulum(entity,num)");
    CREATE_SLF(entity, physical_ifc_set_pendulum__vector3d__num, "physical_ifc_set_pendulum(vector3d,num)");
    CREATE_SLF(entity, physical_ifc_start_biped_physics, "physical_ifc_start_biped_physics()");
    CREATE_SLF(entity, physical_ifc_start_prop_physics__vector3d__num, "physical_ifc_start_prop_physics(vector3d,num)");
    CREATE_SLF(entity, physical_ifc_stop_biped_physics, "physical_ifc_stop_biped_physics()");
    CREATE_SLF(entity, physical_ifc_stop_prop_physics, "physical_ifc_stop_prop_physics()");
    CREATE_SLF(entity, play_anim__str, "play_anim(str)");
    CREATE_SLF(entity, play_anim__str__num__num, "play_anim(str,num,num)");
    CREATE_SLF(entity, poison__num__num, "poison(num,num)");
    CREATE_SLF(entity, pop_ai_base_machine, "pop_ai_base_machine()");
    CREATE_SLF(entity, pre_roll__num, "pre_roll(num)");
    CREATE_SLF(entity, push_ai_base_machine__str, "push_ai_base_machine(str)");
    CREATE_SLF(entity, randomize_position__vector3d__num__num__num, "randomize_position(vector3d,num,num,num)");
    CREATE_SLF(entity, regenerate__num__num, "regenerate(num,num)");
    if constexpr (!xbpack::v10) {
        CREATE_SLF(entity, rel_angle__vector3d, "rel_angle(vector3d)");
    }
    CREATE_SLF(entity, remove_collision_ignorance__entity, "remove_collision_ignorance(entity)");
    CREATE_SLF(entity,
               remove_exclusive_interactor__string_hash__interactable_interface,
               "remove_exclusive_interactor(string_hash,interactable_interface)");
    CREATE_SLF(entity, remove_selectable_target__entity, "remove_selectable_target(entity)");
    CREATE_SLF(entity, remove_vehicle_from_traffic_system, "remove_vehicle_from_traffic_system()");
    if constexpr (xbpack::v10) {
        CREATE_SLF(entity, render_name__num, "render_name(num)");
    }
    if constexpr (!xbpack::v10) {
        CREATE_SLF(entity, reset_ai, "reset_ai()");
    }
    CREATE_SLF(entity, restart, "restart()");
    CREATE_SLF(entity, seriously_kill, "seriously_kill()");
    CREATE_SLF(entity, set_abs_xz_facing__vector3d, "set_abs_xz_facing(vector3d)");
    CREATE_SLF(entity, set_active__num, "set_active(num)");
    CREATE_SLF(entity, set_ai_param_float__str__num, "set_ai_param_float(str,num)");
    CREATE_SLF(entity, set_ai_param_float_variance__str__num__num, "set_ai_param_float_variance(str,num,num)");
    CREATE_SLF(entity, set_ai_param_hash__str__num, "set_ai_param_hash(str,num)");
    CREATE_SLF(entity, set_ai_param_hash__str__str, "set_ai_param_hash(str,str)");
    if constexpr (!xbpack::v10) {
        CREATE_SLF(entity, set_ai_param_hash__str__string_hash, "set_ai_param_hash(str,string_hash)");
    }
    CREATE_SLF(entity, set_ai_param_int__str__num, "set_ai_param_int(str,num)");
    CREATE_SLF(entity, set_ai_param_str__str__str, "set_ai_param_str(str,str)");
    CREATE_SLF(entity, set_ai_param_vector3d__str__vector3d, "set_ai_param_vector3d(str,vector3d)");
    CREATE_SLF(entity, set_ambient_factor__vector3d, "set_ambient_factor(vector3d)");
    CREATE_SLF(entity, set_anchor_activated__num, "set_anchor_activated(num)");
    CREATE_SLF(entity,
               set_car_combat_info__num__num__num__num__num__num__num__num,
               "set_car_combat_info(num,num,num,num,num,num,num,num)");
    CREATE_SLF(entity, set_crawlable__num, "set_crawlable(num)");
    if constexpr (!xbpack::v10) {
        CREATE_SLF(entity, set_default_variant, "set_default_variant()");
    }
    CREATE_SLF(entity, set_distance_clip__num, "set_distance_clip(num)");
    CREATE_SLF(entity, set_entity_blur__num, "set_entity_blur(num)");
    CREATE_SLF(entity, set_facing__vector3d__vector3d, "set_facing(vector3d,vector3d)");
    CREATE_SLF(entity, set_fade_timer__num, "set_fade_timer(num)");
    CREATE_SLF(entity, set_hires_shadow__num, "set_hires_shadow(num)");
    CREATE_SLF(entity, set_ifc_num__str__num, "set_ifc_num(str,num)");
    CREATE_SLF(entity, set_ifc_str__str__str, "set_ifc_str(str,str)");
    CREATE_SLF(entity, set_ifc_vec__str__vector3d, "set_ifc_vec(str,vector3d)");
    CREATE_SLF(entity, set_ignore_limbo__num, "set_ignore_limbo(num)");
    CREATE_SLF(entity, set_immobile__num, "set_immobile(num)");
    CREATE_SLF(entity, set_inode_param_entity__str__str__entity, "set_inode_param_entity(str,str,entity)");
    CREATE_SLF(entity, set_inode_param_float__str__str__num, "set_inode_param_float(str,str,num)");
    CREATE_SLF(
        entity, set_inode_param_float_variance__str__str__num__num, "set_inode_param_float_variance(str,str,num,num)");
    CREATE_SLF(entity, set_inode_param_hash__str__str__num, "set_inode_param_hash(str,str,num)");
    CREATE_SLF(entity, set_inode_param_hash__str__str__str, "set_inode_param_hash(str,str,str)");
    CREATE_SLF(entity, set_inode_param_int__str__str__num, "set_inode_param_int(str,str,num)");
    CREATE_SLF(entity, set_inode_param_str__str__str__str, "set_inode_param_str(str,str,str)");
    CREATE_SLF(entity, set_inode_param_vector3d__str__str__vector3d, "set_inode_param_vector3d(str,str,vector3d)");
    CREATE_SLF(entity, set_invulnerable__num, "set_invulnerable(num)");
    CREATE_SLF(entity, set_kill_ent_on_destroy__num, "set_kill_ent_on_destroy(num)");
    CREATE_SLF(entity, set_member_hidden__num, "set_member_hidden(num)");
    CREATE_SLF(entity, set_parent__entity, "set_parent(entity)");
    CREATE_SLF(entity, set_parent_rel__entity, "set_parent_rel(entity)");
    CREATE_SLF(entity, set_path_graph__str, "set_path_graph(str)");
    CREATE_SLF(entity, set_path_graph_start_node__num, "set_path_graph_start_node(num)");
    CREATE_SLF(entity, set_pendulum_attach_limb__num, "set_pendulum_attach_limb(num)");
    CREATE_SLF(entity, set_pendulum_length__num, "set_pendulum_length(num)");
    CREATE_SLF(entity, set_physical__num, "set_physical(num)");
    CREATE_SLF(entity, set_po_facing__vector3d, "set_po_facing(vector3d)");
    CREATE_SLF(entity, set_rel_position__vector3d, "set_rel_position(vector3d)");
    CREATE_SLF(entity, set_render_alpha__num, "set_render_alpha(num)");
    CREATE_SLF(entity, set_render_color__vector3d, "set_render_color(vector3d)");
    CREATE_SLF(entity, set_render_scale__vector3d, "set_render_scale(vector3d)");
    CREATE_SLF(entity, set_scale__num, "set_scale(num)");
    CREATE_SLF(entity, set_scripted_target__entity, "set_scripted_target(entity)");
    CREATE_SLF(entity, set_see_thru__num, "set_see_thru(num)");
    CREATE_SLF(entity, set_special_target__num, "set_special_target(num)");
    CREATE_SLF(entity, set_state__num__num, "set_state(num,num)");
    CREATE_SLF(entity, set_targetting__num, "set_targetting(num)");
    if constexpr (!xbpack::v10) {
        CREATE_SLF(entity, set_throwable__num, "set_throwable(num)");
    }
    CREATE_SLF(entity, set_time_dilation__num, "set_time_dilation(num)");
    CREATE_SLF(entity, set_time_mode__num, "set_time_mode(num)");
    CREATE_SLF(entity, set_variant__string_hash, "set_variant(string_hash)");
    CREATE_SLF(entity, set_velocity__vector3d, "set_velocity(vector3d)");
    CREATE_SLF(entity, set_visible__num, "set_visible(num)");
    CREATE_SLF(entity, set_visible_and_disable_fading__num, "set_visible_and_disable_fading(num)");
    CREATE_SLF(entity, set_xz_facing__vector3d, "set_xz_facing(vector3d)");
    CREATE_SLF(entity, setup_tether__vector3d__num, "setup_tether(vector3d,num)");
    CREATE_SLF(entity, snap_to__entity, "snap_to(entity)");
    CREATE_SLF(entity, suspend, "suspend()");
    CREATE_SLF(entity, teleport_to_point__vector3d, "teleport_to_point(vector3d)");
    CREATE_SLF(entity, unforce_regions, "unforce_regions()");
    CREATE_SLF(entity, unsuspend, "unsuspend()");
    CREATE_SLF(entity, use_item__str, "use_item(str)");
    CREATE_SLF(entity, use_item_by_id__str, "use_item_by_id(str)");
    CREATE_SLF(entity, wait_camera_set_roll__num__num, "wait_camera_set_roll(num,num)");
    CREATE_SLF(entity, wait_change_color__vector3d__vector3d__num, "wait_change_color(vector3d,vector3d,num)");
    CREATE_SLF(entity, wait_change_range__num__num__num, "wait_change_range(num,num,num)");
    CREATE_SLF(entity, wait_change_render_color__vector3d__num__num, "wait_change_render_color(vector3d,num,num)");
    CREATE_SLF(entity, wait_change_render_scale__vector3d__num, "wait_change_render_scale(vector3d,num)");
    CREATE_SLF(entity, wait_for_not_sector__str, "wait_for_not_sector(str)");
    CREATE_SLF(entity, wait_for_pickup, "wait_for_pickup()");
    CREATE_SLF(entity, wait_for_sector__str, "wait_for_sector(str)");
    CREATE_SLF(entity, wait_lookat__vector3d__num, "wait_lookat(vector3d,num)");
    CREATE_SLF(entity, wait_lookat2__entity__entity__vector3d__num, "wait_lookat2(entity,entity,vector3d,num)");
    CREATE_SLF(entity, wait_looping_anim__str__num__num, "wait_looping_anim(str,num,num)");
    CREATE_SLF(entity, wait_play_anim__str__num__num__num, "wait_play_anim(str,num,num,num)");
    CREATE_SLF(entity, wait_prox__entity__num, "wait_prox(entity,num)");
    CREATE_SLF(entity, wait_prox__vector3d__num, "wait_prox(vector3d,num)");
    CREATE_SLF(entity, wait_prox__vector3d__num__vector3d__num, "wait_prox(vector3d,num,vector3d,num)");
    CREATE_SLF(entity, wait_prox_maxY__vector3d__num__num, "wait_prox_maxY(vector3d,num,num)");
    CREATE_SLF(entity, wait_prox_minY__vector3d__num__num, "wait_prox_minY(vector3d,num,num)");
    CREATE_SLF(entity, wait_prox_sector__vector3d__num__str, "wait_prox_sector(vector3d,num,str)");
    CREATE_SLF(entity, wait_rotate__vector3d__num__num, "wait_rotate(vector3d,num,num)");
    CREATE_SLF(entity, wait_rotate_WCS__vector3d__vector3d__num__num, "wait_rotate_WCS(vector3d,vector3d,num,num)");
    CREATE_SLF(entity,
               wait_rotate_WCS_cosmetic__vector3d__vector3d__num__num,
               "wait_rotate_WCS_cosmetic(vector3d,vector3d,num,num)");
    CREATE_SLF(entity,
               wait_rotate_WCS_with_compute_sector__vector3d__vector3d__num__num,
               "wait_rotate_WCS_with_compute_sector(vector3d,vector3d,num,num)");
    CREATE_SLF(entity, wait_rotate_cosmetic__vector3d__num__num, "wait_rotate_cosmetic(vector3d,num,num)");
    CREATE_SLF(entity, wait_set_scale__num__num, "wait_set_scale(num,num)");
    CREATE_SLF(entity, wait_translate__vector3d__num, "wait_translate(vector3d,num)");
    CREATE_SLF(entity, wait_translate_WCS__vector3d__num, "wait_translate_WCS(vector3d,num)");
    CREATE_SLF(entity, wait_translate_WCS_cosmetic__vector3d__num, "wait_translate_WCS_cosmetic(vector3d,num)");
    CREATE_SLF(entity,
               wait_translate_WCS_with_compute_sector__vector3d__num,
               "wait_translate_WCS_with_compute_sector(vector3d,num)");
    CREATE_SLF(entity, wait_translate_cosmetic__vector3d__num, "wait_translate_cosmetic(vector3d,num)");
    CREATE_SLF(
        entity, wait_translate_with_compute_sector__vector3d__num, "wait_translate_with_compute_sector(vector3d,num)");
    if (!OPENUSM_XBOX_MASH_FORMAT || slc_manager::using_xbox_v14()) {
        CREATE_SLF(entity, was_occluded_last_frame, "was_occluded_last_frame()");
    }

#undef CREATE_SLF
#undef BUILD_SLF_NAME
}

int slc_entity_t::_find_instance(const mString &a1) const
{
    TRACE("slc_entity_t::find_instance");

    if (a1 == "NULL") {
        return 0;
    }

    auto *ent = (entity *)entity_handle_manager::find_entity(string_hash{a1.c_str()}, IGNORE_FLAVOR, true);
    if (ent == nullptr) {
        auto v5 = "entity " + a1;
        mString v6 = v5 + " not found";
        error(v6.c_str());
    }

    return ent->get_my_vhandle().get_goodies();
}

void script_lib_entity_patch()
{
    {
        FUNC_ADDRESS(address, &slc_entity_t::_find_instance);
        set_vfunc(0x0089A4C4, address);
    }

    {
        FUNC_ADDRESS(address, &slf__entity__add_item__entity__t::operator());
        set_vfunc(0x0089AF48, address);
    }
}
