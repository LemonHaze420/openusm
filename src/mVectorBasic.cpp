#include "mvector.h"

#include "actor.h"
#include "ai_adv_strength_test_data.h"
#include "anim_record.h"
#include "attach_action_trigger_enum.h"
#include "attach_node.h"
#include "common.h"
#include "als_filter_data.h"
#include "als_post_kill_rule.h"
#include "als_post_layer_alter.h"
#include "als_scripted_state.h"
#include "als_transition_rule.h"
#include "als_transition_group_base.h"
#include "combo_system.h"
#include "combo_system_move.h"
#include "combo_system_weapon.h"
#include "func_wrapper.h"
#include "gab_manager.h"
#include "mash_virtual_base.h"
#include "memory.h"
#include "param_block.h"
#include "sound_alias_database.h"
#include "subdivision_obb.h"
#include "trace.h"

VALIDATE_SIZE(mVectorBasic<int>, 0x10);

template<>
void mVectorBasic<subdivision_node_large_obb>::destruct_mashed_class() {
    if ( !this->is_pointer_in_mash_image(this->m_data) ) {
      operator delete[](this->m_data);
    }

    this->m_data = nullptr;
    this->m_max_size = 0;
    this->clear();
    mContainer_base::destruct_mashed_class();
}

template<>
void mVectorBasic<attach_action_trigger_enum>::custom_unmash(mash_info_struct *a1, void *)
{
    TRACE("mVectorBasic<attach_action_trigger_enum>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if ( this->m_size <= 0 )
    {
        this->m_data = nullptr;
    }
    else
#else
    if ( this->m_data != nullptr )
#endif
    {
        this->m_data = bit_cast<value_type *>(a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
                4 * this->m_size, 4));
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (DWORD) this];
}

template<>
void mVectorBasic<attach_action_trigger_enum>::unmash(mash_info_struct *a1, void *a2)
{
#ifdef TARGET_XBOX
    [](mash_info_struct *a1, mash::buffer_type a2, uint32_t &a3)
    {
        a3 = *bit_cast<int *>(a1->read_from_buffer(a2, 4, 4));
    }(a1, mash::SHARED_BUFFER, m_size);
#endif

    this->custom_unmash(a1, a2);
}

template<>
void mVectorBasic<int>::custom_unmash(mash_info_struct *a1, void *)
{
    TRACE("mVectorBasic<int>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if ( this->m_size <= 0 )
    {
        this->m_data = nullptr;
    }
    else
#else
    if ( this->m_data != nullptr )
#endif
    {
        this->m_data = (int *) a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
                4 * this->m_size, 4);
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (DWORD) this];
}

template<>
void mVectorBasic<int>::unmash(mash_info_struct *a1, void *a2)
{
#ifdef TARGET_XBOX
    [](mash_info_struct *a1, mash::buffer_type a2, uint32_t &a3)
    {
        a3 = *bit_cast<int *>(a1->read_from_buffer(a2, 4, 4));
    }(a1, mash::SHARED_BUFFER, m_size);
#endif

    this->custom_unmash(a1, a2);
}

template<>
void mVectorBasic<vhandle_type<actor>>::custom_unmash(mash_info_struct *a1, void *)
{
    TRACE("mVectorBasic<int>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if ( this->m_size <= 0 )
    {
        this->m_data = nullptr;
    }
    else
#else
    if ( this->m_data != nullptr )
#endif
    {
        this->m_data = bit_cast<value_type *>(a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
                4 * this->m_size, 4));
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (DWORD) this];
}

template<>
void mVectorBasic<vhandle_type<actor>>::unmash(mash_info_struct *a1, void *a2)
{
#ifdef TARGET_XBOX
    [](mash_info_struct *a1, mash::buffer_type a2, uint32_t &a3)
    {
        a3 = *bit_cast<int *>(a1->read_from_buffer(a2, 4, 4));
    }(a1, mash::SHARED_BUFFER, m_size);
#endif

    this->custom_unmash(a1, a2);
}

template<>
void mVectorBasic<vhandle_type<actor>>::reserve(int a2)
{
    if ( a2 > this->m_max_size )
    {
        auto *mem = operator new(4 * a2);
        auto *v2 = new (mem) vhandle_type<actor>[a2] {};

        if ( this->m_data != nullptr )
        {
            if ( this->m_size > 0 ) {
                std::memcpy(v2, this->m_data, 4 * this->m_size);
            }

            if ( !this->is_pointer_in_mash_image(this->m_data) ) {
                operator delete[](this->m_data);
            }
        }

        this->m_data = v2;
        this->m_max_size = a2;
    }
}

template<>
void mVectorBasic<gab_source>::custom_unmash(mash_info_struct *a1, void *)
{
    TRACE("mVectorBasic<gab_source>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if ( this->m_size <= 0 )
    {
        this->m_data = nullptr;
    }
    else
#else
    if ( this->m_data != nullptr )
#endif
    {
        this->m_data = bit_cast<value_type *>(a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            8 * this->m_size, 4));
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (DWORD) this];
}


template<>
void mVectorBasic<gab_source>::unmash(mash_info_struct *a1, void *a2)
{
#ifdef TARGET_XBOX
    [](mash_info_struct *a1, mash::buffer_type a2, uint32_t &a3)
    {
        a3 = *bit_cast<int *>(a1->read_from_buffer(a2, 4, 4));
    }(a1, mash::SHARED_BUFFER, m_size);
#endif

    this->custom_unmash(a1, a2);
}

template<>
void mVectorBasic<subdivision_node_large_obb>::custom_unmash(mash_info_struct *a1, void *)
{
    TRACE("mVectorBasic<glass_house>::custom_unmash");

#ifdef TARGET_XBOX
    this->field_C = this->m_size;
    if ( this->m_size <= 0 )
    {
        this->m_data = nullptr;
    }
    else
#else
    if ( this->m_data != nullptr )
#endif
    {
        this->m_data = bit_cast<value_type *>(a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
            sizeof(value_type) * this->m_size, 4));
    }

    this->field_0 = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (DWORD) this];
}


template<>
void mVectorBasic<subdivision_node_large_obb>::unmash(mash_info_struct *a1, void *a2)
{
#ifdef TARGET_XBOX
    [](mash_info_struct *a1, mash::buffer_type a2, uint32_t &a3)
    {
        a3 = *bit_cast<int *>(a1->read_from_buffer(a2, 4, 4));
    }(a1, mash::SHARED_BUFFER, m_size);
#endif

    this->custom_unmash(a1, a2);
}
