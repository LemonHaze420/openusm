#include "trigger_manager.h"

#include "box_trigger.h"
#include "common.h"
#include "entity_trigger.h"
#include "func_wrapper.h"
#include "memory.h"
#include "point_trigger.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

VALIDATE_SIZE(trigger_manager, 8u);

trigger_manager *& trigger_manager::instance = var<trigger_manager *>(0x0095FF98);

trigger_manager::trigger_manager() : m_triggers(nullptr) {
    this->m_vtbl = 0x00891274;
}

void trigger_manager::create_inst()
{
    TRACE("trigger_manager::create_inst");

    assert(instance == nullptr);

    instance = new trigger_manager {};
}

void trigger_manager::deinit() {
    purge();
}

void trigger_manager::purge() {
    TRACE("trigger_manager::purge");

    if constexpr (1) {
        while (this->m_triggers != nullptr) {
            this->remove(&m_triggers);
        }

    } else {
        THISCALL(0x0050CBA0, this);
    }
}

void trigger_manager::update() {
    THISCALL(0x00541F30, this);
}

void trigger_manager::update_trigger(trigger **a1, trigger_struct *a2, int a3)
{
    auto *tmp = *a1;
    assert(tmp != nullptr);

    if ( (tmp->is_box_trigger()
        && bit_cast<box_trigger *>(tmp)->field_58.get_volatile_ptr() != nullptr
        && bit_cast<box_trigger *>(tmp)->get_box_ent() == nullptr)
        || (tmp->is_entity_trigger()
        && bit_cast<entity_trigger *>(tmp)->get_ent() == nullptr) )
    {
        this->remove(a1);
    }
    else
    {
        tmp->update(a2, a3);
    }
}

trigger *trigger_manager::find_instance(entity_base *ent)
{
    TRACE("trigger_manager::find_instance");

    if constexpr (1)
    {
        if ( ent != nullptr )
        {
            for ( auto *t = this->m_triggers; t != nullptr; t = t->m_next_trigger )
            {
                if ( t->is_box_trigger() )
                {
                    if ( bit_cast<box_trigger *>(t)->get_box_ent() == ent) {
                        return t;
                    }
                }
                else if ( t->is_entity_trigger() )
                {
                    if ( bit_cast<entity_trigger *>(t)->get_ent() == ent) {
                        return t;
                    }
                }
            }
        }

        return nullptr;

    }
    else
    {
        return (trigger *) THISCALL(0x0051E5B0, this, ent);
    }
}

trigger * trigger_manager::find_instance(const mString &a2) const
{
    const string_hash v3 {a2.c_str()};
    for ( auto *trig = this->m_triggers; trig != nullptr; trig = trig->m_next_trigger )
    {
        if ( trig->get_id() == v3 ) {
            return trig;
        }
    }

    return nullptr;
}

void trigger_manager::remove(trigger **trem)
{
    TRACE("trigger_manager::remove");

    assert(trem != nullptr && (*trem) != nullptr);

    auto *v2 = *trem;
    *trem = (*trem)->m_next_trigger;
    if ( v2 != nullptr ) {
        void (_fastcall *finalize)(void *, void *, bool) = CAST(finalize, get_vfunc(v2->m_vtbl, 0x0));
        finalize(v2, nullptr, true);
    }
}

//TODO
void trigger_manager::delete_trigger(trigger *delete_me)
{
    TRACE("trigger_manager::delete_trigger");

    if constexpr (1) {
        assert(delete_me != nullptr);
        printf("0x%08X\n", delete_me->m_vtbl);

        trigger **t = nullptr;
        for (t = &this->m_triggers; *t != delete_me && *t != nullptr; t = &(*t)->m_next_trigger) {
            ;
        }

        assert(*t != nullptr && "trigger_manager error: request to delete an unknown trigger");

        this->remove(t);

    } else {
        THISCALL(0x0051E560, this, delete_me);
    }
}

void trigger_manager::add_trigger(trigger *a2) {
    TRACE("trigger_manager::add_trigger");

    a2->m_next_trigger = this->m_triggers;
    this->m_triggers = a2;
}

point_trigger *trigger_manager::new_point_trigger(vector3d a2, Float a5)
{
    TRACE("trigger_manager::new_point_trigger");

    auto *mem = mem_alloc(sizeof(point_trigger));
    auto *t = new (mem) point_trigger {ANONYMOUS, a2, a5};

    this->add_trigger(t);
    return t;
}

entity_trigger *trigger_manager::new_entity_trigger(entity_base *a2, Float a3)
{
    auto *mem = mem_alloc(sizeof(entity_trigger));
    auto *t = new (mem) entity_trigger {ANONYMOUS, a2, a3};

    this->add_trigger(t);

    return t;
}

box_trigger *trigger_manager::new_box_trigger(string_hash a2, const vector3d &a3)
{
    TRACE("trigger_manager::new_box_trigger");

    auto *mem = mem_alloc(sizeof(box_trigger));
    auto *t = new (mem) box_trigger {a2, a3};

    this->add_trigger(t);
    return t;
}

box_trigger *trigger_manager::new_box_trigger(string_hash a2, entity_base *a3)
{
    TRACE("trigger_manager::new_box_trigger");

    auto *mem = mem_alloc(sizeof(box_trigger));
    auto *t = new (mem) box_trigger{a2, a3};

    this->add_trigger(t);
    return t;
}

point_trigger *trigger_manager::new_point_trigger(
        string_hash a2,
        vector3d a3,
        Float a4)
{
    TRACE("trigger_manager::new_point_trigger");

    auto *mem = mem_alloc(sizeof(box_trigger));
    auto *t = new (mem) point_trigger {a2, a3, a4};

    this->add_trigger(t);
    return t;
}

void trigger_manager_patch()
{
    REDIRECT(0x005E1118, trigger_manager::create_inst);

    {
        FUNC_ADDRESS(address, &trigger_manager::delete_trigger);
        SET_JUMP(0x0051E560, address);
    }

    {
        point_trigger * (trigger_manager::*func)(vector3d, Float) = &trigger_manager::new_point_trigger;
        FUNC_ADDRESS(address, func);
        //SET_JUMP(0x00541B90, address);
    }

    {
        point_trigger * (trigger_manager::*func)(string_hash, vector3d, Float) = &trigger_manager::new_point_trigger;
        FUNC_ADDRESS(address, func);
        //SET_JUMP(0x00541C20, address);
    }

    {
        box_trigger * (trigger_manager::*func)(string_hash, entity_base *) = &trigger_manager::new_box_trigger;
        FUNC_ADDRESS(address, func);
        SET_JUMP(0x00541D70, address);
    }

    {
        box_trigger * (trigger_manager::*func)(string_hash, const vector3d &) = &trigger_manager::new_box_trigger;
        FUNC_ADDRESS(address, func);
        SET_JUMP(0x00541E00, address);
    }
}
