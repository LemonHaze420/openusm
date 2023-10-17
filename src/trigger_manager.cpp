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

Var<trigger_manager *> trigger_manager::instance{0x0095FF98};

trigger_manager::trigger_manager() : m_triggers(nullptr) {}

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

trigger *trigger_manager::sub_51E5B0(entity_base *a2) {
    return (trigger *) THISCALL(0x0051E5B0, this, a2);
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

trigger *trigger_manager::new_point_trigger(vector3d a2, Float a5)
{
    TRACE("trigger_manager::new_point_trigger");

    auto *mem = mem_alloc(sizeof(point_trigger));

#if 0
    auto *t = new (mem) point_trigger{ANONYMOUS, a2, a5};
#else
    trigger * (__fastcall *func)(void *, void *, string_hash, const vector3d *, Float) = CAST(func, 0x0053C810);
    auto *t = func(mem, nullptr, ANONYMOUS, &a2, a5);
#endif

    this->add_trigger(t);
    return t;
}

trigger *trigger_manager::new_entity_trigger(entity_base *a2, Float a3) {
    auto *mem = mem_alloc(sizeof(entity_trigger));

    auto *result = new (mem) entity_trigger{ANONYMOUS, a2, a3};

    this->add_trigger(result);

    return result;
}

trigger *trigger_manager::new_box_trigger(string_hash a2, const vector3d &a3)
{
    TRACE("trigger_manager::new_box_trigger");

    auto *mem = mem_alloc(sizeof(box_trigger));
    auto *result = new (mem) box_trigger{a2, a3};

    this->add_trigger(result);
    return result;
}

trigger *trigger_manager::new_box_trigger(string_hash a2, entity_base *a3) {
    TRACE("trigger_manager::new_box_trigger");

    auto *mem = mem_alloc(sizeof(box_trigger));
    auto *result = new (mem) box_trigger{a2, a3};

    this->add_trigger(result);
    return result;
}

trigger *trigger_manager::new_point_trigger(
        string_hash a2,
        vector3d a3,
        Float a4)
{
    TRACE("trigger_manager::new_point_trigger");

    auto *mem = mem_alloc(sizeof(box_trigger));

#if 0
    auto *t= new (mem) point_trigger {a2, a3, a4};
#else
    trigger * (__fastcall *func)(void *, void *, string_hash, const vector3d *, Float) = CAST(func, 0x0053C810);
    auto *t = func(mem, nullptr, a2, &a3, a4);
#endif

    this->add_trigger(t);
    return t;
}

void trigger_manager_patch() {
    {
        FUNC_ADDRESS(address, &trigger_manager::delete_trigger);
        SET_JUMP(0x0051E560, address);
    }

    {
        trigger * (trigger_manager::*func)(vector3d, Float) = &trigger_manager::new_point_trigger;
        FUNC_ADDRESS(address, func);
        //SET_JUMP(0x00541B90, address);
    }

    {
        trigger * (trigger_manager::*func)(string_hash, vector3d, Float) = &trigger_manager::new_point_trigger;
        FUNC_ADDRESS(address, func);
        //SET_JUMP(0x00541C20, address);
    }

    {
        trigger * (trigger_manager::*func)(string_hash, entity_base *) = &trigger_manager::new_box_trigger;
        FUNC_ADDRESS(address, func);
        SET_JUMP(0x00541D70, address);
    }

    {
        trigger * (trigger_manager::*func)(string_hash, const vector3d &) = &trigger_manager::new_box_trigger;
        FUNC_ADDRESS(address, func);
        SET_JUMP(0x00541E00, address);
    }
}
