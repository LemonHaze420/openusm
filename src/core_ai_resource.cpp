#include "core_ai_resource.h"

#include "combo_system.h"
#include "common.h"
#include "func_wrapper.h"
#include "resource_key.h"
#include "resource_manager.h"
#include "trace.h"
#include "utility.h"

#include <cassert>

namespace ai {

VALIDATE_SIZE(core_ai_resource, 0x48);
VALIDATE_OFFSET(core_ai_resource, my_locomotion_graphs, 0x28);

core_ai_resource::core_ai_resource(from_mash_in_place_constructor *a2)
    : field_0(a2), my_base_graphs(a2), my_locomotion_graphs(a2)
{
    TRACE("core_ai_resource::core_ai_resource");

    if constexpr (1) {
        if (this->field_10 != nullptr) {
            mash_info_struct::construct_class(this->field_10);
        }

        this->initialize(mash::FROM_MASH);
    } else {
        THISCALL(0x006D9A10, this, a2);
    }
}

void core_ai_resource::initialize(mash::allocation_scope scope)
{
    if ( scope )
    {
        assert(scope == mash::FROM_MASH);

        this->field_3C = resource_manager::get_resource_context();
    }
    else
    {
        this->field_3C = nullptr;
        this->field_40 = 0;
        this->field_C = 0;
        this->field_10 = nullptr;
        this->field_44 = false;
    }
}

int core_ai_resource::destruct_mashed_class() {
    return THISCALL(0x006D71A0, this);
}

void core_ai_resource::unmash(mash_info_struct *a1, void *a3)
{
    TRACE("ai::core_ai_resource::unmash");

    if constexpr (1)
    {
        a1->unmash_class_in_place(this->field_0, this);
        a1->unmash_class_in_place(this->my_base_graphs, this);
        a1->unmash_class_in_place(this->my_locomotion_graphs, this);

#ifdef TARGET_XBOX
        {
            uint8_t class_mashed = -1;
            class_mashed = *a1->read_from_buffer(mash::SHARED_BUFFER, 1, 1);
            assert(class_mashed == 0xAF || class_mashed == 0);
        }
#endif

        if ( this->field_10 != nullptr )
        {
            a1->unmash_class(this->field_10, this
#ifdef TARGET_XBOX
                , mash::NORMAL_BUFFER
#endif
                    );
        }

        a1->align_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
                4);
        this->field_40 = *(int*) a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
                4, 4);
        a1->align_buffer(
#ifdef TARGET_XBOX
                mash::NORMAL_BUFFER,
#endif
                16);
        auto v6 = this->field_40;
        this->field_C = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0]];
        a1->advance_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
                v6);
        a1->read_from_buffer(
#ifdef TARGET_XBOX
            mash::NORMAL_BUFFER,
#endif
                4, 4);
    }
    else
    {
        THISCALL(0x006D71F0, this, a1, a3);
    }
}

bool core_ai_resource::does_base_graph_exist(resource_key the_graph) const
{
    assert(the_graph.get_type() == RESOURCE_KEY_TYPE_AI_STATE_GRAPH);

    for (auto &curr : this->my_base_graphs)
    {
        assert(curr->get_type() == RESOURCE_KEY_TYPE_AI_STATE_GRAPH);

        if (*curr == the_graph) {
            return true;
        }
    }

    return false;
}

bool core_ai_resource::does_locomotion_graph_exist(resource_key the_graph) const
{
    assert(the_graph.get_type() == RESOURCE_KEY_TYPE_AI_STATE_GRAPH);

    for (auto &curr : this->my_locomotion_graphs)
    {
        assert(curr->get_type() != RESOURCE_KEY_TYPE_AI_STATE_GRAPH);

        if (*curr == the_graph) {
            return true;
        }
    }

    return false;
}

} // namespace ai

void core_ai_resource_patch()
{
    {
        FUNC_ADDRESS(address, &ai::core_ai_resource::unmash);
        REDIRECT(0x00568A67, address);
    }
}
