#include "cached_special_effect.h"

#include "common.h"
#include "func_wrapper.h"
#include "fx_cache.h"
#include "parse_generic_mash.h"
#include "trace.h"
#include "utility.h"
#include "wds.h"

VALIDATE_SIZE(cached_special_effect, 0x40u);

cached_special_effect::cached_special_effect()
{
    TRACE("cached_special_effect::cached_special_effect");

    this->field_10 = vector3d{1.0f, 0.0f, 0.0f};
    this->field_8.set_type(RESOURCE_KEY_TYPE_ENTITY);
    this->field_28 = nullptr;
    this->field_2C = -1.0f;
    this->field_30 = nullptr;
    this->field_3C = false;
    this->field_3D = false;
    this->field_38 = 5;
}

void cached_special_effect::initialize() {}

void cached_special_effect::frame_advance(Float a2)
{
    auto *v2 = this->field_30;
    if (v2 != nullptr) {
        v2->frame_advance(a2);
    }
}

void cached_special_effect::spawn(bool a1, const vector3d &a2, const vector3d &a3, handheld_item *a6, entity_base *a7,
                                  entity_base *a8, const vector3d &a9, bool a10, bool a11)
{
    TRACE("cached_special_effect::spawn");

    THISCALL(0x004EFC00, this, a1, &a2, &a3, a6, a7, a8, &a9, a10, a11);
}

void cached_special_effect::fill_cache()
{
    TRACE("cached_special_effect::fill_cache");

    if constexpr (0) {
    } else {
        THISCALL(0x004D4E10, this);
    }
}

void cached_special_effect::un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4)
{
    TRACE("cached_special_effect::un_mash");

    if constexpr (1) {
        this->field_24 = 0;
        this->field_30 = nullptr;
        this->field_28 = nullptr;

        a4->rebase_shared(4u);

        auto v5 = *a4->get_from_shared<int>();

        this->field_1C = a4->get_from_shared<char>(v5);

        a4->rebase_shared(4u);

        auto v9 = *a4->get_from_shared<int>();

        this->field_20 = a4->get_from_shared<char>(v9);

        a4->rebase_shared(4u);

        this->field_30 = a4->get_from_shared<fx_cache>();

        auto *v13 = this->field_30;
        v13->un_mash(a2, this, v13, a4);

        this->field_3C = false;
        this->field_3D = false;
    } else {
        THISCALL(0x004D3650, this, a2, a3, a4);
    }
}

void cached_special_effect::release_mem()
{
    auto *v2 = this->field_30;
    if (v2 != nullptr) {
        void(__fastcall * sub_4CE5D0)(void *) = CAST(sub_4CE5D0, 0x004CE5D0);
        sub_4CE5D0(&v2->field_8);

        --v2->field_0;
        this->field_30 = nullptr;
    }

    if (this->field_1C != nullptr && this->field_3C) {
        operator delete[](this->field_1C);
        this->field_1C = nullptr;
    }

    if (this->field_20 != nullptr && this->field_3D) {
        operator delete[](this->field_20);
        this->field_20 = nullptr;
    }

    if (this->field_28 != nullptr) {
        g_world_ptr->ent_mgr.destroy_entity(this->field_28);
        this->field_28 = nullptr;
    }
}

void *__fastcall cached_special_effect_constructor(void *mem)
{
    return new (mem) cached_special_effect{};
}

void cached_special_effect_patch()
{
    {
        auto *func = cached_special_effect_constructor;
        REDIRECT(0x004DEABA, func);
    }

    {
        FUNC_ADDRESS(address, &cached_special_effect::un_mash);
        REDIRECT(0x004D9EF8, address);
    }

    {
        FUNC_ADDRESS(address, &cached_special_effect::fill_cache);
        REDIRECT(0x0054AC6D, address);
        REDIRECT(0x0054AD0D, address);
    }
}
