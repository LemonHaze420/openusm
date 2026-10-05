#include "combo_system.h"

#include "combo_system_move.h"
#include "combo_system_weapon.h"
#include "common.h"
#include "mash_info_struct.h"
#include "trace.h"
#include "vtbl.h"


#include "memory.h"

#include <cstdint>

VALIDATE_SIZE(combo_system_chain, 0x44);
VALIDATE_SIZE(combo_system_chain::telegraph_info, 0xC);
VALIDATE_SIZE(combo_system, 0x50);

#if defined(OPENUSM_XBPACK_V10) && !defined(TARGET_XBOX)
namespace {
struct pc_mash_info {
    uint8_t *image;
    int used;
    int size;
    int field_C;
};

static_assert(sizeof(pc_mash_info) == 0x10);
}  // namespace
#endif

void combo_system_chain::telegraph_info::_unmash(mash_info_struct *, void *)
{
    TRACE("combo_system_chain::telegraph_info::unmash");

    ;
}

int combo_system_chain::telegraph_info::get_mash_sizeof()
{
    int(__fastcall * func)(combo_system_chain::telegraph_info *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    return func(this);
}

combo_system_chain::combo_system_chain(from_mash_in_place_constructor *a2) : field_0(a2), field_14(a2), field_1C(a2)
{
    this->initialize(mash::FROM_MASH);
}

void combo_system_chain::initialize(mash::allocation_scope a2)
{
    if (a2 == mash::ALLOCATED) {
        this->field_18 = 0;
        this->field_2C = 0;
        this->field_30 = 0x40000000;
        this->field_34 = 0;
        this->field_38 = 3.4028235e38;
        this->field_3C = 1.0;
        this->field_40 = -1.0;
    }
}

void combo_system_chain::unmash(mash_info_struct *a1, void *)
{
    TRACE("combo_system_chain::unmash");

    a1->unmash_class_in_place(this->field_0, this);
    a1->unmash_class_in_place(this->field_14, this);
    a1->unmash_class_in_place(this->field_1C, this);
}

combo_system::combo_system() {}

combo_system::combo_system(from_mash_in_place_constructor *a2) : field_0(a2), field_14(a2), field_28(a2), field_3C(a2)
{}


namespace {
template <typename T, typename Cleanup>
void clear_combo_vector(mVector<T> &vector, Cleanup cleanup)
{
    if (vector.field_10) {
        for (int i = 0; i < vector.m_size; ++i) {
            auto *element = vector.m_data[i];
            if (element != nullptr) {
                cleanup(*element);
                if (!vector.is_pointer_in_mash_image(element))
                    ::operator delete(element);
            }
            vector.m_data[i] = nullptr;
        }
    }
    if (!vector.is_pointer_in_mash_image(vector.m_data))
        mem_dealloc(vector.m_data, sizeof(*vector.m_data) * vector.m_max_size);
    vector.m_data = nullptr;
    vector.m_max_size = 0;
    vector.mContainer_base::clear();
    vector.mContainer_base::destruct_mashed_class();
}
}

void combo_system::destruct_mashed_class()
{
    clear_combo_vector(field_0, [](combo_system_move &move) {
        auto &result = move.field_4;
        result.field_4.destruct_mashed_class();
        result.field_8.destruct_mashed_class();
        result.field_C.destruct_mashed_class();
        result.field_10.destruct_mashed_class();
        clear_combo_vector(move.field_80.field_30,
                           [](combo_system_move::link_info &link) { link.field_4.destruct_mashed_class(); });
    });
    clear_combo_vector(field_14, [](combo_system_chain &chain) {
        clear_combo_vector(chain.field_0, [](combo_system_chain::telegraph_info &) {});
        chain.field_14.destruct_mashed_class();
        if (!chain.field_1C.is_pointer_in_mash_image(chain.field_1C.m_data))
            ::operator delete[](chain.field_1C.m_data);
        chain.field_1C.m_data = nullptr;
        chain.field_1C.m_max_size = 0;
        chain.field_1C.mContainer_base::clear();
        chain.field_1C.mContainer_base::destruct_mashed_class();
    });
    clear_combo_vector(field_28, [](combo_system_weapon &weapon) {
        weapon.field_0.destruct_mashed_class();
        weapon.field_8.destruct_mashed_class();
        weapon.field_10.destruct_mashed_class();
        weapon.field_18.destruct_mashed_class();
        weapon.field_1C.destruct_mashed_class();
    });
    clear_combo_vector(field_3C, [](string_hash &hash) { hash.destruct_mashed_class(); });
}

combo_system_weapon *combo_system::get_weapon(int idx)
{
    return this->field_28.m_data[(uint16_t)idx];
}

int combo_system::get_num_weapons()
{
    return this->field_28.size();
}

void combo_system::unmash(mash_info_struct *a1, [[maybe_unused]] void *a3)
{
    TRACE("combo_system::unmash");

    if constexpr (OPENUSM_XBOX_MASH_FORMAT) {
#if defined(OPENUSM_XBPACK_V10) && !defined(TARGET_XBOX)
        auto *pc_mash = reinterpret_cast<pc_mash_info *>(a1);
        mash_info_struct mash_ctx{pc_mash->image, pc_mash->size};
        mash_ctx.buffer_size_used[mash::NORMAL_BUFFER] = pc_mash->used;
        a1 = &mash_ctx;
#endif

        a1->unmash_class_in_place(this->field_0, this);

        a1->unmash_class_in_place(this->field_14, this);

        a1->unmash_class_in_place(this->field_28, this);

        a1->unmash_class_in_place(this->field_3C, this);

#if defined(OPENUSM_XBPACK_V10) && !defined(TARGET_XBOX)
        pc_mash->used = a1->buffer_size_used[mash::NORMAL_BUFFER];
#endif
    } else {
        a1->unmash_class_in_place(this->field_0, this);
        a1->unmash_class_in_place(this->field_14, this);
        a1->unmash_class_in_place(this->field_28, this);
        a1->unmash_class_in_place(this->field_3C, this);
    }
}

void combo_system_patch()
{
    {
        FUNC_ADDRESS(address, &combo_system_chain::telegraph_info::_unmash);
        set_vfunc(0x008737C4, address);
    }

    {
        FUNC_ADDRESS(address, &combo_system::unmash);
        REDIRECT(0x006D7265, address);
    }
}
