#include "script_instance_mashing.h"

#include "common.h"
#include "mission_manager.h"
#include "oldmath_po.h"
#include "parse_generic_mash.h"
#include "resource_manager.h"
#include "script_executable.h"
#include "script_executable_entry.h"
#include "script_manager.h"
#include "variables.h"

VALIDATE_SIZE(script_instance_info, 0x18);

void script_instance_info::un_mash(
        generic_mash_header *a2,
        void *a3,
        generic_mash_data_ptrs *a4)
{
    if constexpr (0)
    {}
    else
    {
        void (__fastcall *func)(void *, void *edx,
            generic_mash_header *,
            void *,
            generic_mash_data_ptrs *) = CAST(func, 0x00599780);
        func(this, nullptr, a2, a3, a4);
    }
}

bool script_instance_info::initialize_single(
        const script_executable *a2,
        string_hash a3,
        const po &a4)
{
    if constexpr (0)
    {
    }
    else
    {
        bool (__fastcall *func)(void *, void *edx,
            const script_executable *,
            string_hash ,
            const po *) = CAST(func, 0x005ABB00);
        return func(this, nullptr, a2, a3, &a4);
    }
}

bool script_instance_info::initialize(const script_executable *a2, const po &a3)
{
    if constexpr (0)
    {}
    else
    {
        bool (__fastcall *func)(void *, void *edx, const script_executable *a2, const po *a3) = CAST(func, 0x005AB990);
        return func(this, nullptr, a2, &a3);
    }
}

bool initialize_game_init_instances(const script_executable *se, string_hash a2)
{
    if constexpr (0)
    {
        if ( g_is_the_packer ) {
            return true;
        }

        auto *entry = script_manager::find_entry(se);
        assert(entry != nullptr);

        auto v14 = entry->field_C;
        if ( v14.is_set() )
        {
            auto a4 = mission_manager::s_inst->get_mission_key_po();
            auto *resource = bit_cast<generic_mash_header *>(resource_manager::get_resource(v14, nullptr, nullptr));
            if ( resource == nullptr )
            {
                auto *partition_pointer = resource_manager::get_partition_pointer((resource_partition_enum)6);
                auto pack_slots = partition_pointer->get_pack_slots();
                for ( int i = 0; pack_slots.size() && !resource; ++i )
                {
                    auto &slot = pack_slots[i];
                    if ( slot->is_pack_ready() )
                    {
                        auto __old_context = resource_manager::push_resource_context(slot);
                        resource = (generic_mash_header *)resource_manager::get_resource(v14, 0, 0);
                        resource_manager::pop_resource_context();

                        assert(resource_manager::get_resource_context() == __old_context);
                    }
                }
            }

            if ( resource == nullptr ) {
                return false;
            }

            script_instance_info *info = nullptr;
            if ( resource->field_4 < 0 )
            {
                info = bit_cast<script_instance_info *>(resource + 1);
            }
            else
            {
                bool allocated_mem = parse_generic_object_mash(info, resource, 0, 0, 0, 0, 0, 0);
                assert(!allocated_mem);
            }

            if ( a2 != string_hash {} ) {
                info->initialize_single(se, a2, a4);
            } else {
                info->initialize(se, a4);
            }

            return true;
        }
        else
        {
            return true;
        }
    }
    else
    {
        bool (*func)(const script_executable *se, string_hash a2) = CAST(func, 0x005ABCC0);
        return func(se, a2);
    }
}

void script_instance_mashing_patch()
{
    {
        //REDIRECT(0x, );
    }
}
