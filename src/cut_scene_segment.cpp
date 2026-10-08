#include "cut_scene_segment.h"

#include "camera_setup_entry.h"
#include "common.h"
#include "func_wrapper.h"
#include "tracking_panel.h"
#include "entity_class_entry.h"
#include "nal_system.h"
#include "scene_anim.h"
#include "tlresource_directory.h"

VALIDATE_SIZE(cut_scene_segment, 0xB0u);

cut_scene_segment::cut_scene_segment(from_mash_in_place_constructor *a2)
    : field_10(a2), field_20(a2), field_34(a2), field_48(a2), field_5C(a2), field_70(a2), field_84(a2), field_98(a2)
{
    for (int i = 0; i < field_10.size(); ++i) {
        auto &animation = field_10.at(i);
        const auto hash = reinterpret_cast<std::uintptr_t>(animation);
        if (auto *resource = nalGetSceneAnimDirectory()->Find(hash))
            animation = resource;
    }
}

void cut_scene_segment::destruct_mashed_class()
{
    if (!field_10.is_pointer_in_mash_image(field_10.m_data))
        ::operator delete[](field_10.m_data);
    field_10.m_data = nullptr;
    field_10.m_max_size = 0;
    field_10.mContainer_base::clear();
    field_10.mContainer_base::destruct_mashed_class();
    field_20.destruct_mashed_class();
    field_34.destruct_mashed_class();
    field_48.destruct_mashed_class();
    field_5C.destruct_mashed_class();
    field_70.destruct_mashed_class();
    field_84.destruct_mashed_class();
    field_94.destruct_mashed_class();
    field_98.destruct_mashed_class();
    field_C = nullptr;
}

void cut_scene_segment::unmash(mash_info_struct *a1, [[maybe_unused]] void *a3)
{
    a1->unmash_class_in_place(this->field_10, this);
    a1->unmash_class_in_place(this->field_20, this);
    a1->unmash_class_in_place(this->field_34, this);
    a1->unmash_class_in_place(this->field_48, this);
    a1->unmash_class_in_place(this->field_5C, this);
    a1->unmash_class_in_place(this->field_70, this);
    a1->unmash_class_in_place(this->field_84, this);
    a1->unmash_class_in_place(this->field_94, this);
    a1->unmash_class_in_place(this->field_98, this);

    if (this->field_C != nullptr) {
        this->field_C = reinterpret_cast<replay_info *>(a1->read_from_buffer(sizeof(void *), 4));
    }
}

template <>
void mVector<cut_scene_segment>::custom_unmash(mash_info_struct *a1, void *)
{
    if (this->m_data != nullptr) {
        this->m_data = reinterpret_cast<value_type **>(a1->read_from_buffer(sizeof(value_type *) * this->m_size, 4));
        for (int i = 0; i < this->m_size; ++i) {
            auto *segment = reinterpret_cast<value_type *>(a1->read_from_buffer(sizeof(value_type), 4));
            this->m_data[i] = segment;
            segment->unmash(a1, nullptr);
        }
    }

    this->field_0 = reinterpret_cast<int>(&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (DWORD)this]);
}

template <>
void mVectorBasic<nalSceneAnim *>::unmash(mash_info_struct *a1, [[maybe_unused]] void *a3)
{
    if (this->m_data != nullptr) {
        this->m_data = reinterpret_cast<value_type *>(a1->read_from_buffer(sizeof(value_type) * this->m_size, 4));
    }
    this->field_0 = reinterpret_cast<int>(&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (DWORD)this]);
}

template <>
void mVector<mString>::custom_unmash(mash_info_struct *a1, [[maybe_unused]] void *a3)
{
    if (this->m_data != nullptr) {
        this->m_data = reinterpret_cast<value_type **>(a1->read_from_buffer(sizeof(value_type *) * this->m_size, 4));
        for (int i = 0; i < this->m_size; ++i) {
            auto *value = reinterpret_cast<value_type *>(a1->read_from_buffer(sizeof(value_type), 4));
            this->m_data[i] = value;
            a1->unmash_class_in_place(*value, nullptr);
        }
    }
    this->field_0 = reinterpret_cast<int>(&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (DWORD)this]);
}

template <>
void mVector<mVector<mString>>::custom_unmash(mash_info_struct *a1, [[maybe_unused]] void *a3)
{
    if (this->m_data != nullptr) {
        this->m_data = reinterpret_cast<value_type **>(a1->read_from_buffer(sizeof(value_type *) * this->m_size, 4));
        for (int i = 0; i < this->m_size; ++i) {
            auto *value = reinterpret_cast<value_type *>(a1->read_from_buffer(sizeof(value_type), 4));
            this->m_data[i] = value;
            a1->unmash_class_in_place(*value, nullptr);
        }
    }
    this->field_0 = reinterpret_cast<int>(&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (DWORD)this]);
}

template <>
void mVector<camera_setup_entry>::custom_unmash(mash_info_struct *a1, [[maybe_unused]] void *a3)
{
    if (this->m_data != nullptr) {
        this->m_data = reinterpret_cast<value_type **>(a1->read_from_buffer(sizeof(value_type *) * this->m_size, 4));
        for (int i = 0; i < this->m_size; ++i) {
            auto *value = reinterpret_cast<value_type *>(a1->read_from_buffer(sizeof(value_type), 4));
            this->m_data[i] = value;
            value->unmash(a1, value);
        }
    }
    this->field_0 = reinterpret_cast<int>(&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (DWORD)this]);
}

template <>
void mVector<tracking_panel>::custom_unmash(mash_info_struct *a1, [[maybe_unused]] void *a3)
{
    if (this->m_data != nullptr) {
        this->m_data = reinterpret_cast<value_type **>(a1->read_from_buffer(sizeof(value_type *) * this->m_size, 4));
        for (int i = 0; i < this->m_size; ++i) {
            auto *value = reinterpret_cast<value_type *>(a1->read_from_buffer(sizeof(value_type), 4));
            this->m_data[i] = value;
            value->unmash(a1, value);
        }
    }
    this->field_0 = reinterpret_cast<int>(&a1->mash_image_ptr[0][a1->buffer_size_used[0] - (DWORD)this]);
}
