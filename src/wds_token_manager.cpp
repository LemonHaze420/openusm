#include "wds_token_manager.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"
#include "osassert.h"
#include "resource_manager.h"
#include "script_manager.h"
#include "terrain.h"
#include "token_def.h"
#include "token_def_list.h"
#include "trace.h"
#include "trigger_manager.h"
#include "utility.h"
#include "wds.h"

#include <cassert>

VALIDATE_SIZE(wds_token_manager, 0x24);

void wds_token_manager_region_change_callback(bool a1, region *a2)
{
    if constexpr (1)
    {
        assert(g_world_ptr != nullptr);
        
        if ( a1 ) {
            g_world_ptr->field_188.register_region(a2);
        } else {
            g_world_ptr->field_188.unregister_region(a2);
        }
    } else {
        CDECL_CALL(0x00558660, a1, a2);
    }
}

wds_token_manager::wds_token_manager() : tokens(nullptr)
{
    editing = false;
    field_4 = false;
    field_14 = false;
}

void wds_token_manager::initialize(const resource_key &a2)
{
    TRACE("wds_token_manager::initialize");

    if constexpr (1)
    {
        assert(this->tokens == nullptr);
        assert(g_world_ptr != nullptr);

        this->field_8 = 0.0;
        auto *the_terrain = g_world_ptr->get_the_terrain();

        int size;
        auto *res = resource_manager::get_resource(a2, &size, nullptr);

        if (res != nullptr) {

            {
                if (mString {"CITY_ARENA.TOKENS"} == a2.get_platform_string(3)) {
                    assert(16948 == size);
                }
            }

#ifndef TARGET_XBOX
            mash_info_struct info_struct{res, size};
#else
            mash_info_struct info_struct {mash::UNMASH_MODE, res, size, true};
#endif

            info_struct.unmash_class(this->tokens, nullptr
#ifdef TARGET_XBOX
            , mash::NORMAL_BUFFER
#endif 
                    );

            mash_info_struct::construct_class(this->tokens);

            this->field_C = (int) script_manager::get_game_var_address(mString{"gv_token_tally"},
                                                                       nullptr,
                                                                       nullptr);

            this->token_collected_array = (int)
                script_manager::get_game_var_address(mString{"gv_token_collected"},
                                                     nullptr,
                                                     nullptr);

            the_terrain->register_region_change_callback(wds_token_manager_region_change_callback);
            region *reg = nullptr;


            for (int i = 0; i < this->tokens->field_0.m_size; ++i)
            {
                assert(this->tokens->field_0.m_data != nullptr);

                auto *def = this->tokens->field_0.m_data[i];
                assert(def != nullptr);

                reg = the_terrain->find_region(def->field_10, reg);
                if (reg == nullptr) {
                    error("Token was placed outside of world < %f, %f, %f >",
                           def->field_10[0],
                           def->field_10[1],
                           def->field_10[2]);

                }

                def->field_28 = reg;
            }

            this->field_4 = 0;
        } else {
            auto str = a2.m_hash.to_string();

            warning("Could not find token resource %s", str);
        }

    } else {
        THISCALL(0x005586A0, this, &a2);
    }
}

void wds_token_manager::frame_advance(Float a1) {
    TRACE("wds_token_manager::frame_advance");

    THISCALL(0x00555B50, this, a1);
}

void wds_token_manager::register_region(region *reg)
{
    TRACE("wds_token_manager::register_region");

    THISCALL(0x00550A00, this, reg);
}

void wds_token_manager::unregister_region(region *reg)
{
    TRACE("wds_token_manager::unregister_region");

    assert(reg != nullptr);

    assert(tokens != nullptr);

    for ( auto it = this->field_18.begin(); it != this->field_18.end(); )
    {
        if ( it->field_0->field_28 == reg ) {
            it = this->remove_active_token(it, true, true);
        } else {
            ++it;
        }
    }
}

_std::list<wds_token_manager::active_token>::iterator wds_token_manager::remove_active_token(
        _std::list<wds_token_manager::active_token>::iterator a3,
        bool a4,
        bool a5)
{
    TRACE("wds_token_manager::remove_active_token");

    auto *trigger_mgr = trigger_manager::instance;
    assert(trigger_mgr != nullptr);

    auto &v10 = (*a3);
    v10.field_0->show_dot(false);

    auto *trig = v10.field_8.get_volatile_ptr();
    assert(trig != nullptr);

    if ( ((trig->field_4 & 0x1000) != 0) && a5 && !v10.field_C ) {
        this->sub_54C0C0();
    }

    trigger_mgr->delete_trigger(trig);
    if ( a4 )
    {
        auto *icon = v10.field_4.get_volatile_ptr();
        assert(icon != nullptr);

        g_world_ptr->ent_mgr.release_entity(icon);
    }

    auto Next = a3._Ptr->_Next;
    if ( a3._Ptr!= this->field_18.m_head )
    {
        a3._Ptr->_Prev->_Next = Next;
        a3._Ptr->_Next->_Prev = a3._Ptr->_Prev;
        operator delete(a3._Ptr);
        --this->field_18.m_size;
    }

    _std::list<wds_token_manager::active_token>::iterator result {Next}; 
    return result;
}

void wds_token_manager::sub_54C0C0()
{
    THISCALL(0x0054C0C0, this);
}

void wds_token_manager_patch()
{
    {
        FUNC_ADDRESS(address, &wds_token_manager::initialize);
        REDIRECT(0x0055B377, address);
    }

    {
        FUNC_ADDRESS(address, &token_def_list::unmash);
        REDIRECT(0x005587B3, address);
    }
}
