#include "mash_info_struct.h"

#include "ai_interaction_data.h"
#include "als_animation_logic_system_shared.h"
#include "als_basic_rule_data.h"
#include "als_dest_weight_data.h"
#include "als_res_data.h"
#include "als_transition_rule.h"
#include "base_ai_res_state_graph.h"
#include "base_ai_data.h"
#include "combo_system.h"
#include "core_ai_resource.h"
#include "cut_scene.h"
#include "func_wrapper.h"
#include "gab_manager.h"
#include "glass_house.h"
#include "interactable_interface.h"
#include "mashed_state.h"
#include "nugget.h"
#include "panelfile.h"
#include "path_graph.h"
#include "patrol_def_set.h"
#include "skeleton_interface.h"
#include "sound_alias_database.h"
#include "string_hash_entry.h"
#include "token_def_list.h"
#include "trace.h"

template <>
void mash_info_struct::construct_class<mAvlTree<string_hash_entry>>(mAvlTree<string_hash_entry> *&a1)
{
    TRACE("mash_info_struct::construct_class<mAvlTree<string_hash_entry>>");

    if constexpr (1) {
        if (a1 != nullptr) {
            from_mash_in_place_constructor *v1 = nullptr;
            a1 = new (a1) mAvlTree<string_hash_entry>{v1};
        }
    } else {
        auto *v1 = a1;
        if (a1 != nullptr) {
            THISCALL(0x00420EF0, v1, nullptr);
            a1 = v1;
        } else {
            a1 = nullptr;
        }
    }
}

template <>
void mash_info_struct::construct_class(PanelFile *&a1)
{
    TRACE("mash_info_struct::construct_class<PanelFile>");
    if (a1 != nullptr) {
        void(__fastcall * func)(void *, int edx, void *) = CAST(func, 0x00642FA0);
        func(a1, 0, nullptr);
    }
}

template <>
void mash_info_struct::construct_class(sound_alias_database *&a1)
{
    if (a1 != nullptr) {
        if constexpr (1) {
            from_mash_in_place_constructor *v1 = nullptr;
            a1 = new (a1) sound_alias_database{v1};
        } else {
            void(__fastcall * func)(void *, int edx, void *) = CAST(func, 0x005D9040);
            func(a1, 0, nullptr);
        }
    }
}

template <>
void mash_info_struct::construct_class(token_def_list *&a1)
{
    TRACE("mash_info_struct::construct_class<token_def_list>");
    if (a1 != nullptr) {
        if constexpr (0) {
            void(__fastcall * func)(void *, int edx, void *) = CAST(func, 0x005DEDA0);
            func(a1, 0, nullptr);
        } else {
            new (a1) token_def_list{nullptr};
        }
    }
}

template <>
void mash_info_struct::construct_class(path_graph *&a1)
{
    if (a1 != nullptr) {
        void(__fastcall * func)(void *, int edx, void *) = CAST(func, 0x005DE080);
        func(a1, 0, nullptr);
    }
}

template <>
void mash_info_struct::construct_class(mVector<als::dest_weight_data> *&a1)
{
    if (a1 != nullptr) {
        if constexpr (1) {
            from_mash_in_place_constructor *v1 = nullptr;
            a1 = new (a1) mVector<als::dest_weight_data>{v1};
        } else {
            void(__fastcall * func)(void *, int edx, void *) = CAST(func, 0x004B1770);
            func(a1, 0, nullptr);
        }
    }
}

template <>
void mash_info_struct::construct_class(mVector<als::layer_transition_rule> *&a1)
{
    if (a1 != nullptr) {
        a1 = new (a1) mVector<als::layer_transition_rule>{};
    }
}

template <>
void mash_info_struct::construct_class(als::basic_rule_data::post_action_rule_set *&a1)
{
    if (a1 != nullptr) {
        void(__fastcall * func)(void *, int edx, void *) = CAST(func, 0x004AC210);
        func(a1, 0, nullptr);
    }
}

template <>
void mash_info_struct::construct_class(attach_interact_data *&a1)
{
    if (a1 != nullptr) {
        void(__fastcall * func)(void *, int edx, void *) = CAST(func, 0x006B8F80);
        func(a1, 0, nullptr);
    }
}

template <>
void mash_info_struct::construct_class(ai::param_block *&a1)
{
    if (a1 != nullptr) {
        if constexpr (1) {
            from_mash_in_place_constructor *v1 = nullptr;
            a1 = new (a1) ai::param_block{v1};
        } else {
            void(__fastcall * func)(void *, int edx, void *) = CAST(func, 0x006D9900);
            func(a1, 0, nullptr);
        }
    }
}

template <>
void mash_info_struct::construct_class(nugget *&a1)
{
    static_assert(std::is_base_of_v<mash_virtual_base, nugget>, "");
    if (a1 != nullptr) {
        a1 = static_cast<nugget *>(mash_virtual_base::construct_class_helper(a1));
    }
}

template <>
void mash_info_struct::construct_class(ai::state_graph *&a1)
{
    if (a1 != nullptr) {
        if constexpr (1) {
            a1 = new (a1) ai::state_graph{nullptr};
        } else {
            void(__fastcall * func)(void *, int edx, void *) = CAST(func, 0x006DA190);
            func(a1, 0, nullptr);
        }
    }
}

template <>
void mash_info_struct::construct_class(ai::mashed_state *&a1)
{
    if (a1 != nullptr) {
        a1 = new (a1) ai::mashed_state{nullptr};
    }
}

template <>
void mash_info_struct::construct_class(gab_database *&a1)
{
    if (a1 != nullptr) {
        if constexpr (1) {
            from_mash_in_place_constructor *v1 = nullptr;
            a1 = new (a1) gab_database{v1};
        } else {
            void(__fastcall * func)(void *, int edx, void *) = CAST(func, 0x005E0E80);
            func(a1, 0, nullptr);
        }
    }
}

template <>
void mash_info_struct::construct_class(als::animation_logic_system_shared *&a1)
{
    TRACE("mash_info_struct::construct_class<als::animation_logic_system_shared>");

    if (a1 != nullptr) {
        if constexpr (0) {
            a1 = new (a1) als::animation_logic_system_shared{nullptr};
        } else {
            void(__fastcall * func)(void *, int edx, void *) = CAST(func, 0x004AC000);
            func(a1, 0, nullptr);
        }
    }
}

template <>
void mash_info_struct::construct_class(ai::core_ai_resource *&a1)
{
    if (a1 != nullptr) {
        void(__fastcall * func)(void *, int edx, void *) = CAST(func, 0x006D9A10);
        func(a1, 0, nullptr);
    }
}

template <>
void mash_info_struct::construct_class(cut_scene *&a1)
{
    if (a1 != nullptr) {
        void(__fastcall * func)(void *, int edx, void *) = CAST(func, 0x00742890);
        func(a1, 0, nullptr);
    }
}

template <>
void mash_info_struct::construct_class(ai_interaction_data *&a1)
{
    if (a1 != nullptr) {
        void(__fastcall * func)(void *, int edx, void *) = CAST(func, 0x006B65B0);
        func(a1, 0, nullptr);
    }
}

template <>
void mash_info_struct::construct_class(als_res_data *&a1)
{
    TRACE("mash_info_struct::construct_class<als_res_data>");

    if (a1 != nullptr) {
        void(__fastcall * func)(void *, int edx, void *) = CAST(func, 0x004ABF80);
        func(a1, 0, nullptr);
    }
}

template <>
void mash_info_struct::construct_class(skeleton_interface *&a1)
{
    if (a1 != nullptr) {
        auto func = [](skeleton_interface *self, int a2, int a3) {
            self->my_conglomerate = CAST(self->my_conglomerate, a3);
            self->dynamic = (a2 == 1);
        };

        func(a1, 1, 0);
    }
}

template <>
void mash_info_struct::construct_class(interactable_interface *&a1)
{
    if (a1 != nullptr) {
        from_mash_in_place_constructor *v1 = nullptr;
        *a1 = interactable_interface{v1};
    }
}

template <>
void mash_info_struct::construct_class(base_ai_data *&a1)
{
    if (a1 != nullptr) {
        from_mash_in_place_constructor *v1 = nullptr;
        *a1 = base_ai_data{v1};
    }
}

template <>
void mash_info_struct::construct_class(patrol_def_set *&a1)
{
    if (a1 != nullptr) {
        from_mash_in_place_constructor *a2 = nullptr;
        a1 = new (a1) patrol_def_set{a2};
    }
}

template <>
void mash_info_struct::construct_class(glass_house *&a1)
{
    if (a1 != nullptr) {
        from_mash_in_place_constructor *v1 = nullptr;
        a1 = new (a1) glass_house{v1};
    }
}

template <>
void mash_info_struct::construct_class(ai::param_block::param_data_array *&a1)
{
    if (a1 != nullptr) {
        a1 = new (a1) ai::param_block::param_data_array{nullptr};
    }
}

template <>
void mash_info_struct::construct_class(combo_system *&a1)
{
    if (a1 != nullptr) {
        a1 = new (a1) combo_system{nullptr};
    }
}
