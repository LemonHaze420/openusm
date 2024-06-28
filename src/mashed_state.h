#pragma once

#include "mash_virtual_base.h"
#include "param_block.h"

struct from_mash_in_place_constructor;

namespace ai {

struct mashed_state {
    param_block field_0;
    string_hash field_C;
    uint32_t field_10;
    mash::virtual_types_enum field_14;

    mashed_state();

    mashed_state(from_mash_in_place_constructor *a2);

    void unmash(mash_info_struct *, void *);

    string_hash get_name() const;

    bool is_flag_set(uint8_t a2) const
    {
        return (this->field_10 & (1 << a2)) != 0;
    }

    bool operator>(const mashed_state &state);

    bool operator<(const mashed_state &state);

    static inline constexpr auto IS_INTERRUPT_STATE = 0;
};

} // namespace ai
