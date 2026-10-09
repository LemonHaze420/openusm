#include "combat_state.h"

#include "advanced_entity_ptrs.h"
#include "aeps.h"
#include "ai_pedestrian.h"
#include "ai_std_combat_target.h"
#include "ai_universal_soldier_inode.h"
#include "als_inode.h"
#include "anim_event.h"
#include "base_ai_core.h"
#include "base_ai_state_machine.h"
#include "combo_system.h"
#include "common.h"
#include "conglom.h"
#include "controller_inode.h"
#include "core_ai_resource.h"
#include "cpu_controller_inode.h"
#include "damage_inode.h"
#include "damage_interface.h"
#include "event_manager.h"
#include "game.h"
#include "game_settings.h"
#include "combat_inode.h"
#include "collision_geometry.h"
#include "info_node_desc_list.h"
#include "loco_inode.h"
#include "mashed_state.h"
#include "native_pfx.h"
#include "oldmath_po.h"
#include "ped_spawner.h"
#include "pendulum.h"
#include "physical_interface.h"
#include "script.h"
#include "script_access.h"
#include "script_executable.h"
#include "script_object.h"
#include "sound_and_pfx_interface.h"
#include "state_machine.h"
#include "vm_thread.h"
#include "weapon_inode.h"
#include "handheld_item.h"
#include "wds.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>

VALIDATE_SIZE(cpu_combat_state, 0x138);
VALIDATE_SIZE(player_combat_state, 0x130);
VALIDATE_SIZE(spidey_combat_state, 0x130);
VALIDATE_SIZE(parker_combat_state, 0x130);
VALIDATE_OFFSET(combat_state, facing, 0x50);
VALIDATE_OFFSET(cpu_combat_state, allow_facing, 0x130);

namespace {
constexpr auto base_layer = static_cast<als::layer_types>(0);
const string_hash guide_speed{int(to_hash("guide_speed"))};
const string_hash force_stop_movement{int(to_hash("force_stop_movement"))};
const string_hash allow_facing_key{int(to_hash("allow_facing"))};
const string_hash recovery_time{int(to_hash("default_attack_recovery_time"))};
string_hash &combat_category()
{
    return var<string_hash>(0x009582FC);
}

template <class Result, class T, class... Args>
Result call(T *self, int offset, const Args &...args)
{
    using fn = Result(__fastcall *)(T *, void *, Args...);
    return reinterpret_cast<fn>(get_vfunc(self->m_vtbl, offset))(self, nullptr, args...);
}
template <class T>
T *node(const combat_state *self, string_hash id, bool required = true)
{
    return static_cast<T *>(self->get_core()->get_info_node(id, required));
}
bool weapon_move(int type)
{
    return (type >= 3 && type <= 12) || type == 16;
}
actor *target(int handle)
{
    return vhandle_type<actor>{handle}.get_volatile_ptr();
}
bool viable(actor *value)
{
    return value && (!value->has_damage_ifc() || value->damage_ifc()->is_alive());
}
bool core_allows_facing(ai::ai_core *core)
{
    auto *loco = core->field_40;
    return loco && call<const resource_key *>(loco, 0x34)->is_set() && loco->allow_facing_change;
}
template <class T>
void clear_vector(mVectorBasic<T> &values)
{
    if (!values.is_pointer_in_mash_image(values.m_data))
        ::operator delete[](values.m_data);
    values.m_data = nullptr;
    values.m_max_size = 0;
    values.mContainer_base::clear();
    values.mContainer_base::destruct_mashed_class();
}
template <class T>
void read_vector(mVectorBasic<T> &values, mash_info_struct *info, void *)
{
    if (values.m_data)
        values.m_data = reinterpret_cast<T *>(info->read_from_buffer(sizeof(T) * values.m_size, 4));
    values.field_0 =
        reinterpret_cast<int>(info->mash_image_ptr[0] + info->buffer_size_used[0]) - reinterpret_cast<int>(&values);
}
template <class T>
void append(mVectorBasic<T> &values, const T &value)
{
    if (values.m_size == values.m_max_size || values.is_pointer_in_mash_image(values.m_data)) {
        const int capacity = 8 * (values.m_size / 8) + 8;
        auto *data = static_cast<T *>(::operator new(sizeof(T) * capacity));
        if (values.m_size)
            std::uninitialized_copy_n(values.m_data, values.m_size, data);
        if (!values.is_pointer_in_mash_image(values.m_data))
            ::operator delete[](values.m_data);
        values.m_data = data;
        values.m_max_size = capacity;
    }
    new (values.m_data + values.m_size++) T(value);
}
bool contains(const mVectorBasic<entity_base_vhandle> &values, entity_base_vhandle handle)
{
    for (int i = 0; i < values.m_size; ++i)
        if (values.m_data[i] == handle)
            return true;
    return false;
}
entity_base *event_bone(event *evt, entity_base_vhandle handle)
{
    auto *owner = static_cast<conglomerate *>(handle.get_volatile_ptr());
    return owner->get_bone(static_cast<anim_event *>(evt)->field_C, true);
}
void contact_effect(entity_base *owner, entity_base *other, const vector3d &position, bool blocked = false)
{
    aeps::ActionInfoStruct info;
    info.owner = owner;
    info.other = other;
    info.hash = blocked ? 2 : 1;
    info.position = &position;
    auto *ifc = owner->my_sound_and_pfx_interface;
    if (ifc && ifc->field_28) {
        using fn = void(__fastcall *)(void *, void *, int, sound_and_pfx_interface *, aeps::ActionInfoStruct *);
        auto *graph = ifc->field_28;
        reinterpret_cast<fn>(get_vfunc(*static_cast<int *>(graph), 0x34))(graph, nullptr, 5, ifc, &info);
    } else if (auto *effect = var<native_pfx::Instance *>(0x0095A5E8)) {
        native_pfx::set_position(effect, position);
        native_pfx::spawn_action(effect, 0, info.delay, info.lifetime, 1.0f);
    }
}
void raise_impact(combat_state *self, entity_base_vhandle recipient)
{
    attack_impact_sound_event evt;
    evt.sound = self->field_A0.sound;
    evt.volume = self->field_A0.volume;
    evt.source = recipient;
    event_manager::raise_event(&evt, self->get_actor()->my_handle);
}
void __cdecl attack_event(event *evt, entity_base_vhandle handle, void *context)
{
    auto *self = static_cast<combat_state *>(context);
    if (call<bool>(self->field_30, 0xA4) && self->field_30->get_cur_move()->field_4.field_2C == 7) {
        if (auto *victim = target(self->field_B8.field_0)) {
            if (auto *constraint = victim->physical_ifc()->get_pendulum(4)) {
                constraint->field_0 = 0;
                constraint->field_4 = self->field_68;
                constraint->field_10 = self->field_68;
            }
        }
    }
    if (!call<bool>(self, 0x64))
        self->field_FA = true;
    if (!self->field_F9) {
        self->field_FC = true;
        if (auto *bone = event_bone(evt, handle))
            self->field_C0 = static_cast<entity *>(bone);
    }
}
void __cdecl attack_begin(event *evt, entity_base_vhandle handle, void *context)
{
    auto *self = static_cast<combat_state *>(context);
    if (auto *bone = event_bone(evt, handle))
        append(self->field_C4, combat_bone_cache{static_cast<entity *>(bone), bone->get_abs_position()});
}
void __cdecl attack_end(event *evt, entity_base_vhandle handle, void *context)
{
    auto &cache = static_cast<combat_state *>(context)->field_C4;
    auto *bone = event_bone(evt, handle);
    for (int i = 0; i < cache.m_size; ++i) {
        if (cache.m_data[i].bone == bone) {
            const int remaining = cache.m_size - i - 1;
            if (remaining)
                std::copy_n(cache.m_data + i + 1, remaining, cache.m_data + i);
            --cache.m_size;
            break;
        }
    }
}
void __cdecl impact_sound(event *evt, entity_base_vhandle handle, void *context)
{
    auto *self = static_cast<combat_state *>(context);
    auto *anim = static_cast<anim_event *>(evt);
    const auto level = anim->field_10[1];
    self->field_A0.sound = anim->field_10[0];
    self->field_A0.volume = level == string_hash{int(to_hash("LOW"))}      ? bit_cast<float>(0x3EAAAAAB)
                            : level == string_hash{int(to_hash("MEDIUM"))} ? bit_cast<float>(0x3F2AAAAB)
                                                                           : 1.0f;
    self->field_A0.source = handle;
}
void __cdecl fx_event(event *evt, entity_base_vhandle handle, void *context)
{
    auto *self = static_cast<combat_state *>(context);
    self->field_FC = true;
    if (auto *bone = event_bone(evt, handle))
        self->field_C0 = static_cast<entity *>(bone);
}
void __cdecl web_end(event *evt, entity_base_vhandle handle, void *context)
{
    auto *self = static_cast<combat_state *>(context);
    if (auto *bone = event_bone(evt, handle))
        call<void>(self->field_30, 0x64, bone);
}
void __cdecl grab_start(event *evt, entity_base_vhandle handle, void *context)
{
    auto *owner = handle.get_volatile_ptr();
    if (owner && owner->is_a_conglomerate()) {
        if (auto *bone = event_bone(evt, handle))
            static_cast<combat_state *>(context)->field_BC = static_cast<entity *>(bone);
    }
}
void __cdecl grab_end(event *, entity_base_vhandle, void *context)
{
    static_cast<combat_state *>(context)->field_BC = nullptr;
}
void __cdecl motion_start(event *evt, entity_base_vhandle handle, void *)
{
    auto *owner = handle.get_volatile_ptr();
    auto *bone = event_bone(evt, handle);
    auto *ifc = owner->my_sound_and_pfx_interface;
    if (!bone || !ifc || !ifc->field_28)
        return;
    auto *graph = ifc->field_28;
    using predicate = bool(__fastcall *)(void *, void *, int);
    if (!reinterpret_cast<predicate>(get_vfunc(*static_cast<int *>(graph), 0x40))(graph, nullptr, 5))
        return;
    aeps::ActionInfoStruct info;
    info.owner = owner;
    info.other = bone;
    info.hash = event::MO_START.source_hash_code;
    using fn = void(__fastcall *)(void *, void *, int, sound_and_pfx_interface *, aeps::ActionInfoStruct *);
    reinterpret_cast<fn>(get_vfunc(*static_cast<int *>(graph), 0x34))(graph, nullptr, 5, ifc, &info);
}
const std::array<string_hash, 12> callback_events{event::ANIM_ACTION,
                                                  event::ATTACK,
                                                  event::ATTACK_BEGIN,
                                                  event::ATTACK_END,
                                                  event::ATTACK_IMPACT_SOUND,
                                                  event::FX_SMALL,
                                                  event::FX_BIG,
                                                  event::WEB_START,
                                                  event::WEB_END,
                                                  event::GRAB_START,
                                                  event::GRAB_END,
                                                  event::MO_START};
const std::array<void (*)(event *, entity_base_vhandle, void *), 12> callbacks{attack_event,
                                                                               attack_event,
                                                                               attack_begin,
                                                                               attack_end,
                                                                               impact_sound,
                                                                               fx_event,
                                                                               fx_event,
                                                                               web_start_call_back,
                                                                               web_end,
                                                                               grab_start,
                                                                               grab_end,
                                                                               motion_start};
template <class T, unsigned Type>
unsigned __fastcall type(const T *)
{
    return Type;
}
template <class T>
int __fastcall size(const T *)
{
    return sizeof(T);
}
template <class T, unsigned Type>
bool __fastcall subclass(const T *, void *, unsigned value)
{
    if constexpr (Type == 275 || Type == 276 || Type == 278)
        return (Type == 275 && value == 278) || (Type != 276 && value == 276) || value == 560 || value == 535 ||
               value == 567 || value == 573;
    return value == Type || value == 560 || value == 535 || value == 567 || value == 573;
}
void __fastcall destruct(combat_state *self, void *)
{
    self->_destruct_mashed_class();
}
void __fastcall unmash(combat_state *self, void *, mash_info_struct *info, void *owner)
{
    self->_unmash(info, owner);
}
template <class T>
void *__fastcall destroy(T *self, void *, unsigned flags)
{
    self->~T();
    if (flags & 1)
        ::operator delete(self);
    return self;
}
template <class T>
void __fastcall activate(T *self, void *, ai::ai_state_machine *machine, const ai::mashed_state *state,
                         const ai::mashed_state *previous, const ai::param_block *params,
                         ai::base_state::activate_flag_e flags)
{
    self->_activate(machine, state, previous, params, flags);
}
template <class T>
void __fastcall deactivate(T *self, void *, const ai::mashed_state *next)
{
    self->_deactivate(next);
}
template <class T>
ai::state_trans_messages __fastcall frame(T *self, void *, Float delta)
{
    return self->_frame_advance(delta);
}
void __fastcall info(combat_state *, void *, ai::info_node_desc_list &list)
{
    list.add_entry({ai::combat_inode::default_id, 342});
    list.add_entry({ai::combat_target_inode::default_id, 351});
    list.add_entry({ai::controller_inode::default_id, 357});
    list.add_entry({ai::als_inode::default_id, 333});
    list.add_entry({ai::damage_inode::default_id, 343});
    list.add_entry({string_hash{int(to_hash("instant_kill"))}, 345});
}
template <unsigned Type>
void __fastcall player_info(combat_state *, void *, ai::info_node_desc_list &list)
{
    list.add_entry({ai::combat_inode::default_id, 342});
    list.add_entry({ai::combat_target_inode::default_id, 353});
    list.add_entry({ai::controller_inode::default_id, 358});
    list.add_entry({ai::als_inode::default_id, 333});
    list.add_entry({ai::damage_inode::default_id, 343});
    list.add_entry({string_hash{"track_field"}, 420});
    if constexpr (Type == 275 || Type == 278)
        list.add_entry({ai::combat_inode::default_id, 248});
    if constexpr (Type == 275)
        list.add_entry({ai::als_inode::default_id, 334});
}
void __fastcall movement(combat_state *self, void *, Float delta, const combo_system_move *move, bool update,
                         bool start)
{
    self->advance_movement(delta, move, update, start);
}
bool __fastcall area(combat_state *self, void *, Float delta)
{
    return self->advance_area_damage(delta);
}
bool __fastcall damage_ped(combat_state *self, void *, const vector3d *points, int count, actor *owner,
                           const vector3d &position, const combo_system_move::results &results, entity *victim,
                           damage_interface *damage)
{
    return self->advance_area_damage_peds(points, count, owner, position, results, victim, damage);
}
bool __fastcall communication(combat_state *self, void *, Float delta, const combo_system_move *move)
{
    return self->advance_communications(delta, move);
}
ai::state_trans_messages __fastcall exit_check(combat_state *self, void *, Float delta)
{
    return self->check_exit(delta);
}
bool __fastcall web_spot(combat_state *self, void *)
{
    return self->find_web_hang_spot();
}
void __fastcall clean(combat_state *self, void *)
{
    self->clean_for_exit();
}
void __fastcall setup(combat_state *self, void *)
{
    self->call_backs_setup();
}
void __fastcall cleanup(combat_state *self, void *)
{
    self->call_backs_cleanup();
}
bool __fastcall sane(const combat_state *self, void *)
{
    return self->is_sane_range();
}
bool __fastcall stays(const cpu_combat_state *self, void *, const ai::mashed_state *next)
{
    return self->stays_in_combat(next);
}
ai::state_trans_action *__fastcall process(const cpu_combat_state *self, void *, ai::state_trans_action *out,
                                           Float delta, ai::state_trans_messages message)
{
    *out = self->_process_message(delta, message);
    return out;
}
template <class T, unsigned Type>
auto make_table()
{
    std::array<void *, Type == 270 ? 27 : 26> table{};
    std::copy_n(static_cast<void **>(ai::enhanced_state::native_vtable()), 16, table.begin());
    table[0] = reinterpret_cast<void *>(&destruct);
    table[1] = reinterpret_cast<void *>(&unmash);
    table[2] = reinterpret_cast<void *>(&destroy<T>);
    table[3] = reinterpret_cast<void *>(&type<T, Type>);
    table[4] = reinterpret_cast<void *>(&subclass<T, Type>);
    table[6] = reinterpret_cast<void *>(&activate<T>);
    table[7] = reinterpret_cast<void *>(&deactivate<T>);
    table[8] = reinterpret_cast<void *>(&frame<T>);
    if constexpr (Type == 270)
        table[9] = reinterpret_cast<void *>(&info);
    if constexpr (Type == 275 || Type == 276 || Type == 278)
        table[9] = reinterpret_cast<void *>(&player_info<Type>);
    table[13] = reinterpret_cast<void *>(&size<T>);
    table[16] = reinterpret_cast<void *>(&movement);
    table[17] = reinterpret_cast<void *>(&area);
    table[18] = reinterpret_cast<void *>(&damage_ped);
    table[19] = reinterpret_cast<void *>(&communication);
    table[20] = reinterpret_cast<void *>(&exit_check);
    table[21] = reinterpret_cast<void *>(&web_spot);
    table[22] = reinterpret_cast<void *>(&clean);
    table[23] = reinterpret_cast<void *>(&setup);
    table[24] = reinterpret_cast<void *>(&cleanup);
    table[25] = reinterpret_cast<void *>(&sane);
    if constexpr (Type == 270) {
        table[12] = reinterpret_cast<void *>(&process);
        table[26] = reinterpret_cast<void *>(&stays);
    }
    return table;
}
}

void *combat_state::native_vtable()
{
    static auto table = make_table<combat_state, 560>();
    return table.data();
}
void *cpu_combat_state::native_vtable()
{
    static auto table = make_table<cpu_combat_state, 270>();
    return table.data();
}
void *player_combat_state::native_vtable()
{
    static auto table = make_table<player_combat_state, 276>();
    return table.data();
}
void *spidey_combat_state::native_vtable()
{
    static auto table = make_table<spidey_combat_state, 278>();
    return table.data();
}
void *parker_combat_state::native_vtable()
{
    static auto table = make_table<parker_combat_state, 275>();
    return table.data();
}
player_combat_state::player_combat_state() : combat_state()
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
}
player_combat_state::player_combat_state(from_mash_in_place_constructor *tag) : combat_state(tag)
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
}
spidey_combat_state::spidey_combat_state() : player_combat_state()
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
}
spidey_combat_state::spidey_combat_state(from_mash_in_place_constructor *tag) : player_combat_state(tag)
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
}
parker_combat_state::parker_combat_state() : spidey_combat_state()
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
}
parker_combat_state::parker_combat_state(from_mash_in_place_constructor *tag) : spidey_combat_state(tag)
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
}
cpu_combat_state::cpu_combat_state() : combat_state(), allow_facing{-1}, previous_allow_facing{}, padding_135{}
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
}
cpu_combat_state::cpu_combat_state(from_mash_in_place_constructor *tag) : combat_state(tag)
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
}
combat_state::~combat_state()
{
    if (get_actor() && get_actor()->has_physical_ifc() && get_machine())
        clean_for_exit();
    clear_vector(field_E4);
    clear_vector(field_D4);
    clear_vector(field_C4);
}
void combat_state::_destruct_mashed_class()
{
    call<void>(&field_A0, 0);
    clear_vector(field_C4);
    clear_vector(field_D4);
    clear_vector(field_E4);
    using fn = void(__fastcall *)(ai::enhanced_state *, void *);
    reinterpret_cast<fn>(static_cast<void **>(ai::enhanced_state::native_vtable())[0])(this, nullptr);
}
void combat_state::_unmash(mash_info_struct *mash, void *owner)
{
    using fn = void(__fastcall *)(ai::enhanced_state *, void *, mash_info_struct *, void *);
    reinterpret_cast<fn>(static_cast<void **>(ai::enhanced_state::native_vtable())[1])(this, nullptr, mash, owner);
    mash_virtual_base::fixup_vtable(reinterpret_cast<mash_virtual_base *>(&field_A0));
    call<void>(&field_A0, 4, mash, this);
    read_vector(field_C4, mash, this);
    read_vector(field_D4, mash, this);
    read_vector(field_E4, mash, this);
}
void combat_state::call_backs_setup()
{
    const auto owner = get_actor()->my_handle;
    for (unsigned i = 0; i < callbacks.size(); ++i)
        callback_ids[i] = event_manager::add_callback(callback_events[i], owner, callbacks[i], this, false);
}
void combat_state::call_backs_cleanup()
{
    const auto owner = get_actor()->my_handle;
    for (unsigned i = 0; i < callback_events.size(); ++i)
        event_manager::remove_callback(callback_ids[i], callback_events[i], owner);
}
void combat_state::clean_for_exit()
{
    if (field_90) {
        script::new_thread(deactivate_function, field_90);
        script::exec_thread(true);
        field_8C->remove_instance(field_90);
    }
    call<void>(this, 0x60);
    get_actor()->remove_collision_ignorance(field_B8);
    field_34->field_24 = -1.0f;
    call<void>(field_30, 0xAC);
}
void combat_state::_activate(ai::ai_state_machine *machine, const ai::mashed_state *state,
                             const ai::mashed_state *previous, const ai::param_block *params, activate_flag_e flags)
{
    ++var<int>(0x009682EC);
    ai::enhanced_state::activate(machine, state, previous, params, flags);
    call<void>(this, 0x5C);
    auto *owner = get_actor();
    field_FC = field_FD = field_F9 = field_FE = false;
    field_C0 = nullptr;
    field_B8 = entity_base_vhandle{0};
    field_A0.source = entity_base_vhandle{0};
    field_BC = nullptr;
    field_38 = node<ai::controller_inode>(this, ai::controller_inode::default_id)
                   ->get_axis(static_cast<ai::controller_inode::eControllerAxis>(2));
    const auto &pose = owner->get_abs_po();
    field_44 = pose.get_y_facing();
    facing = field_38;
    field_68 = owner->get_abs_position();
    movement_distance = -1.0f;
    field_F8 = field_FA = false;
    field_30 = node<ai::combat_inode>(this, ai::combat_inode::default_id);
    field_34 = node<ai::als_inode>(this, ai::als_inode::default_id);
    const float default_speed = 3.0f;
    field_84 = field_30->my_param_block.get_optional_pb_float(guide_speed, default_speed, nullptr);
    combat_category() = field_34->get_category_id(base_layer);
    field_90 = nullptr;
    if (call<bool>(field_30, 0xB4)) {
        const auto *next = field_30->get_next_move();
        if (next->field_80.field_10.field_4 != 6 && next->field_80.field_10.field_4 != 4 &&
            !viable(target(field_30->field_20)))
            call<void>(field_30, 0xBC);
    }
    auto *source = owner->adv_ptrs ? owner->adv_ptrs->my_script : nullptr;
    if (source && call<bool>(field_30, 0xB4)) {
        field_8C = source->parent->parent->find_object(string_hash{field_30->get_next_move()->field_4.field_10.c_str()},
                                                       nullptr);
        if (field_8C) {
            field_90 = script::create_instance(string_hash{int(to_hash("__auto_cmbt_move"))}, field_8C);
            script::push_arg(owner);
            script::exec_thread(true);
            if (field_90) {
                frame_function =
                    script::find_function(string_hash{int(to_hash("frame_advance(num,num)"))}, field_8C, true);
                activate_function = script::find_function(string_hash{int(to_hash("activate()"))}, field_8C, true);
                deactivate_function = script::find_function(string_hash{int(to_hash("deactivate()"))}, field_8C, true);
            }
        }
    }
    if (field_90) {
        script::new_thread(activate_function, field_90);
        script::exec_thread(true);
    }
    if (call<bool>(field_30, 0xB4)) {
        const auto *next = field_30->get_next_move();
        const int movement_type = next->field_4.field_2C;
        if (movement_type == 6) {
            field_68 = *call<const vector3d *>(field_30, 0x90);
            field_38 = *call<const vector3d *>(field_30, 0x94);
        }
        if (movement_type == 6 && !call<bool>(field_30, 0x98)) {
            call<void>(field_30, 0xBC);
        } else if (movement_type == 7 && target(field_30->field_20)) {
            if (!call<bool>(this, 0x54)) {
                call<void>(field_30, 0xBC);
            } else if (auto *constraint = ai::combat_pendulum_manager::acquire_free_pendulum()) {
                constraint->sub_48AFB0(owner);
                auto *victim = target(field_30->field_20);
                constraint->m_constraint = (owner->get_abs_position() - victim->get_abs_position()).length();
                victim->physical_ifc()->set_pendulum(4, constraint);
                constraint->set_attach_limb(9);
            }
        }
    }
    if (call<bool>(field_30, 0xB4)) {
        call<void>(field_30, 0xC8);
        const auto *move = field_30->get_cur_move();
        if (move->field_80.field_10.field_4 != 4 && move->field_80.field_10.field_4 != 5)
            field_B8 = entity_base_vhandle{static_cast<uint32_t>(field_30->field_1C)};
        call<void>(this, 0x40, Float{0.0f}, move, true, true);
        if (owner == g_world_ptr->get_hero_ptr(0))
            g_game_ptr->gamefile->update_web_fluid_used(Float{0.1f});
    }
}
void combat_state::_deactivate(const ai::mashed_state *next)
{
    if (call<bool>(field_30, 0xA4)) {
        const auto *move = field_30->get_cur_move();
        if (weapon_move(move->field_4.field_28) &&
            (!call<bool>(field_30, 0xB4) || !weapon_move(field_30->get_next_move()->field_4.field_28)))
            field_30->disable_weapon_based_effect(move);
    }
    --var<int>(0x009682EC);
    ai::base_state::_deactivate(next);
}
void cpu_combat_state::_activate(ai::ai_state_machine *machine, const ai::mashed_state *state,
                                 const ai::mashed_state *previous, const ai::param_block *params, activate_flag_e flags)
{
    auto *core = machine->my_core;
    auto *combat = static_cast<ai::combat_inode *>(core->get_info_node(ai::combat_inode::default_id, true));
    if (call<int>(combat, 0x50) == 1) {
        auto *controller =
            static_cast<ai::cpu_controller_inode *>(core->get_info_node(ai::controller_inode::default_id, true));
        auto *system = core->field_6C->field_10;
        controller->combo_chain_index = -1;
        for (int i = 0; i < system->field_14.m_size; ++i) {
            if (system->field_14.m_data[i]->field_14 == string_hash{combat->field_70}) {
                controller->combo_chain_index = i;
                break;
            }
        }
        controller->next_chain_index = 0;
        controller->field_104 = false;
        controller->pending_trigger = 0;
        controller->level_time = g_world_ptr->time_manager.get_level_time();
        controller->engage_current_move();
        if (!call<bool>(combat, 0xB4))
            call<bool>(combat, 0x9C);
        ai::universal_soldier_inode::redeem_attack_token(static_cast<ai::universal_soldier_inode *>(
            core->get_info_node(ai::universal_soldier_inode::default_id, false)));
    }
    combat_state::_activate(machine, state, previous, params, flags);
    const int no_stop = 0;
    if (my_mashed_state->field_0.get_optional_pb_int(force_stop_movement, no_stop, nullptr))
        core->stop_movement();
    previous_allow_facing = core_allows_facing(core);
    const int unchanged = -1;
    bool found;
    allow_facing = my_mashed_state->field_0.get_optional_pb_int(allow_facing_key, unchanged, &found);
    if (found)
        core->set_allow_facing(allow_facing != 0);
}
bool cpu_combat_state::stays_in_combat(const ai::mashed_state *next) const
{
    return next && next->field_14 == 270;
}
void cpu_combat_state::_deactivate(const ai::mashed_state *next)
{
    if (call<bool>(this, 0x68, next)) {
        combat_state::_deactivate(next);
        return;
    }
    auto *combat = node<ai::combat_inode>(this, ai::combat_inode::default_id);
    call<void>(combat, 0xB0);
    combat_state::_deactivate(next);
    if (combat->field_74) {
        auto *chain = reinterpret_cast<const combo_system_chain *>(combat->field_74);
        combat->field_6C = chain->field_40;
        if (combat->field_6C < 0.0f) {
            const float zero = 0.0f;
            combat->field_6C = get_core()->field_50.get_optional_pb_float(recovery_time, zero, nullptr);
        }
    }
    combat->field_1C = combat->field_74 = combat->field_70 = 0;
    if (call<int>(combat, 0x50) != 3) {
        call<void>(combat, 0xBC);
        node<ai::cpu_controller_inode>(this, ai::controller_inode::default_id)->stop_chain();
    }
    ai::universal_soldier_inode::release_attack_token(
        node<ai::universal_soldier_inode>(this, ai::universal_soldier_inode::default_id, false));
    get_core()->set_allow_facing(previous_allow_facing);
}
ai::state_trans_messages cpu_combat_state::_frame_advance(Float delta)
{
    if (allow_facing >= 0)
        get_core()->set_allow_facing(allow_facing > 0);
    return combat_state::_frame_advance(delta);
}
ai::state_trans_action cpu_combat_state::_process_message(Float delta, ai::state_trans_messages message) const
{
    const auto value = static_cast<int>(message);
    ai::state_trans_action result = get_default_return_code();
    if (value != 1 && value != 2)
        return result;
    result = {static_cast<ai::state_trans_actions>(0), get_name(), static_cast<ai::state_trans_messages>(75), nullptr};
    if (value == 1) {
        auto *combat = node<ai::combat_inode>(this, ai::combat_inode::default_id);
        if (call<bool>(combat, 0xB4))
            return result;
        const auto *move = combat->get_cur_move();
        if (weapon_move(move->field_4.field_28))
            combat->disable_weapon_based_effect(move);
    } else {
        node<ai::cpu_controller_inode>(this, ai::controller_inode::default_id)->stop_chain();
    }
    return ai::enhanced_state::process_message(delta, message);
}

void web_start_call_back(event *evt, entity_base_vhandle handle, void *context)
{
    auto *self = static_cast<combat_state *>(context);
    auto *bone = event_bone(evt, handle);
    if (!bone)
        return;
    auto *victim = target(self->field_B8.field_0);
    const auto position =
        victim ? victim->get_abs_position() : self->get_actor()->get_abs_position() + self->field_38 * 10.0f;
    call<void>(self->field_30, 0x60, static_cast<entity *>(bone), position.x, position.y, position.z);
}

void combat_state::advance_movement(Float delta, const combo_system_move *move, bool, bool start)
{
    auto *layer = field_34->get_als_layer(base_layer);
    const float action_time = layer->get_time_to_signal(event::ANIM_ACTION);
    if (!field_FE && !start) {
        if (layer->get_time_to_signal(event::FX_SMALL) >= 0.0f)
            field_F9 = true;
        field_FE = true;
    }
    const auto category = field_34->get_category_id(base_layer);
    if (category != combat_category()) {
        combat_category() = category;
        field_44 = get_actor()->get_abs_po().get_y_facing();
    }
    auto direction = field_38;
    const auto up = field_44;
    auto guided = facing;
    auto destination = field_68;
    const auto &position = get_actor()->get_abs_position();
    const auto &results = move->field_4;
    const int movement_type = results.field_2C;
    const float distance = bit_cast<float>(results.field_30);
    const float height = bit_cast<float>(results.field_34);
    const float tolerance = results.field_78;
    auto *victim = target(field_30->field_1C);
    auto target_position = position + direction * distance;
    if (victim)
        target_position = victim->get_abs_position();
    field_88 = true;
    if (victim && movement_type > 3 && movement_type < 8 && !(movement_type == 6 && action_time > 0.0f)) {
        const auto offset = position - target_position;
        field_88 = std::fabs(std::sqrt(offset.x * offset.x + offset.z * offset.z) - distance) < tolerance &&
                   std::fabs(offset.y - height) < tolerance;
    }
    const auto displacement = target_position - position;
    vector3d horizontal{displacement.x, 0.0f, displacement.z};
    if (horizontal.length2() > 1.0e-6f)
        horizontal.normalize();
    else if (victim)
        horizontal = victim->get_abs_po().get_z_facing();
    vector3d vertical{0.0f, displacement.y, 0.0f};
    if (vertical.length2() > 1.0e-6f)
        vertical.normalize();
    if (movement_type == 9) {
        auto *controller = node<ai::controller_inode>(this, ai::controller_inode::default_id);
        guided = (direction + controller->get_axis(static_cast<ai::controller_inode::eControllerAxis>(2)) *
                                  (field_84 * delta.value))
                     .normalized();
    }
    bool update = movement_type == 2 || movement_type == 3 || movement_type == 8 || movement_type == 9;
    if (movement_type == 4 || movement_type == 5 || (movement_type == 6 && action_time <= 0.0f && !start)) {
        destination = target_position - horizontal * distance - vertical * height;
        if (movement_distance >= 0.0f)
            movement_distance += (destination - field_68).length();
        else
            movement_distance = 0.0001f;
        update = true;
    }
    if (update) {
        if (movement_type != 2)
            direction = guided;
        if ((victim && !field_FA) || movement_type == 9) {
            field_38 = direction;
            field_44 = up;
            facing = displacement.normalized();
            field_68 = destination;
        }
    }
    destination = field_68;
    if (movement_type == 4)
        destination.y = position.y;
    als::param_list params;
    if (movement_type != 1) {
        params.add_param(27, field_38);
        params.add_param(24, field_44);
        params.add_param(58, facing);
        params.add_param(55, facing);
        params.add_param(33, destination);
        if (field_FA)
            params.add_param(als::param{54, 1.0f});
        field_34->set_desired_params(params, base_layer);
    }
    if (start)
        field_34->request_category_transition(results.field_4, base_layer, true, false, true);
    params.clear();
}

ai::state_trans_messages combat_state::check_exit(Float delta)
{
    if (!call<bool>(field_30, 0xC4))
        return static_cast<ai::state_trans_messages>(1);
    const auto *move = field_30->get_cur_move();
    const float chain_time = field_34->get_als_layer(base_layer)->get_time_to_signal(event::CMBT_CHAIN);
    if (move->field_4.field_24 == 16) {
        if (field_1C > bit_cast<float>(move->field_4.field_20)) {
            if (weapon_move(move->field_4.field_28))
                field_30->disable_weapon_based_effect(move);
            return static_cast<ai::state_trans_messages>(1);
        }
    } else if (chain_time < delta.value && chain_time > -delta.value && call<bool>(field_30, 0xB4)) {
        const auto *next = field_30->get_next_move();
        if (next->field_80.field_10.field_4 == 6 || next->field_80.field_10.field_4 == 4 ||
            viable(target(field_30->field_20)))
            return static_cast<ai::state_trans_messages>(1);
        call<void>(field_30, 0xBC);
    }
    return static_cast<ai::state_trans_messages>(75);
}

bool combat_state::advance_communications(Float, const combo_system_move *move)
{
    const float eta = field_34->get_eta_of_combat_signal(base_layer);
    const auto &results = move->field_4;
    auto *victim = target(field_30->field_1C);
    if (move->field_80.field_10.field_4 == 4) {
        if (field_FC && weapon_move(results.field_28) && victim) {
            auto *weapon = node<ai::weapon_inode>(this, string_hash{int(to_hash("WEAPON"))});
            const auto index = static_cast<uint16_t>(results.field_28 == 16 ? 0 : results.field_28 - 3);
            const auto handle = weapon->get_weapon_handle(index);
            field_30->field_78 = handle.field_0.field_0;
            call<void>(handle.get_volatile_ptr(), 0x2F4, get_actor()->my_handle, victim->my_handle, false);
            if (results.field_28 == 16)
                call<void>(weapon->get_weapon_handle(1).get_volatile_ptr(),
                           0x2F4,
                           get_actor()->my_handle,
                           victim->my_handle,
                           false);
        }
        return false;
    }
    if (!victim) {
        if (field_FC) {
            if (auto *previous = target(field_B8.field_0)) {
                field_FC = false;
                if (field_C0)
                    contact_effect(get_actor(), previous, field_C0->get_abs_position(), true);
            }
        }
        field_FA = true;
        return false;
    }
    auto *combat =
        static_cast<ai::combat_inode *>(victim->get_ai_core()->get_info_node(ai::combat_inode::default_id, true));
    if (!combat)
        return false;
    ai::combat_inode::incoming_move incoming;
    incoming.field_4 = get_actor()->my_handle.field_0;
    incoming.field_8 = g_world_ptr->time_manager.field_C;
    incoming.field_14 = results;
    incoming.field_14.field_78 += call<float>(field_30, 0x68);
    const float elapsed = g_world_ptr->time_manager.field_10;
    incoming.field_C = eta - elapsed;
    incoming.field_90 = call<bool>(this, 0x64);
    incoming.field_10 = incoming.field_C > 0.0f ? 1 : 2;
    if (field_BC && victim->has_physical_ifc() && !(victim->physical_ifc()->field_C & 0x80000))
        call<void, ai::combat_inode, const po &>(combat, 0x30, field_BC->get_abs_po());
    if (results.field_2C == 6 && field_34->get_als_layer(base_layer)->get_time_to_signal(event::ANIM_ACTION) > 0.0f)
        return false;
    if (field_FD) {
        call<void>(combat, 0xDC, incoming, false);
        field_FD = false;
    }
    if (field_FC && !field_FA) {
        field_FD = true;
        call<void>(combat, 0xD8, incoming);
        return true;
    }
    if (!weapon_move(incoming.field_14.field_28)) {
        if (results.field_78 > movement_distance)
            call<void>(combat, 0xD8, incoming);
        else if (elapsed * 3.0f < incoming.field_C)
            field_FA = true;
    }
    return false;
}

ai::state_trans_messages combat_state::_frame_advance(Float delta)
{
    ai::enhanced_state::frame_advance(delta);
    if (!call<bool>(field_30, 0xA4))
        return static_cast<ai::state_trans_messages>(2);
    const auto *move = field_30->get_cur_move();
    const bool attack = call<bool>(this, 0x4C, delta, move);
    call<void>(this, 0x40, delta, move, true, false);
    call<bool>(this, 0x44, delta);
    if (field_F8 && (move->field_4.field_24 < 4 || move->field_4.field_24 == 13)) {
        auto *victim = target(field_30->field_1C);
        if (field_C0)
            contact_effect(get_actor(), victim, field_C0->get_abs_position());
        field_F8 = false;
    }
    if (attack) {
        if (field_A0.source == get_actor()->my_handle)
            raise_impact(this, entity_base_vhandle{static_cast<uint32_t>(field_30->field_1C)});
        field_F8 = true;
    }
    if (field_90) {
        script::new_thread(frame_function, field_90);
        if (auto *thread = script::thread()) {
            thread->get_data_stack().push(delta.value);
            const float result = field_FA ? -1.0f : (field_FC && move->field_80.field_10.field_4 != 4 ? 1.0f : 0.0f);
            thread->get_data_stack().push(result);
        }
        script::exec_thread(true);
    }
    field_FC = false;
    return call<ai::state_trans_messages>(this, 0x50, delta);
}

namespace {
bool sphere_hit(const vector3d &point, float radius, entity *victim)
{
    if (victim->colgeom) {
        const float total = radius + victim->colgeom->get_bounding_sphere_radius();
        return (point - victim->get_abs_position()).length2() < total * total;
    }
    if (victim->is_a_conglomerate()) {
        auto *members = static_cast<conglomerate *>(victim)->field_FC;
        if (members && !members->empty()) {
            for (auto *member : *members) {
                if (!member->colgeom)
                    continue;
                const auto center =
                    member->get_abs_position() + member->colgeom->get_local_space_bounding_sphere_center();
                const float total = radius + member->colgeom->get_bounding_sphere_radius();
                if ((point - center).length2() < total * total)
                    return true;
            }
            return false;
        }
    }
    const float total = radius + victim->get_visual_radius();
    return (point - victim->get_visual_center()).length2() < total * total;
}
int attack_samples(combat_state *self, vector3d *points, int capacity, float radius)
{
    int count = 0;
    const float distance = radius * 1.5f;
    const float threshold = distance * distance;
    for (int i = 0; i < self->field_C4.m_size; ++i) {
        auto &entry = self->field_C4.m_data[i];
        const auto &position = entry.bone->get_abs_position();
        if (count < capacity) {
            points[count++] = position;
            if (count < capacity && (entry.position - position).length2() > threshold)
                points[count++] = entry.position + (position - entry.position) * 0.5f;
        }
        entry.position = position;
    }
    return count;
}
void apply_area_damage(combat_state *self, actor *owner, const vector3d &point,
                       const combo_system_move::results &results, entity *victim, damage_interface *damage)
{
    damage->apply_damage(owner,
                         bit_cast<float>(results.field_20),
                         1,
                         point,
                         ZEROVEC,
                         0,
                         results.field_8,
                         results.field_4,
                         results.field_C,
                         true,
                         ZEROVEC,
                         results.field_24,
                         false);
    damage->apply_subdue(owner, 0.1f);
    if (victim->is_an_actor()) {
        if (auto *core = victim->get_ai_core()) {
            if (self->field_A0.source == owner->my_handle) {
                raise_impact(self, victim->my_handle);
                contact_effect(owner, victim, point);
            }
            auto *node = static_cast<ai::damage_inode *>(core->get_info_node(ai::damage_inode::default_id, false));
            if (node && !(node->field_C->field_8 & 0x4000)) {
                node->field_1C = g_world_ptr->time_manager.field_C;
                node->field_20 = g_world_ptr->time_manager.field_8;
            }
        }
    }
}
bool hit_spawned_peds(combat_state *self, const combo_system_move *move, const vector3d *points, int count)
{
    bool hit = false;
    auto *owner = self->get_actor();
    const auto &position = owner->get_abs_position();
    const float radius = move->field_4.field_3C;
    for (auto *spawner : ped_spawner::ped_spawner_list) {
        auto *victim = spawner->get_my_actor();
        auto *core = victim->get_ai_core();
        if (spawner->field_5 || !ai::pedestrian_inode::is_a_pedestrian(core))
            continue;
        auto *ped = static_cast<ai::pedestrian_inode *>(core->get_info_node(ai::pedestrian_inode::default_id, true));
        if (ped->field_D1)
            continue;
        const auto &victim_position = victim->get_abs_position();
        if ((position - victim_position).length2() >= (radius + 30.0f) * (radius + 30.0f) ||
            contains(self->field_E4, victim->my_handle))
            continue;
        const float threshold = (radius + 0.75f) * (radius + 0.75f);
        for (int i = 0; i < count; ++i) {
            if ((points[i] - victim_position).length2() >= threshold)
                continue;
            hit = true;
            ped->m_hit_points -= bit_cast<float>(move->field_4.field_20);
            if (ped->m_hit_points <= 0.0f) {
                ped->m_hit_points = 0.0f;
                ped->field_D1 = true;
            }
            auto *combat = static_cast<ai::combat_inode *>(core->get_info_node(ai::combat_inode::default_id, false));
            if (combat) {
                call<void,
                     ai::combat_inode,
                     string_hash,
                     string_hash,
                     string_hash,
                     int,
                     vhandle_type<entity>,
                     const vector3d &,
                     bool>(combat,
                           0x124,
                           string_hash{int(to_hash("Wounded_Upper"))},
                           move->field_4.field_8,
                           move->field_4.field_C,
                           move->field_4.field_24,
                           vhandle_type<entity>{owner->my_handle},
                           ZEROVEC,
                           false);
            }
            if (self->field_A0.source == owner->my_handle) {
                raise_impact(self, victim->my_handle);
                contact_effect(owner, victim, points[i]);
            }
            append(self->field_E4, victim->my_handle);
            break;
        }
    }
    return hit;
}
}

bool combat_state::advance_area_damage_peds(const vector3d *points, int count, actor *owner, const vector3d &position,
                                            const combo_system_move::results &results, entity *victim,
                                            damage_interface *damage)
{
    const auto &victim_position = victim->get_abs_position();
    const float radius = results.field_3C;
    if ((position - victim_position).length2() >= (radius + 30.0f) * (radius + 30.0f) ||
        contains(field_D4, victim->my_handle))
        return false;
    int nearest = -1;
    float distance = 100000000.0f;
    for (int i = 0; i < count; ++i) {
        const float candidate = (points[i] - victim_position).length2();
        if (candidate < distance) {
            distance = candidate;
            nearest = i;
        }
    }
    if (distance >= (radius + 10.0f) * (radius + 10.0f) || !sphere_hit(points[nearest], radius, victim))
        return false;
    apply_area_damage(this, owner, points[nearest], results, victim, damage);
    return true;
}

bool combat_state::advance_area_damage(Float)
{
    if (field_C4.m_size <= 0)
        return false;
    actor *preferred = nullptr;
    if (auto *selection = node<ai::combat_target_inode>(this, ai::combat_target_inode::default_id, false)) {
        vhandle_type<actor> handle;
        call<vhandle_type<actor> *>(selection, 0x38, &handle);
        preferred = handle.get_volatile_ptr();
    }
    auto *owner = get_actor();
    const auto &position = owner->get_abs_position();
    const auto *move = field_30->get_cur_move();
    vector3d points[100];
    const int count = attack_samples(this, points, 100, move->field_4.field_3C);
    for (auto *damage : *damage_interface::all_damage_interfaces) {
        auto *victim = static_cast<entity *>(damage->field_4);
        if (victim == owner)
            continue;
        if (call<bool,
                 combat_state,
                 const vector3d *,
                 int,
                 actor *,
                 const vector3d &,
                 const combo_system_move::results &,
                 entity *,
                 damage_interface *>(this, 0x48, points, count, owner, position, move->field_4, victim, damage)) {
            append(field_D4, victim->my_handle);
            if (victim == preferred) {
                als::param_list params;
                params.add_param(als::param{54, 2.0f});
                field_34->set_desired_params(params, base_layer);
                params.clear();
            }
        }
    }
    return hit_spawned_peds(this, move, points, count);
}
