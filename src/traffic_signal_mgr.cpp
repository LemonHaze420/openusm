#include "traffic_signal_mgr.h"

#include "actor.h"
#include "conglom.h"
#include "entity.h"
#include "func_wrapper.h"
#include "vtbl.h"
#include <vector.hpp>

#if STANDALONE_SYSTEM
namespace {
struct signal_light {
    vhandle_type<entity> owner;
    int direction;
};

_std::vector<signal_light> &lights()
{
    return var<_std::vector<signal_light>>(0x0095CBF8);
}

void initialize_signal_state()
{
    static const bool initialized = [] {

        var<int>(0x00921D74) = 1;
        return true;
    }();
    (void)initialized;
}

void switch_ifl(entity *owner, int state)
{
    if (owner == nullptr || !owner->is_an_actor())
        return;
    static_cast<actor *>(owner)->ifl_lock(state);
    if ((owner->field_4 & 4) != 0) {
        auto *group = static_cast<conglomerate *>(owner);
        static const string_hash light_id{"LIGHT01"};
        static const string_hash pedestrian_id{"PEDLIGHT01"};
        for (auto *member : {group->get_member(light_id, true), group->get_member(pedestrian_id, true)}) {
            if (member != nullptr) {
                using lock = void (__fastcall *)(entity_base *, void *, int);
                reinterpret_cast<lock>(get_vfunc(member->m_vtbl, 0x268))(member, nullptr, state);
            }
        }
    }
}

void remove_first_expired_light()
{
    auto &entries = lights();
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        if (it->owner.get_volatile_ptr() == nullptr) {
            entries.erase(it);
            return;
        }
    }
}
}
#endif

traffic_signal_mgr::traffic_signal_mgr() {}

void traffic_signal_mgr::frame_advance(Float a1)
{
#if STANDALONE_SYSTEM
    initialize_signal_state();
#endif
    m_state_timer().field_0 = m_state_timer().field_0 - a1;
    if (m_state_timer().field_0 < 0.0)
        m_state_timer().field_0 = 0.0;
    if (m_state_timer().field_0 <= 0.0)
        switch_to_next_state();
}

void traffic_signal_mgr::switch_to_next_state()
{
#if STANDALONE_SYSTEM
    initialize_signal_state();
    auto &state = var<int>(0x00921D74);
    auto &direction = var<int>(0x0095C86C);
    if (state == 1) {
        state = 2;
        m_state_timer().field_0 = 30.0f;
        direction = direction == 0;
    } else if (state == 2) {
        state = 1;
        m_state_timer().field_0 = 4.0f;
    }
    bool expired = false;
    for (const auto &light : lights()) {
        auto *owner = light.owner.get_volatile_ptr();
        const bool aligned = (light.direction != 0 && light.direction != 2) == direction;
        int light_state = (aligned ? state == 2 : state == 0) ? 2 : 0;
        if (aligned ? state == 1 : state == 0)
            light_state = 1;
        if (owner != nullptr)
            switch_ifl(owner, light_state);
        else
            expired = true;
    }
    if (expired)
        remove_first_expired_light();
#else
    CDECL_CALL(0x005528D0);
#endif
}

void traffic_signal_mgr::add_traffic_light(entity *owner, bool direction)
{
#if STANDALONE_SYSTEM
    initialize_signal_state();
    if (!owner->is_a_conglomerate_clone()) {
        lights().push_back(signal_light{vhandle_type<entity>{owner->get_my_handle()}, direction});
        switch_ifl(owner, var<int>(0x00921D74));
    }
#else
    void (*func)(entity *, bool) = CAST(func, 0x0054E140);
    func(owner, direction);
#endif
}

void traffic_signal_mgr::remove_traffic_light(entity *owner)
{
#if STANDALONE_SYSTEM
    auto &entries = lights();
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        if (it->owner.get_volatile_ptr() == owner) {
            entries.erase(it);
            return;
        }
    }
#else
    CDECL_CALL(0x00544090, owner);
#endif
}
