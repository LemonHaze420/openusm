#include "mvector.h"

#include "actor.h"
#include "ai_adv_strength_test_data.h"
#include "als_category.h"
#include "als_filter_data.h"
#include "als_post_kill_rule.h"
#include "als_post_layer_alter.h"
#include "als_state.h"
#include "alter_conditions.h"
#include "als_scripted_state.h"
#include "als_transition_rule.h"
#include "als_transition_group_base.h"
#include "als_meta_anim_base.h"
#include "anim_record.h"
#include "attach_action_trigger_enum.h"
#include "attach_node.h"
#include "base_state.h"
#include "combo_system.h"
#include "combo_system_move.h"
#include "combo_system_weapon.h"
#include "common.h"
#include "cut_scene_segment.h"
#include "entity_base_vhandle.h"
#include "entity_viseme_entry.h"
#include "fetext.h"
#include "femultilinetext.h"
#include "func_wrapper.h"
#include "gab_manager.h"
#include "interact_sound_entry.h"
#include "interaction.h"
#include "layer_state_machine_shared.h"
#include "mashed_state.h"
#include "meta_anim_interact.h"
#include "mash_virtual_base.h"
#include "memory.h"
#include "panelanim.h"
#include "panelanimkeyframe.h"
#include "panelanimfile.h"
#include "panelquad.h"
#include "panelquadsection.h"
#include "param_block.h"
#include "path_graph.h"
#include "patrol_def.h"
#include "sound_alias_database.h"
#include "token_def.h"
#include "trace.h"
#include "trigger_region.h"
#include "variables.h"
#include "vtbl.h"
#include "web_interface.h"

VALIDATE_SIZE(mVector<int>, 0x14);

template <>
void mVector<sound_alias>::destruct_mashed_class()
{
    if constexpr (0) {
        //this->clear();
    } else {
        THISCALL(0x005D6EE0, this);
    }
}

template <>
void mVector<als::layer_state_machine_shared>::destruct_mashed_class()
{
    THISCALL(0x004B01C0, this);
}

template <>
void mVector<als::als_meta_anim_base>::destruct_mashed_class()
{
    THISCALL(0x004B01C0, this);
}

template <>
void mVector<PanelAnim>::custom_unmash(mash_info_struct *a1, [[maybe_unused]] void *a3)
{
    TRACE("mVector<PanelAnim>::custom_unmash");
#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (PanelAnim **)a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            auto &a1a = this->m_data[i];
            auto *v6 = bit_cast<PanelAnim *>(a1->read_from_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                sizeof(PanelAnim),
                4));

            a1a = v6;
            a1->unmash_class_in_place(v6->field_0, v6);
        }
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<PanelAnimKeyframe>::custom_unmash(mash_info_struct *a2, [[maybe_unused]] void *a3)
{
    TRACE("mVector<PanelAnimKeyframe>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (PanelAnimKeyframe **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);

        for (auto i = 0; i < this->m_size; ++i) {
            auto &v5 = this->m_data[i];
            v5 = (PanelAnimKeyframe *)a2->read_from_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                sizeof(PanelAnimKeyframe),
                4);
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<PanelAnimFile>::custom_unmash(mash_info_struct *a2, [[maybe_unused]] void *a3)
{
#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (PanelAnimFile **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            auto &a1 = this->m_data[i];
            auto *v6 = a2->read_from_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                sizeof(PanelAnimFile),
                4);
            a1 = (PanelAnimFile *)v6;
            a2->unmash_class_in_place(a1->field_0, v6);
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<PanelQuadSection>::custom_unmash(mash_info_struct *a2, [[maybe_unused]] void *a3)
{
    TRACE("mVector<PanelQuadSection>::custom_unmash", std::to_string(this->m_size).c_str());

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (PanelQuadSection **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            auto &v5 = this->m_data[i];
            auto *v6 = (PanelQuadSection *)a2->read_from_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                sizeof(PanelQuadSection),
                4);

            v5 = v6;
            a2->unmash_class_in_place(v6->field_14, v6);
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<FEText>::custom_unmash(mash_info_struct *a2, [[maybe_unused]] void *a3)
{
    TRACE("mVector<FEText>::custom_unmash");

#ifdef TARGET_XBOX

    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else {
        this->m_data = bit_cast<FEText **>(a2->read_from_buffer(mash::NORMAL_BUFFER, 4 * this->m_size, 4));

        sp_log("size = %d", this->size());
        for (auto i = 0; i < this->m_size; ++i) {
            TRACE("mash_info_struct::unmash_class<FEText>");
            auto &v5 = this->m_data[i];

            {
                constexpr auto mash_size = 0x60;

                struct fetext {
                    struct string_t {
                        uint32_t m_size;
                        char *guts;
                        int field_8;
                    };

                    char field_0[0x1C];
                    string_t field_1C;
                    int field_28[2];
                    int field_30[2];
                    int field_38[2];
                    int field_40[2];
                    int field_48;
                    string_t field_4C;

                    struct {
                        int field_0[2];
                    } field_58;
                };

                const fetext *v6 = CAST(v6, a2->read_from_buffer(mash::NORMAL_BUFFER, mash_size, 0));

                sp_log("0x%08X", v6);
                VALIDATE_SIZE(fetext, mash_size);
                VALIDATE_OFFSET(fetext, field_4C, 0x4C);

                v5 = static_cast<FEText *>(malloc(sizeof(FEText)));

                {
                    std::memcpy(v5, v6, sizeof(v6->field_0));
                    std::memcpy(&v5->field_1C.m_size, &v6->field_1C, sizeof(v6->field_1C));
                    std::memcpy(&v5->field_2C, &v6->field_28, sizeof((int)&v6->field_28 - (int)&v6->field_4C));
                    std::memcpy(&v5->field_50.m_size, &v6->field_4C, sizeof(v6->field_4C));
                    std::memcpy(&v5->field_60, &v6->field_58, sizeof(v6->field_58));
                }

                mash_virtual_base::fixup_vtable(v5);

                {
                    //sp_log("0x%08X", tmp->m_vtbl);
                    assert(v5->m_vtbl == 0x00879FE0 || v5->m_vtbl == 0x0087AE58);
                }

                const auto v7 = v5->get_mash_sizeof();
                //sp_log("mash_size = 0x%X", v7);

                if (v7 == 0x98) {
                    struct multilinetext {
                        fetext base{};
                        char field_60[0x38];
                    } *text = CAST(text, v6);

                    VALIDATE_SIZE(multilinetext, 0x98);

                    auto *tmp = malloc(sizeof(FEMultiLineText));
                    std::memcpy(tmp, v5, sizeof(FEText));
                    std::memcpy(bit_cast<char *>(tmp) + sizeof(FEText), text->field_60, sizeof(text->field_60));

                    free(v5);
                    v5 = static_cast<FEText *>(tmp);
                }

                a2->advance_buffer(mash::NORMAL_BUFFER, v7 - mash_size);
            }

            v5->unmash(a2, nullptr);
        }
    }

#else

    if (this->m_data != nullptr) {
        this->m_data = bit_cast<FEText **>(a2->read_from_buffer(4 * this->m_size, 4));

        sp_log("size = %d", this->size());
        for (auto i = 0; i < this->m_size; ++i) {
            TRACE("mash_info_struct::unmash_class<FEText>");
            sp_log("i = %d", i);
            auto &v5 = this->m_data[i];
            auto *v6 = a2->read_from_buffer(sizeof(FEText), 0);
            sp_log("0x%08X", v6);

            v5 = CAST(v5, v6);

            sp_log("%d %d", v5->field_1C.m_size, v5->field_50.m_size);
            mash_virtual_base::fixup_vtable(v6);

            {
                struct {
                    int m_vtbl;
                } *tmp = CAST(tmp, v6);

                //sp_log("0x%08X", tmp->m_vtbl);
                assert(tmp->m_vtbl == 0x00879FE0 || tmp->m_vtbl == 0x0087AE58);
            }

            auto v7 = v5->get_mash_sizeof();
            a2->advance_buffer(v7 - sizeof(FEText));

            v5->unmash(a2, nullptr);
        }
    }
#endif

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}


template <>
void mVector<PanelQuad>::custom_unmash(mash_info_struct *a2, [[maybe_unused]] void *a3)
{
    TRACE("mVector<PanelQuad>::custom_unmash", std::to_string(this->m_size).c_str());

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else {
        this->m_data = bit_cast<PanelQuad **>(a2->read_from_buffer(mash::NORMAL_BUFFER, 4 * this->m_size, 4));

        for (auto i = 0; i < this->m_size; ++i) {
            sp_log("i = %d", i);
            TRACE("mash_info_struct::unmash_class<PanelQuad>");
            auto unmash_class = [](mash_info_struct *self, PanelQuad *&a2) {
                [](mash_info_struct *a2, PanelQuad *&v5) {
                    constexpr auto mash_size = 0x48;

                    struct {
                        char field_0[0x3C];
                        struct {
                            int m_size;
                            char *guts;
                            int field_8;
                        } field_3C;
                    } *v6 = CAST(v6, a2->read_from_buffer(mash::NORMAL_BUFFER, mash_size, 0));

                    v5 = static_cast<PanelQuad *>(malloc(sizeof(PanelQuad)));

                    {
                        //sp_log("0x%08X", v6->field_3C.guts);
                        std::memcpy(v5, v6, sizeof(v6->field_0));
                        std::memcpy(&v5->field_3C.m_size, &v6->field_3C, sizeof(v6->field_3C));
                    }

                    mash_virtual_base::fixup_vtable(v5);

                    const auto v7 = v5->get_mash_sizeof();

                    a2->advance_buffer(mash::NORMAL_BUFFER, v7 - mash_size);
                }(self, a2);

                a2->unmash(self, nullptr);
            };

            unmash_class(a2, this->m_data[i]);
        }
    }

#else
    if (this->m_data != nullptr) {
        this->m_data = bit_cast<PanelQuad **>(a2->read_from_buffer(4 * this->m_size, 4));

        for (auto i = 0; i < this->m_size; ++i) {
            sp_log("i = %d", i);

            auto unmash_class = [](mash_info_struct *self, PanelQuad *&a2) {
                [](mash_info_struct *a2, PanelQuad *&v5) {
                    constexpr auto mash_size = sizeof(PanelQuad);

                    auto *v6 = a2->read_from_buffer(mash_size, 0);

                    v5 = CAST(v5, v6);

                    mash_virtual_base::fixup_vtable(v6);

                    {
                        struct {
                            int m_vtbl;
                        } *tmp = CAST(tmp, v6);

                        assert(tmp->m_vtbl == 0x0087B990);
                    }

                    const auto v7 = v5->get_mash_sizeof();

                    a2->advance_buffer(v7 - mash_size);
                }(self, a2);

                a2->unmash(self, nullptr);
            };

            unmash_class(a2, this->m_data[i]);
        }
    }
#endif

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<sound_alias>::custom_unmash(mash_info_struct *a2, [[maybe_unused]] void *a3)
{
    TRACE("mVector<sound_alias>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (sound_alias **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);

        for (auto i = 0; i < this->m_size; ++i) {
            auto &a2a = this->m_data[i];
            auto *v6 = (sound_alias *)a2->read_from_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                sizeof(sound_alias),
                4);

            a2a = v6;
            a2->unmash_class_in_place(v6->field_0, v6);
            a2->unmash_class_in_place(v6->field_4, v6);
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<als::layer_state_machine_shared>::custom_unmash(mash_info_struct *a2, [[maybe_unused]] void *a3)
{
    TRACE("mVector<als::layer_state_machine_shared>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif

    {
        this->m_data = (value_type **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            auto &v5 = this->m_data[i];

            {
                auto *v6 = a2->read_from_buffer(
#ifdef TARGET_XBOX
                    mash::NORMAL_BUFFER,
#endif
                    sizeof(als::layer_state_machine_shared),
                    0);

                v5 = (als::layer_state_machine_shared *)v6;
                mash_virtual_base::fixup_vtable(v6);

#if !STANDALONE_SYSTEM
                {
                    struct {
                        int m_vtbl;
                    } *tmp = CAST(tmp, v6);

                    assert(tmp->m_vtbl == 0x0087E3A4);
                }
#endif

                auto v7 = v5->get_mash_sizeof();
                a2->advance_buffer(
#ifdef TARGET_XBOX
                    mash::NORMAL_BUFFER,
#endif
                    v7 - sizeof(als::layer_state_machine_shared));
            }

            v5->unmash(a2, nullptr);
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<interaction>::custom_unmash(mash_info_struct *a2, [[maybe_unused]] void *a3)
{
    TRACE("mVector<interaction>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif

    {
        this->m_data = (interaction **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            auto &v5 = this->m_data[i];

            {
                auto *v6 = a2->read_from_buffer(
#ifdef TARGET_XBOX
                    mash::NORMAL_BUFFER,
#endif
                    sizeof(als::layer_state_machine_shared),
                    0);

                v5 = (interaction *)v6;
                mash_virtual_base::fixup_vtable(v6);

                {
                    struct {
                        int m_vtbl;
                    } *tmp = CAST(tmp, v6);

                    assert(tmp->m_vtbl == 0x0087E3A4);
                }

                auto v7 = v5->get_mash_sizeof();
                a2->advance_buffer(
#ifdef TARGET_XBOX
                    mash::NORMAL_BUFFER,
#endif
                    v7 - sizeof(interaction));
            }

            v5->unmash(a2, nullptr);
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<als::state>::custom_unmash(mash_info_struct *a2, [[maybe_unused]] void *a3)
{
    TRACE("mVector<als::state>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (als::state **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);

        for (auto i = 0; i < this->m_size; ++i) {
            auto &v5 = this->m_data[i];
            auto *v6 = a2->read_from_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                sizeof(als::state),
                0);
            v5 = (als::state *)v6;

            mash_virtual_base::fixup_vtable(v5);

#if !STANDALONE_SYSTEM
            assert(v5->m_vtbl == 0x0087E1D8 || v5->m_vtbl == 0x0087E214);
#endif

            auto v7 = v5->get_mash_sizeof();
            a2->advance_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                v7 - sizeof(value_type));

            v5->unmash(a2, nullptr);
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (DWORD)this];
}

template <>
void mVector<als::als_meta_anim_base>::custom_unmash(mash_info_struct *a2, void *a3)
{
    TRACE("mVector<als::als_meta_anim_base>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (als::als_meta_anim_base **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a2->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<combo_system_move>::custom_unmash(mash_info_struct *a2, void *a3)
{
    if (this->m_data != nullptr) {
        this->m_data = (combo_system_move **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a2->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<combo_system_chain>::custom_unmash(mash_info_struct *a1, void *a3)
{
    if (this->m_data != nullptr) {
        this->m_data = (combo_system_chain **)a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a1->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<combo_system_chain::telegraph_info>::custom_unmash(mash_info_struct *a2, void *a3)
{
    TRACE("mVector<combo_system_chain::telegraph_info>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (combo_system_chain::telegraph_info **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a2->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<combo_system_move::link_info>::custom_unmash(mash_info_struct *a2, void *a3)
{
    if (this->m_data != nullptr) {
        this->m_data = (combo_system_move::link_info **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a2->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<combo_system_weapon>::custom_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("mVector<combo_system_weapon>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (combo_system_weapon **)a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a1->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<string_hash>::custom_unmash(mash_info_struct *a2, void *a3)
{
    TRACE("mVector<string_hash>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (string_hash **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a2->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<resource_key>::custom_unmash(mash_info_struct *a2, void *a3)
{
    TRACE("mVector<resource_key>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (resource_key **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);

#ifndef TARGET_XBOX
        for (auto i = 0; i < this->m_size; ++i) {
            a2->unmash_class(this->m_data[i], a3);
        }
#endif
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<als::meta_key_anim>::custom_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("mVector<als::meta_key_anim>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (als::meta_key_anim **)a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a1->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );

            //sp_log("%s", this->m_data[i]->field_0.to_string());
        }
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<als::category>::custom_unmash(mash_info_struct *a2, [[maybe_unused]] void *a3)
{
    TRACE("mVector<als::category>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (als::category **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            auto &v5 = this->m_data[i];
            auto *v6 = a2->read_from_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                sizeof(als::category),
                0);
            v5 = (als::category *)v6;
            mash_virtual_base::fixup_vtable(v5);

#if !STANDALONE_SYSTEM
            assert(v5->m_vtbl == 0x0087E250);
#endif

            auto v7 = v5->get_mash_sizeof();
            a2->advance_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                v7 - sizeof(als::category));
            v5->unmash(a2, nullptr);
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (DWORD)this];
}

template <>
void mVector<als::transition_group_base>::custom_unmash(mash_info_struct *a2, void *)
{
    TRACE("mVector<als::transition_group_base>::custom_unmash");
#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (als::transition_group_base **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            auto &v5 = this->m_data[i];
            auto *v6 = a2->read_from_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                4,
                0);
            v5 = (als::transition_group_base *)v6;
            mash_virtual_base::fixup_vtable(v5);

            auto v7 = v5->get_mash_sizeof();
            a2->advance_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                v7 - 4);
            v5->unmash(a2, nullptr);
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (DWORD)this];
}

template <>
void mVector<ai::param_block::param_data>::destroy_element(ai::param_block::param_data **a2)
{
    if (bit_cast<mContainer_base *>(this)->is_pointer_in_mash_image(*a2)) {
        (*a2)->destruct_mashed_class();
    } else if ((*a2) != nullptr) {
        delete (*a2);
    }

    *a2 = nullptr;
}

template <>
void mVector<ai::param_block::param_data>::clear()
{
    if constexpr (0) {
        if (this->field_10) {
            for (int i = 0; i < this->m_size; ++i) {
                this->destroy_element(&this->m_data[i]);
            }
        }

        if (!this->is_pointer_in_mash_image(this->m_data)) {
            mem_dealloc(this->m_data, 4 * this->m_max_size);
        }

        this->m_data = nullptr;
        this->m_max_size = 0;

        mContainer_base::clear();
    } else {
        THISCALL(0x0043E400, this);
    }
}

#if 0
template<>
void mVector<path_graph_node>::initialize(mash::allocation_scope scope)
{
    TRACE("mVector<path_graph_node>::initialize");

    if ( scope )
    {
        assert(scope == mash::FROM_MASH);

        if ( this->m_data != nullptr )
        {
            assert(this->m_size > 0);
            for ( int i = 0; i < this->m_size; ++i ) {
                new (this->m_data[i]) value_type {nullptr};
            }
        }
    }
    else
    {
        this->m_data = nullptr;
        this->m_max_size = 0;
        this->field_10 = true;
    }
}

template<>
void mVector<sound_alias>::initialize(mash::allocation_scope scope)
{
    TRACE("mVector<sound_alias>::initialize");

    if ( scope )
    {
        assert(scope == mash::FROM_MASH);

        if ( this->m_data != nullptr )
        {
            assert(this->m_size > 0);
            for ( int i = 0; i < this->m_size; ++i ) {
                new (this->m_data[i]) value_type {nullptr};
            }
        }
    }
    else
    {
        this->m_data = nullptr;
        this->m_max_size = 0;
        this->field_10 = true;
    }
}

template<>
void mVector<als::meta_key_anim>::initialize(mash::allocation_scope scope)
{
    TRACE("mVector<als::meta_key_anim>::initialize");

    if ( scope )
    {
        assert(scope == mash::FROM_MASH);

        if ( this->m_data != nullptr )
        {
            assert(this->m_size > 0);
            for ( int i = 0; i < this->m_size; ++i ) {
                new (this->m_data[i]) value_type {nullptr};
            }
        }
    }
    else
    {
        this->m_data = nullptr;
        this->m_max_size = 0;
        this->field_10 = true;
    }
}

template<>
void mVector<cut_scene_segment>::initialize(mash::allocation_scope scope)
{
    TRACE("mVector<cut_scene_segment>::initialize");

    if ( scope )
    {
        assert(scope == mash::FROM_MASH);

        if ( this->m_data != nullptr )
        {
            assert(this->m_size > 0);
            for ( int i = 0; i < this->m_size; ++i ) {
                new (this->m_data[i]) value_type {nullptr};
            }
        }
    }
    else
    {
        this->m_data = nullptr;
        this->m_max_size = 0;
        this->field_10 = true;
    }
}

template<>
void mVector<als::als_meta_anim_base>::initialize(
        mash::allocation_scope scope)
{

    if ( scope )
    {
        assert(scope == mash::FROM_MASH);
        if ( this->m_data != nullptr )
        {
            assert(this->m_size > 0);
            for ( auto i = 0; i < this->m_size; ++i ) {
                this->m_data[i] = (value_type *) mash_virtual_base::construct_class_helper(this->m_data[i]);
            }
        }
    }
    else
    {
        this->m_data = nullptr;
        this->m_max_size = 0;
        this->field_10 = true;
    }
}

template<>
void mVector<token_def>::initialize(mash::allocation_scope scope)
{
    TRACE("mVector<token_def>::initialize");

    if ( scope )
    {
        assert(scope == mash::FROM_MASH);

        if ( this->m_data != nullptr )
        {
            assert(this->m_size > 0);
            for ( int i = 0; i < this->m_size; ++i ) {
                new (this->m_data[i]) value_type {nullptr};
            }
        }
    }
    else
    {
        this->m_data = nullptr;
        this->m_max_size = 0;
        this->field_10 = true;
    }
}
#endif

template <>
void mVector<token_def>::destroy_element(token_def **a1)
{
    TRACE("mVector<token_def>::destroy_element");

    if (mContainer_base::is_pointer_in_mash_image(*a1)) {
        (*a1)->destruct_mashed_class();
    } else if (*a1) {
        delete (*a1);
    }

    (*a1) = nullptr;
}

template <>
void mVector<token_def>::clear()
{
    if constexpr (1) {
        if (this->field_10) {
            for (int i = 0; i < this->m_size; ++i) {
                this->destroy_element(&this->m_data[i]);
            }
        }

        if (!this->is_pointer_in_mash_image(this->m_data)) {
            mem_dealloc(this->m_data, 4 * this->m_max_size);
        }

        this->m_data = nullptr;
        this->m_max_size = 0;

        mContainer_base::clear();
    } else {
        assert(0);
    }
}


template <>
void mVector<ai::param_block::param_data>::destruct_mashed_class()
{
    this->finalize(mash::FROM_MASH);
    //mContainer_base::destruct_mashed_class();
}

template <>
void mVector<ai::param_block::param_data>::custom_unmash(mash_info_struct *a2, void *a3)
{
#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (value_type **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);

        for (auto i = 0; i < this->m_size; ++i) {
            auto &a1 = this->m_data[i];
            auto *v6 = (value_type *)a2->read_from_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                sizeof(value_type),
                4);
            a1 = v6;
            a1->unmash(a2, a3);
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (DWORD)this];
}


template <>
void mVector<token_def>::custom_unmash(mash_info_struct *a2, void *a3)
{
    TRACE("mVector<token_def>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (value_type **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);

        for (auto i = 0; i < this->m_size; ++i) {
            auto &a1 = this->m_data[i];
            auto *v6 = (value_type *)a2->read_from_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                sizeof(value_type),
                4);
            a1 = v6;
            a1->unmash(a2, a3);
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (DWORD)this];
}

template <>
void mVector<web_info_nugget>::reserve(int a2)
{
    if (a2 > this->m_max_size) {
        auto *v2 = static_cast<value_type **>(mem_alloc(4 * a2));
        if (this->m_data != nullptr) {
            if (this->m_size > 0) {
                std::memcpy(v2, this->m_data, 4 * this->m_size);
            }

            if (!this->is_pointer_in_mash_image(this->m_data)) {
                mem_dealloc(this->m_data, 4 * this->m_max_size);
            }
        }

        this->m_data = v2;
        this->m_max_size = a2;
    }
}

template <>
void mVector<web_interface>::reserve(int a2)
{
    if (a2 > this->m_max_size) {
        auto *v2 = static_cast<value_type **>(mem_alloc(4 * a2));
        if (this->m_data != nullptr) {
            if (this->m_size > 0) {
                std::memcpy(v2, this->m_data, 4 * this->m_size);
            }

            if (!this->is_pointer_in_mash_image(this->m_data)) {
                mem_dealloc(this->m_data, 4 * this->m_max_size);
            }
        }

        this->m_data = v2;
        this->m_max_size = a2;
    }
}

template <>
void mVector<als::meta_key_anim>::reserve(int a2)
{
    if (a2 > this->m_max_size) {
        auto *v2 = static_cast<value_type **>(mem_alloc(4 * a2));
        if (this->m_data != nullptr) {
            if (this->m_size > 0) {
                std::memcpy(v2, this->m_data, 4 * this->m_size);
            }

            if (!this->is_pointer_in_mash_image(this->m_data)) {
                mem_dealloc(this->m_data, 4 * this->m_max_size);
            }
        }

        this->m_data = v2;
        this->m_max_size = a2;
    }
}

template <>
void mVector<als::implicit_transition_rule>::custom_unmash(mash_info_struct *a2, void *)
{
#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (als::implicit_transition_rule **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            auto &v5 = this->m_data[i];
            auto *v6 = (als::implicit_transition_rule *)a2->read_from_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                0x24,
                4);
            v5 = v6;
            v5->unmash(a2, nullptr);
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (DWORD)this];
}

template <>
void mVector<als::layer_transition_rule>::custom_unmash(mash_info_struct *a1, void *a3)
{
#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (als::layer_transition_rule **)a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a1->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (DWORD)this];
}

template <>
void mVector<als::dest_weight_data>::custom_unmash(mash_info_struct *a1, void *)
{
#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (als::dest_weight_data **)a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            auto &v5 = this->m_data[i];
            auto *v6 = (als::dest_weight_data *)a1->read_from_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                8,
                4);
            v5 = v6;
            a1->unmash_class_in_place(v5->field_0, v6);
        }
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (DWORD)this];
}

template <>
void mVector<als::explicit_transition_rule>::custom_unmash(mash_info_struct *a1, void *)
{
#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (als::explicit_transition_rule **)a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            auto &a1a = this->m_data[i];
            auto *v6 = (als::explicit_transition_rule *)a1->read_from_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                40,
                4);
            a1a = v6;
            a1a->unmash(a1, nullptr);
            a1->unmash_class_in_place(a1a->field_24, a1a);
        }
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (DWORD)this];
}

template <>
void mVector<als::alter_conditions>::custom_unmash(mash_info_struct *a1, void *a3)
{
#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (als::alter_conditions **)a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);

        for (auto i = 0; i < this->m_size; ++i) {
            a1->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<als::incoming_transition_rule>::custom_unmash(mash_info_struct *a1, void *)
{
#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (als::incoming_transition_rule **)a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            auto &v5 = this->m_data[i];
            auto *v6 = (als::incoming_transition_rule *)a1->read_from_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                44,
                4);
            v5 = v6;
            v5->unmash(a1, nullptr);
        }
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<als::post_kill_rule>::custom_unmash(mash_info_struct *a2, void *a3)
{
#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (als::post_kill_rule **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a2->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<als::post_layer_alter>::custom_unmash(mash_info_struct *a1, void *a3)
{
#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (als::post_layer_alter **)a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a1->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<als::filter_data>::custom_unmash(mash_info_struct *a2, void *)
{
#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (als::filter_data **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a2->unmash_class(this->m_data[i],
                             this
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<ai::mashed_state>::custom_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("mVector<ai::mashed_state>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (ai::mashed_state **)a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a1->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<ai::base_state>::custom_unmash(mash_info_struct *a2, void *a3)
{
    TRACE("mVector<ai::base_state>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (ai::base_state **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a2->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<anim_record>::custom_unmash(mash_info_struct *a2, void *a3)
{
    TRACE("mVector<anim_record>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (anim_record **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a2->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<interact_sound_entry>::custom_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("mVector<interact_sound_entry>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (interact_sound_entry **)a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a1->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<ai_adv_strength_test_data>::custom_unmash(mash_info_struct *a2, void *a3)
{
    TRACE("mVector<ai_adv_strength_test_data>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (ai_adv_strength_test_data **)a2->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
            a2->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<attach_node>::custom_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("mVector<attach_node>::unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if (this->m_size <= 0) {
        this->m_data = nullptr;
    } else
#else
    if (this->m_data != nullptr)
#endif
    {
        this->m_data = (attach_node **)a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            4 * this->m_size,
            4);
        for (auto i = 0; i < this->m_size; ++i) {
#ifdef TARGET_XBOX
            auto &a1a = this->m_data[i];
            struct {
                char field_0[0x10];
                struct {
                    int field_0[3];
                } field_10;
                int field_1C[2];
            } *temp = CAST(temp, a1->read_from_buffer(mash::NORMAL_BUFFER, 0x24, 4));

            {
                std::memcpy(&a1a->field_0, temp->field_0, sizeof(temp->field_0));
                std::memcpy(&a1a->field_10.m_size, &temp->field_10, sizeof(temp->field_10));
                std::memcpy(&a1a->field_20, temp->field_1C, sizeof(temp->field_1C));
            }

            a1a->unmash(a1, a3);
#else
            a1->unmash_class(this->m_data[i], a3);
#endif
        }
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (uint32_t)this];
}

template <>
void mVector<entity_viseme_entry>::destroy_element(entity_viseme_entry **a1)
{
    if (this->is_pointer_in_mash_image(*a1)) {
        (*a1)->destruct_mashed_class();
    } else if ((*a1) != nullptr) {
        delete (*a1);
    }

    *a1 = nullptr;
}

template <>
void mVector<entity_viseme_entry>::clear()
{
    if (this->field_10) {
        for (int i = this->m_size; i > 0; --i) {
            this->destroy_element(&this->m_data[i]);
        }
    }

    if (!this->is_pointer_in_mash_image(this->m_data)) {
        mem_dealloc(this->m_data, 4 * this->m_max_size);
    }

    this->m_data = nullptr;
    this->m_max_size = 0;
    mContainer_base::clear();
}

template <>
void mVector<patrol_def>::destroy_element(patrol_def **a1)
{
    TRACE("mVector<patrol_def>::destroy_element");

    if (mContainer_base::is_pointer_in_mash_image(*a1)) {
        (*a1)->destruct_mashed_class();
    } else if (*a1) {
        delete (*a1);
    }

    (*a1) = nullptr;
}

template <>
void mVector<patrol_def>::clear()
{
    if (this->field_10) {
        for (int i = this->m_size; i > 0; --i) {
            this->destroy_element(&this->m_data[i]);
        }
    }

    if (!this->is_pointer_in_mash_image(this->m_data)) {
        mem_dealloc(this->m_data, 4 * this->m_max_size);
    }

    this->m_data = nullptr;
    this->m_max_size = 0;
    mContainer_base::clear();
}

template <>
void mVector<patrol_def>::destruct_mashed_class()
{
    this->finalize(mash::FROM_MASH);
    mContainer_base::destruct_mashed_class();
}

template <>
void mVector<entity_viseme_entry>::destruct_mashed_class()
{
    this->finalize(mash::FROM_MASH);
    mContainer_base::destruct_mashed_class();
}

template <>
void mVector<entity_viseme_entry>::custom_unmash(mash_info_struct *a2, void *)
{
    if (this->m_data != nullptr) {
        this->m_data = (value_type **)a2->read_from_buffer(4 * this->m_size, 4);
        for (int i = 0; i < this->m_size; ++i) {
            auto &v5 = this->m_data[i];
            auto *v6 = (value_type *)a2->read_from_buffer(8, 4);
            v5 = v6;
            v5->unmash(a2, v6);
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (DWORD)this];
}

template <>
void mVector<trigger_region>::custom_unmash(mash_info_struct *a2, void *)
{
    if (this->m_data != nullptr) {
        this->m_data = (trigger_region **)a2->read_from_buffer(4 * this->m_size, 4);
        for (int i = 0; i < this->m_size; ++i) {
            auto &v5 = this->m_data[i];
            auto *v6 = a2->read_from_buffer(sizeof(trigger_region), 0);
            v5 = (trigger_region *)v6;
            mash_virtual_base::fixup_vtable(v6);

            auto v7 = v5->get_mash_sizeof();
            a2->advance_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                v7 - sizeof(trigger_region));

            v5->unmash(a2, nullptr);
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (DWORD)this];
}

template <>
void mVector<patrol_def>::custom_unmash(mash_info_struct *a2, void *)
{
    if (this->m_data != nullptr) {
        this->m_data = (patrol_def **)a2->read_from_buffer(4 * this->m_size, 4);
        for (int i = 0; i < this->m_size; ++i) {
            auto &v5 = this->m_data[i];
            auto *v6 = (value_type *)a2->read_from_buffer(sizeof(value_type), 0);

            v5 = v6;
            v5->unmash(a2, v6);
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (DWORD)this];
}

template <>
void mVector<gab_archetype>::custom_unmash(mash_info_struct *a2, void *a3)
{
    TRACE("mVector<gab_archetype>::custom_unmash");

    if (this->m_data != nullptr) {
        this->m_data = (value_type **)a2->read_from_buffer(4 * this->m_size, 4);
        for (int i = 0; i < this->m_size; ++i) {
            a2->unmash_class(this->m_data[i],
                             a3
#ifdef TARGET_XBOX
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    }

    this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (DWORD)this];
}

template <>
void mVector<gab_expression>::custom_unmash(mash_info_struct *a2, void *a3)
{
    TRACE("mVector<gab_expression>::custom_unmash");

    if constexpr (1) {
        if (this->m_data != nullptr) {
            this->m_data = (value_type **)a2->read_from_buffer(4 * this->m_size, 4);
            for (int i = 0; i < this->m_size; ++i) {
                a2->unmash_class(this->m_data[i],
                                 a3
#ifdef TARGET_XBOX
                                 ,
                                 mash::NORMAL_BUFFER
#endif
                );
            }
        }

        this->field_0 = (int)&a2->mash_image_ptr[0][a2->buffer_size_used[0] - (DWORD)this];
    } else {
        void(__fastcall * func)(void *, void *edx, mash_info_struct *, void *) = CAST(func, 0x005E7000);
        func(this, nullptr, a2, a3);
    }
}
