#include "als_res_data.h"

#include "als_animation_logic_system.h"
#include "als_animation_logic_system_shared.h"
#include "common.h"
#include "conglom.h"
#include "mash.h"
#include "memory.h"
#include "resource_manager.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"

VALIDATE_SIZE(als_res_data, 0x10);

als_res_data::als_res_data()
{

}

void als_res_data::initialize(mash::allocation_scope a2)
{
    TRACE("als_res_data::initialize");

    if ( a2 == mash::FROM_MASH )
    {
        if (this->field_0.is_set())
        {
            uint8_t *resource = nullptr;
            if ( !g_is_the_packer ) {
                resource = resource_manager::get_resource(this->field_0, nullptr, nullptr);
            }

            auto *als_shared = bit_cast<als::animation_logic_system_shared *>(resource);
            if (als_shared != nullptr)
            {
                auto *mem = mem_alloc(sizeof(als::animation_logic_system));
                this->field_8 = new (mem) als::animation_logic_system{ global_transfer_variable_the_conglom()};
                this->field_8->create_instance_data(als_shared);
            }
            else
            {
                assert(0 && "I asked for an ALS resource, but I don't have one at runtime.");
                this->field_8 = nullptr;
            }
        }
        else
        {
            assert(0 && "Why is an ALS being created if nobody asked for one?");
            this->field_8 = nullptr;
        }

    }
}

void als_res_data::unmash(mash_info_struct *a2, void *)
{
    TRACE("als_res_data::unmash");

    this->field_0.unmash(a2, this);
}

void als_res_data::sub_4AB7F0(int a2)
{
    if ( a2 == 1 )
    {
        auto *v3 = this->field_8;
        if ( v3 != nullptr ) {
            v3->delete_instance_data();
        }

        auto *v4 = this->field_8;
        if ( v4 != nullptr )
        {
            this->field_8->~animation_logic_system();
            operator delete(v4);
        }

        this->field_8 = nullptr;
    }
}

void als_res_data::destruct_mashed_class()
{
    this->sub_4AB7F0(1u);
    this->field_0.destruct_mashed_class();
}

void als_res_data_patch()
{
    FUNC_ADDRESS(address, &als_res_data::initialize);
    SET_JUMP(0x004ABE40, address);
}
