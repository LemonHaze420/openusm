#include "base_ai_res_state_graph.h"

#include "binary_search_array_deref.h"
#include "common.h"
#include "func_wrapper.h"
#include "mashed_state.h"
#include "trace.h"
#include "utility.h"

namespace ai {

VALIDATE_SIZE(state_graph, 0x34);

state_graph::state_graph()
{

}

void state_graph::destruct_mashed_class()
{
    THISCALL(0x006DA0D0, this);
}

void state_graph::unmash(mash_info_struct *a1, void *a3)
{
    TRACE("ai::state_graph::unmash");

    a1->unmash_class_in_place(this->field_0, this);
    a1->unmash_class_in_place(this->my_states, this);
    a1->unmash_class_in_place(this->field_20, this);

#ifdef TARGET_XBOX
    {
        uint8_t class_mashed = -1;
        class_mashed = *a1->read_from_buffer(mash::SHARED_BUFFER, 1, 1);
        assert(class_mashed == 0xAF || class_mashed == 0);
    }
#endif

    if ( this->field_1C != nullptr )
    {
        a1->unmash_class(this->field_1C, this
#ifdef TARGET_XBOX
            , mash::NORMAL_BUFFER
#endif
                );
    }
}

void sub_86B3C0() {
    CDECL_CALL(0x0086B3C0);
}

unsigned int state_graph::get_size_memory_block() const {
    auto v1 = this->my_states.size();
    auto result = 0;
    if (v1 > 0) {
        do {
            if (result < 24) {
                result = 24;
            }

            --v1;
        } while (v1);
    }

    return result;
}

resource_key state_graph::sub_6B68F0() const {
    return this->field_0;
}

mashed_state *state_graph::find_state(string_hash a2) const {
    if constexpr (1) {
        static mashed_state searcher{};

        searcher.field_C = a2;
        auto **data = this->my_states.m_data;
        auto size = this->my_states.m_size;
        int index = -1;

        mashed_state *result = nullptr;
        if (binary_search_array_deref(&searcher, data, size, &index)) {
            assert(index >= 0);

            result = data[index];
        }

        return result;

    } else {
        return (mashed_state *) THISCALL(0x006D8480, this, a2);
    }
}
} // namespace ai

void state_graph_patch() {
    {
        FUNC_ADDRESS(address, &ai::state_graph::find_state);
        SET_JUMP(0x006D8480, address);
    }
}
