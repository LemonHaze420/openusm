#include "wds_token_manager.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"
#include "osassert.h"
#include "resource_manager.h"
#include "script_manager.h"
#include "script.h"
#include "script_object.h"

#include "oldmath_po.h"

#include "terrain.h"
#include "token_def.h"
#include "token_def_list.h"
#include "trace.h"
#include "trigger_manager.h"
#include "utility.h"
#include "wds.h"

#include <cassert>
#include <cmath>

VALIDATE_SIZE(wds_token_manager, 0x24);

void wds_token_manager_region_change_callback(bool a1, region *a2)
{
    if constexpr (1) {
        assert(g_world_ptr != nullptr);

        if (a1) {
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

void wds_token_manager::mark_invisible_by_id(bool visible)
{
    field_14 = !visible;
    for (auto &token : field_18)
        token.field_4.get_volatile_ptr()->set_visible(visible, false);
}

void wds_token_manager::initialize(const resource_key &a2)
{
    TRACE("wds_token_manager::initialize");

    if constexpr (1) {
        assert(this->tokens == nullptr);
        assert(g_world_ptr != nullptr);

        this->field_8 = 0.0;
        auto *the_terrain = g_world_ptr->get_the_terrain();
#if STANDALONE_SYSTEM
        return;
#endif

        int size;
        auto *res = resource_manager::get_resource(a2, &size, nullptr);

        if (res != nullptr) {
            {
                if (mString{"CITY_ARENA.TOKENS"} == a2.get_platform_string(3)) {
                    assert(16948 == size);
                }
            }

#ifndef TARGET_XBOX
            mash_info_struct info_struct{res, size};
#else
            mash_info_struct info_struct{mash::UNMASH_MODE, res, size, true};
#endif

            info_struct.unmash_class(this->tokens,
                                     nullptr
#ifdef TARGET_XBOX
                                     ,
                                     mash::NORMAL_BUFFER
#endif
            );

            mash_info_struct::construct_class(this->tokens);

            this->field_C = (int)script_manager::get_game_var_address(mString{"gv_token_tally"}, nullptr, nullptr);

            this->token_collected_array =
                (int)script_manager::get_game_var_address(mString{"gv_token_collected"}, nullptr, nullptr);

            the_terrain->register_region_change_callback(wds_token_manager_region_change_callback);
            region *reg = nullptr;

            for (int i = 0; i < this->tokens->field_0.m_size; ++i) {
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

int wds_token_manager::get_token_index_from_id(int type, int id) const
{
    const auto count = tokens != nullptr ? tokens->field_0.m_size : 0;
    for (int index = 0; index < count; ++index) {
        const auto *definition = tokens->field_0.m_data[index];
        if (definition->type == type && definition->field_20 == id) {
            return index;
        }
    }
    return -1;
}

void wds_token_manager::frame_advance(Float elapsed)
{
    TRACE("wds_token_manager::frame_advance");

#if STANDALONE_SYSTEM
    constexpr float full_turn = 6.2831853071795864769f;
    this->field_8 = std::fmod(this->field_8 + elapsed.value * full_turn, full_turn);
    if (this->field_8 < 0.0f) {
        this->field_8 += full_turn;
    }

    if (this->field_18.empty()) {
        return;
    }

    for (auto &active : this->field_18) {
        auto *icon = active.field_4.get_volatile_ptr();
        auto *trigger_ptr = active.field_8.get_volatile_ptr();
        if (icon == nullptr || trigger_ptr == nullptr) {
            continue;
        }
        auto position = icon->get_abs_position();
        const vector3d facing{std::cos(this->field_8), 0.0f, std::sin(this->field_8)};
        const vector3d up{0.0f, 1.0f, 0.0f};
        po transform;
        transform.set_po(facing, up, position);
        entity_set_abs_po(icon, transform);
        if ((trigger_ptr->field_4 & 0x1000) == 0 || editing) {
            if (!active.field_C) {
                this->run_left_token_trigger();
                active.field_C = true;
            }
        } else {
            active.field_C = false;
        }
    }
#else
    THISCALL(0x00555B50, this, elapsed);
#endif
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

    for (auto it = this->field_18.begin(); it != this->field_18.end();) {
        if (it->field_0->field_28 == reg) {
            it = this->remove_active_token(it, true, true);
        } else {
            ++it;
        }
    }
}

_std::list<wds_token_manager::active_token>::iterator
wds_token_manager::remove_active_token(_std::list<wds_token_manager::active_token>::iterator a3, bool a4, bool a5)
{
    TRACE("wds_token_manager::remove_active_token");

    auto *trigger_mgr = trigger_manager::instance;
    assert(trigger_mgr != nullptr);

    auto &v10 = (*a3);
    v10.field_0->show_dot(false);

    auto *trig = v10.field_8.get_volatile_ptr();
    assert(trig != nullptr);

    if (((trig->field_4 & 0x1000) != 0) && a5 && !v10.field_C) {
        this->run_left_token_trigger();
    }

    trigger_mgr->delete_trigger(trig);
    if (a4) {
        auto *icon = v10.field_4.get_volatile_ptr();
        assert(icon != nullptr);

        g_world_ptr->ent_mgr.release_entity(icon);
    }

    auto Next = a3._Ptr->_Next;
    if (a3._Ptr != this->field_18.m_head) {
        a3._Ptr->_Prev->_Next = Next;
        a3._Ptr->_Next->_Prev = a3._Ptr->_Prev;
        operator delete(a3._Ptr);
        --this->field_18.m_size;
    }

    _std::list<wds_token_manager::active_token>::iterator result{Next};
    return result;
}

void wds_token_manager::run_left_token_trigger()
{
#if STANDALONE_SYSTEM
    auto *gso = script::get_gso();
    auto *gsoi = script::get_gsoi();
    if (gso == nullptr || gsoi == nullptr) {
        return;
    }
    const int function =
        script::find_function(string_hash{"left_token_trigger()"}, gso, false);
    if (function >= 0) {
        auto *thread = gso->add_thread(gsoi, function);
        if (thread != nullptr) {
            gsoi->run_single_thread(thread, true);
        }
    }
#else
    THISCALL(0x0054C0C0, this);
#endif
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
