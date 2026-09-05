#include "mash_info_struct.h"

#include "ai_interaction_data.h"
#include "base_ai_res_state_graph.h"
#include "als_animation_logic_system_shared.h"
#include "als_basic_rule_data.h"
#include "als_dest_weight_data.h"
#include "als_transition_rule.h"
#include "base_ai_data.h"
#include "als_res_data.h"
#include "core_ai_resource.h"
#include "cut_scene.h"
#include "combo_system.h"
#include "gab_manager.h"
#include "glass_house.h"
#include "interactable_interface.h"
#include "mashed_state.h"
#include "nugget.h"
#include "path_graph.h"
#include "token_def_list.h"
#include "patrol_def_set.h"
#include "skeleton_interface.h"
#include "sound_alias_database.h"
#include "string_hash_entry.h"
#include "panelfile.h"
#include "trace.h"

#include "func_wrapper.h"

template<>
void mash_info_struct::construct_class<mAvlTree<string_hash_entry>>(mAvlTree<string_hash_entry> *&a1)
{
    if constexpr (!STANDALONE_SYSTEM) {
        auto *v1 = a1;
        if (v1 != nullptr) {
            THISCALL(0x00420EF0, v1, nullptr);
            a1 = v1;
        }
    }
}

template<>
void mash_info_struct::construct_class(PanelFile *&a1)
{
    TRACE("mash_info_struct::construct_class<PanelFile>");
    if (a1 == nullptr)
        return;

    if constexpr (STANDALONE_SYSTEM)
    {
        for (auto i = 0; i < a1->pquads.m_size; ++i)
        {
            auto *quad = a1->pquads.m_data[i];
            if (quad == nullptr)
                continue;

            quad->field_4 = 1.0f;
            quad->field_10 = 4;
            quad->field_34 = 0.0f;
            quad->field_38 = 1.0f;
        }

        for (auto i = 0; i < a1->ptext.m_size; ++i)
        {
            auto *text = a1->ptext.m_data[i];
            if (text == nullptr)
                continue;

            text->field_4 = 1.0f;
            text->field_10 = 4;
        }
    }
    else
    {
        auto func = bit_cast<void (__fastcall *)(void *, int, void *)>(
            0x00642FA0);
        func(a1, 0, nullptr);
    }
}

template<>
void mash_info_struct::construct_class(sound_alias_database *&a1)
{
    if (a1 != nullptr) {
        if constexpr (!STANDALONE_SYSTEM) {
            void (__fastcall *func)(void *, int edx, void *) =
                CAST(func, 0x005D9040);
            func(a1, 0, nullptr);
        }
    }
}

template<>
void mash_info_struct::construct_class(token_def_list *&a1)
{
    TRACE("mash_info_struct::construct_class<token_def_list>");
    if ( a1 != nullptr )
    {
        if constexpr (0) {
            void (__fastcall *func)(void *, int edx, void *) = CAST(func, 0x005DEDA0);
            func(a1, 0, nullptr);
        } else {
            new (a1) token_def_list {nullptr};
        }
    }
}

template<>
void mash_info_struct::construct_class(path_graph *&a1)
{
    if ( a1 != nullptr )
    {
        void (__fastcall *func)(void *, int edx, void *) = CAST(func, 0x005DE080);
        func(a1, 0, nullptr);
    }
}

template<>
void mash_info_struct::construct_class(ai::state_graph *&a1)
{
    if ( a1 != nullptr )
    {
        if constexpr (STANDALONE_SYSTEM) {
            a1 = new (a1) ai::state_graph {nullptr};
        } else {
            void (__fastcall *func)(void *, int edx, void *) = CAST(func, 0x006DA190);
            func(a1, 0, nullptr);
        }
    }
}

template<>
void mash_info_struct::construct_class(gab_database *&a1)
{
    if ( a1 != nullptr )
    {
        if constexpr (STANDALONE_SYSTEM) {
            new (a1) gab_database {nullptr};
        } else {
            void (__fastcall *func)(void *, int edx, void *) =
                CAST(func, 0x005E0E80);
            func(a1, 0, nullptr);
        }
    }
}

template<>
void mash_info_struct::construct_class(als::animation_logic_system_shared *&a1)
{
    TRACE("mash_info_struct::construct_class<als::animation_logic_system_shared>");

    if ( a1 != nullptr )
    {
        if constexpr (STANDALONE_SYSTEM) {
            a1 = new (a1) als::animation_logic_system_shared {nullptr};
        } else {
            void (__fastcall *func)(void *, int edx, void *) = CAST(func, 0x004AC000);
            func(a1, 0, nullptr);
        }
    }
}

template<>
void mash_info_struct::construct_class(ai::core_ai_resource *&a1)
{
    if ( a1 != nullptr )
    {
        if constexpr (STANDALONE_SYSTEM) {
            a1 = new (a1) ai::core_ai_resource {nullptr};
        } else {
            void (__fastcall *func)(void *, int edx, void *) = CAST(func, 0x006D9A10);
            func(a1, 0, nullptr);
        }
    }
}

template<>
void mash_info_struct::construct_class(cut_scene *&a1)
{
    if (a1 != nullptr)
    {
        if constexpr (STANDALONE_SYSTEM) {
            a1 = new (a1) cut_scene {nullptr};
        } else {
            void (__fastcall *func)(void *, int edx, void *) =
                CAST(func, 0x00742890);
            func(a1, 0, nullptr);
        }
    }
}

template<>
void mash_info_struct::construct_class(ai_interaction_data *&a1)
{
    if ( a1 != nullptr )
    {
        void (__fastcall *func)(void *, int edx, void *) = CAST(func, 0x006B65B0);
        func(a1, 0, nullptr);
    }
}

template<>
void mash_info_struct::construct_class(als_res_data *&a1)
{
    TRACE("mash_info_struct::construct_class<als_res_data>");

    if ( a1 != nullptr )
    {
        void (__fastcall *func)(void *, int edx, void *) = CAST(func, 0x004ABF80);
        func(a1, 0, nullptr);
    }
}

template<>
void mash_info_struct::construct_class(skeleton_interface *&a1)
{
    if ( a1 != nullptr )
    {
        auto func = [](skeleton_interface *self, int a2, int a3) {
            self->m_vtbl = 0x0088344C;
            self->my_conglomerate = CAST(self->my_conglomerate, a3);
            self->dynamic = (a2 == 1);
        };

        func(a1, 1, 0);
    }
}

template<>
void mash_info_struct::construct_class(mVector<als::dest_weight_data> *&a1)
{
    if (a1 != nullptr) {
        a1 = new (a1) mVector<als::dest_weight_data>{nullptr};
    }
}

template<>
void mash_info_struct::construct_class(mVector<als::layer_transition_rule> *&a1)
{
    if (a1 != nullptr) {
        a1 = new (a1) mVector<als::layer_transition_rule>{};
    }
}

template<>
void mash_info_struct::construct_class(als::basic_rule_data::post_action_rule_set *&a1)
{
    if (a1 != nullptr) {
        if constexpr (STANDALONE_SYSTEM) {
            from_mash_in_place_constructor *mash = nullptr;
            a1 = new (a1) als::basic_rule_data::post_action_rule_set{mash};
        } else {
            void(__fastcall *func)(void *, int, void *) = CAST(func, 0x004AC210);
            func(a1, 0, nullptr);
        }
    }
}

template<>
void mash_info_struct::construct_class(attach_interact_data *&a1)
{
    if (a1 != nullptr) {
        if constexpr (!STANDALONE_SYSTEM) {
            void(__fastcall *func)(void *, int, void *) = CAST(func, 0x006B8F80);
            func(a1, 0, nullptr);
        }
    }
}

template<>
void mash_info_struct::construct_class(ai::param_block *&a1)
{
    if (a1 != nullptr) {
        a1 = new (a1) ai::param_block{nullptr};
    }
}

template<>
void mash_info_struct::construct_class(nugget *&a1)
{
    if (a1 != nullptr) {
        a1 = static_cast<nugget *>(mash_virtual_base::construct_class_helper(a1));
    }
}

template<>
void mash_info_struct::construct_class(ai::mashed_state *&a1)
{
    if (a1 != nullptr) {
        a1 = new (a1) ai::mashed_state{nullptr};
    }
}

template<>
void mash_info_struct::construct_class(interactable_interface *&a1)
{
    if (a1 != nullptr) {
        from_mash_in_place_constructor *mash = nullptr;
        *a1 = interactable_interface{mash};
    }
}

template<>
void mash_info_struct::construct_class(base_ai_data *&a1)
{
    if (a1 != nullptr) {
        *a1 = base_ai_data{nullptr};
    }
}

template<>
void mash_info_struct::construct_class(patrol_def_set *&a1)
{
    if (a1 != nullptr) {
        a1 = new (a1) patrol_def_set{nullptr};
    }
}

template<>
void mash_info_struct::construct_class(glass_house *&a1)
{
    if (a1 != nullptr) {
        a1 = new (a1) glass_house{nullptr};
    }
}

template<>
void mash_info_struct::construct_class(ai::param_block::param_data_array *&a1)
{
    if (a1 != nullptr) {
        a1 = new (a1) ai::param_block::param_data_array{nullptr};
    }
}

template<>
void mash_info_struct::construct_class(combo_system *&a1)
{
    if (a1 != nullptr) {
        a1 = new (a1) combo_system{nullptr};
    }
}
