#include "fx_cache.h"

#include "common.h"
#include "func_wrapper.h"
#include "oldmath_po.h"
#include "trace.h"
#include "utility.h"
#include "wds.h"

VALIDATE_SIZE(fx_cache, 0x14u);

fx_cache_ent::fx_cache_ent() : field_0(-1.0f) {}

void fx_cache_ent::un_mash(generic_mash_header *a2, cached_special_effect *a3, void *a4, generic_mash_data_ptrs *a5)
{
    TRACE("fx_cache_ent::un_mash");

    if constexpr (1) {
        auto &v6 = this->field_8;
        if (this->field_8.get_volatile_ptr() == nullptr) {
            mString v16{"fx\\"};
            po v11{identity_matrix};
            auto v10 = make_unique_entity_id();
            auto v9 = a3->field_8.m_hash;
            auto *v8 = g_world_ptr->ent_mgr.create_and_add_entity_or_subclass(v9, v10, v11, v16, 129, nullptr);
            v6 = {v8->my_handle};
            v8->set_active(false);
            v8->set_visible(false, false);
            this->field_0 = -1.0;
            this->field_4 = 0;
        }
    } else {
        THISCALL(0x004CD880, this, a2, a3, a4, a5);
    }
}

fx_cache::fx_cache() {}

fx_cache::~fx_cache()
{
    if (this->field_8.size() != 0) {
        for (auto &v3 : this->field_8) {
            auto &v4 = v3.field_8;
            if (v4.get_volatile_ptr() != nullptr) {
                auto *ent = v4.get_volatile_ptr();
                if (ent != nullptr) {
                    g_world_ptr->ent_mgr.destroy_entity(ent);
                }

                v4 = {};
            }
        }
    }

    if (!this->field_8.field_7 && this->field_8.m_data != nullptr) {
        operator delete[](this->field_8.m_data);
    }

    this->field_8.m_data = nullptr;
    this->field_8.m_size = 0;
}

void fx_cache::frame_advance(Float a3)
{
    TRACE("fx_cache::frame_advance");

    sp_log("0x%08X", this);

    THISCALL(0x004D4FB0, this, a3);
}

void fx_cache::un_mash(generic_mash_header *a2, cached_special_effect *a3, void *, generic_mash_data_ptrs *a5)
{
    TRACE("fx_cache::un_mash");

    auto *v6 = &this->field_8;
    v6->custom_un_mash(a2, v6, a5, a3);
    if (this->field_0 == 0) {
        this->field_10 = v6->m_data;
    }

    ++this->field_0;

    sp_log("0x%08X", this);
}

void fx_cache_patch()
{
    {
        FUNC_ADDRESS(address, &fx_cache::frame_advance);
        REDIRECT(0x004EC61C, address);
    }
}
