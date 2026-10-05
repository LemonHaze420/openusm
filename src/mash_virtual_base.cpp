#include "mash_virtual_base.h"
#include "native_enx.h"

#include "anim_record.h"
#include "enum_anim_key.h"
#include "func_wrapper.h"
#include "als_scripted_category.h"
#include "als_scripted_state.h"
#include "als_meta_anim_swing.h"
#include "als_transition_group_base.h"
#include "layer_state_machine_shared.h"
#include "log.h"
#include "mash_config.h"
#include "memory.h"
#include "meta_anim_interact.h"
#include "panelquad.h"
#include "fefloatingtext.h"
#include "femultilinetext.h"
#include "spidey_base_state.h"
#include "std_puppet_trans_state.h"
#include "ai_pedestrian.h"
#include "ai_action_processor_inode.h"
#include "ai_state_car.h"
#include "launch_layer_state.h"
#include "signal_enhanced_state.h"
#include "ai_state_swing.h"
#include "ai_state_run.h"
#include "ai_state_jump.h"
#include "base_ai_core.h"
#include "core_ai_resource.h"
#include "info_node_desc_list.h"
#include "ai_state_web_zip.h"
#include "ai_std_hero.h"
#include "ai_std_avoidance.h"
#include "ai_voice_box_inode.h"
#include "als_meta_aimed_shot_vert.h"
#include "als_mocomp.h"
#include "als_motion_compensator.h"
#include "als_simple_orientation.h"
#include "als_inode.h"
#include "als_use_anim_only.h"
#include "combat_inode.h"
#include "base_full_target_inode.h"
#include "ai_quad_path_inode.h"
#include "ai_std_combat_target.h"
#include "cpu_controller_inode.h"
#include "damage_inode.h"
#include "glass_house_inode.h"
#include "player_controller_inode.h"
#include "prop_physics_inode.h"
#include "slave_inode.h"
#include "std_default_trans_inode.h"
#include "std_puppet_inode.h"
#include "strength_test_inode.h"
#include "enhanced_state.h"
#include "info_node.h"
#include "interaction_inode.h"
#include "loco_inode.h"
#include "physics_inode.h"
#include "interaction.h"
#include "generic_interaction.h"
#include "trigger_region.h"
#include "venom_base_state.h"
#include "combo_system_move.h"
#include "nugget_wait_state.h"
#include "plr_loco_crawl_state.h"
#include "plr_loco_crawl_transition_state.h"
#include "player_combat_inode.h"
#include "pole_swing_inode.h"
#include "spidey_combat_inode.h"
#include "std_fear_inode.h"
#include "track_field_inode.h"
#include "traffic_inode.h"
#include "traffic_base_state.h"
#include "weapon_inode.h"
#include "string_hash.h"
#include "scripted_trans_group.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <new>

#if defined(OPENUSM_XBPACK_MODE) && !defined(TARGET_XBOX)
#include <cstddef>
#include <windows.h>
#endif

namespace {
constexpr size_t ORIGINAL_VTABLE_COUNT = 554;
constexpr std::array<uint32_t, ORIGINAL_VTABLE_COUNT> ORIGINAL_VTABLE_METADATA{{
    0x0087B82C, 0x0087E2E0, 0x0087E330, 0x0087B850, 0x0087E290, 0x0087947C, 0x00879518, 0x0087B810,
    0x008794E4, 0x008794B0, 0x00873638, 0x0087BB6C, 0x008739AC, 0x0087396C, 0x0087398C, 0x00874220,
    0x0087C220, 0x008796B8, 0x00874198, 0x00874158, 0x0087C268, 0x0087C0F0, 0x0087C298, 0x00874118,
    0x008740D8, 0x00874260, 0x0087B318, 0x00879700, 0x008743A0, 0x00874320, 0x0087C2F8, 0x008797E8,
    0x00874360, 0x00873FC0, 0x0087C0C0, 0x0087B2A8, 0x0087C2C8, 0x008742E0, 0x008742A0, 0x008741E0,
    0x00879798, 0x00879748, 0x00874000, 0x00874048, 0x00874090, 0x008744F8, 0x00874538, 0x0087C6F8,
    0x008744B8, 0x0087C728, 0x008745B8, 0x0087C768, 0x00874638, 0x00874578, 0x0087C6C8, 0x008745F8,
    0x00874B10, 0x0087C9B8, 0x00879A00, 0x00874880, 0x008748C0, 0x00874900, 0x00874840, 0x0087C7F8,
    0x008749D0, 0x0087C958, 0x00879928, 0x00874A50, 0x00879970, 0x00874AD0, 0x00874940, 0x00874800,
    0x0087C7C8, 0x0087A2B8, 0x0087A268, 0x0087C928, 0x00874980, 0x008747B8, 0x00874A90, 0x0087C988,
    0x008799B8, 0x00874A10, 0x008798D8, 0x00874C98, 0x00874B90, 0x00874BD0, 0x00874B50, 0x0087C9E8,
    0x00874C10, 0x00874CD8, 0x00874C58, 0x00874D98, 0x00874DE0, 0x008737C0, 0x0087B8BC, 0x00873734,
    0x008737A4, 0x00873788, 0x0087B8A0, 0x00879FC0, 0x0087376C, 0x00873750, 0x0087CBA8, 0x0087CC4C,
    0x0087B468, 0x0087CA78, 0x00875078, 0x00879C08, 0x008751C8, 0x00874F78, 0x00875038, 0x00879BC0,
    0x00874FB8, 0x00874F38, 0x0087A390, 0x0087CA48, 0x00879A48, 0x0087CBE8, 0x00874EF0, 0x00874EB0,
    0x00874E28, 0x00879B30, 0x00879B78, 0x0087CC18, 0x00874E70, 0x00879A90, 0x00874FF8, 0x00879AE0,
    0x0087CCE0, 0x0087A490, 0x008753D8, 0x0087CCAC, 0x0087A438, 0x0087A3D8, 0x00875418, 0x00875298,
    0x00875218, 0x00875258, 0x008752D8, 0x0087A4E8, 0x00875318, 0x0087A540, 0x00875398, 0x00875358,
    0x008738E8, 0x00873928, 0x00873948, 0x00873908, 0x0087CE10, 0x00875560, 0x0087559C, 0x0087CE80,
    0x0087CE40, 0x008755D8, 0x0087E3C8, 0x00875618, 0x0087CFB4, 0x0087CF38, 0x0087CFF0, 0x0087CF80,
    0x00875B88, 0x00875D00, 0x00875CA8, 0x00875D88, 0x00875C08, 0x00875C58, 0x00875BC8, 0x00875D48,
    0x008758A0, 0x00875B50, 0x00875860, 0x0087A598, 0x00875A08, 0x008759C0, 0x00875B10, 0x00875A90,
    0x00875AD0, 0x008758E8, 0x00875A50, 0x00875928, 0x00875980, 0x00875DC8, 0x00875E18, 0x00875E60,
    0x0087D120, 0x00875EA0, 0x00875F48, 0x0087A6A0, 0x0087D150, 0x00875F08, 0x0087D180, 0x00875F88,
    0x0087A720, 0x008762D8, 0x0087A6E0, 0x00876318, 0x00876358, 0x00876398, 0x00876198, 0x0087D1E0,
    0x00876058, 0x00876098, 0x00876118, 0x00876218, 0x0087D1B0, 0x00879D00, 0x00879CB8, 0x008761D8,
    0x00875FC8, 0x00876298, 0x00876158, 0x008760D8, 0x00876018, 0x00876258, 0x008763D8, 0x008764E0,
    0x0087B548, 0x00876628, 0x008765A8, 0x0087A808, 0x0087A9C0, 0x008765E8, 0x00876458, 0x008764A0,
    0x0087D460, 0x0087A910, 0x0087A8B8, 0x0087B5B8, 0x0087D42C, 0x0087A7B8, 0x0087B628, 0x0087A968,
    0x00876568, 0x0087B4D8, 0x00876520, 0x00876768, 0x0087A860, 0x0087B698, 0x00876418, 0x008767E8,
    0x008766A8, 0x00876668, 0x00876728, 0x008766E8, 0x008767A8, 0x0087D5A0, 0x00876828, 0x0087D6D0,
    0x0087D700, 0x0087D838, 0x0087D8D4, 0x00876AB0, 0x00876A50, 0x00879D48, 0x008769C8, 0x00876AF0,
    0x00876A10, 0x00876B30, 0x0087D904, 0x00879DD8, 0x00876C38, 0x00876CC8, 0x00876B70, 0x00876D50,
    0x00876BB8, 0x00876C80, 0x00876BF8, 0x00879D90, 0x00876D90, 0x00876D10, 0x0087B1C8, 0x0087B238,
    0x00873D38, 0x00873D88, 0x00873CF0, 0x0087B160, 0x0087B028, 0x00873CA8, 0x0087B090, 0x0087B0F8,
    0x00876DD8, 0x0087D938, 0x00879E28, 0x00876E18, 0x0087D9D8, 0x0087DA74, 0x00876E58, 0x00876EA0,
    0x00876EE0, 0x00876F28, 0x00877080, 0x008770C8, 0x00877110, 0x00877000, 0x00879EB8, 0x00876FB8,
    0x00876F70, 0x00879E70, 0x00877038, 0x00876868, 0x008768C8, 0x00876928, 0x00876988, 0x00877158,
    0x0087DAD4, 0x008771A0, 0x00877330, 0x00877298, 0x008773D8, 0x00877258, 0x00877378, 0x008772E0,
    0x00877438, 0x00877218, 0x0087AA18, 0x0087DB04, 0x008771E0, 0x00877478, 0x0087DB34, 0x008774B8,
    0x0087A640, 0x00875EE0, 0x0087A5E0, 0x008774F8, 0x00877534, 0x00877570, 0x0087DB64, 0x008775B0,
    0x00873C20, 0x00879598, 0x00879550, 0x00873BE0, 0x008795E0, 0x0087CEC0, 0x0087CEFC, 0x00875740,
    0x0087C048, 0x00873AC8, 0x0087BFDC, 0x00873B10, 0x00873B78, 0x0087C018, 0x0087BBD0, 0x0087BB9C,
    0x0087A1FC, 0x0087BF68, 0x0087BD00, 0x0087BE38, 0x008739D8, 0x0087C370, 0x0087C328, 0x0087C410,
    0x00874420, 0x0087C4B0, 0x0087C5F0, 0x008743E0, 0x0087C550, 0x0087D3D0, 0x0087D370, 0x0087DB94,
    0x00877678, 0x008776C8, 0x008775F0, 0x00877638, 0x00877758, 0x0087DBC4, 0x00877718, 0x00875108,
    0x0087A350, 0x0087A308, 0x008750B8, 0x0087CC7C, 0x00875188, 0x00875148, 0x00874D50, 0x0087CA18,
    0x00874D18, 0x00875788, 0x00875818, 0x008757D0, 0x00879C70, 0x0087DC00, 0x008777C0, 0x0087DC44,
    0x0087DAA4, 0x0087B388, 0x0087C68C, 0x0087A218, 0x00874460, 0x00873A18, 0x00873A60, 0x0087BFA0,
    0x00873EE0, 0x00873F30, 0x00873E90, 0x00873E10, 0x0087C090, 0x00873E5C, 0x00873F80, 0x00879670,
    0x00879628, 0x00873DD0, 0x0087DC74, 0x0087DCA4, 0x00877870, 0x00877828, 0x0087DCD4, 0x00879F00,
    0x0087DD04, 0x008778B8, 0x0087DD34, 0x0087B3F8, 0x008746F8, 0x00874738, 0x0087C798, 0x00874778,
    0x00879830, 0x00879880, 0x00874678, 0x008746B8, 0x0087DD68, 0x008778F8, 0x0087DDA8, 0x00877A60,
    0x00877AE0, 0x008779D0, 0x00877A18, 0x00877980, 0x00877938, 0x00877AA8, 0x0087DE08, 0x0087DF98,
    0x00877C08, 0x00877CD0, 0x00877B78, 0x0087DE68, 0x00877BB8, 0x00877C90, 0x00877D60, 0x00877C48,
    0x0087DE38, 0x0087AA78, 0x0087AAC8, 0x00877B20, 0x00877DA0, 0x00877F20, 0x00877D20, 0x00877EE0,
    0x00877F60, 0x00877DE0, 0x0087AB68, 0x00877E60, 0x00877E20, 0x00877EA0, 0x0087AB18, 0x00879F48,
    0x0087DDD8, 0x0087E040, 0x00878180, 0x00877FA0, 0x00877FE0, 0x00878240, 0x0087E088, 0x0087B708,
    0x0087AD70, 0x0087ACC0, 0x008781C0, 0x0087AD18, 0x0087AC10, 0x00878300, 0x0087E010, 0x0087DFE0,
    0x0087B778, 0x0087ADC8, 0x008780B0, 0x00878068, 0x00878280, 0x00878140, 0x00878200, 0x00878020,
    0x008782C0, 0x0087AC68, 0x008780F8, 0x0087E3A4, 0x0087B8F8, 0x00878340, 0x00878380, 0x00875494,
    0x0087B918, 0x0087B954, 0x00875658, 0x008790B8, 0x00878C90, 0x00879218, 0x00878910, 0x00878D08,
    0x00878F58, 0x00879008, 0x00879168, 0x00878820, 0x00878780, 0x008787D0, 0x008786E0, 0x008792C8,
    0x00878730, 0x00878B40, 0x00878C18, 0x008789B0, 0x00878A50, 0x00878BA0, 0x00878D80, 0x00878A00,
    0x00878DF8, 0x00878EA8, 0x008783C0, 0x00878AF0, 0x00878690, 0x008788C0, 0x00879378, 0x00878870,
    0x00878510, 0x00878570, 0x008784B0, 0x008785D0, 0x00878630, 0x00878410, 0x00878460, 0x00879268,
    0x00878AA0, 0x00878960, 0x0087E214, 0x0087E250, 0x0087E1D8, 0x0087E1B8, 0x00873C60, 0x00873818,
    0x00873858, 0x0087BB3C, 0x008793D4, 0x00873618, 0x0087BA60, 0x0087B990, 0x0087A0F0, 0x0087AE58,
    0x00879FE0, 0x0087553C, 0x0087E380, 0x0087B8D8, 0x008754F4, 0x00875518, 0x008793F8, 0x0087AE20,
    0x00879F9C, 0x00873C84,
}};
static_assert(ORIGINAL_VTABLE_METADATA.size() == ORIGINAL_VTABLE_COUNT);

[[noreturn]] void report_unsupported_standalone_vtable(uint32_t index,
                                                         const void *object)
{
    const auto original_address = index < ORIGINAL_VTABLE_COUNT
        ? ORIGINAL_VTABLE_METADATA[index]
        : 0;
    std::fprintf(stderr,
                 "Standalone unsupported mash vtable index %u "
                 "(original address 0x%08X, object=%p)\n",
                 index,
                 original_address,
                 object);
    std::fflush(stderr);
    std::abort();
}

template<typename T>
void __fastcall native_mash_unmash(
    T *self, int, mash_info_struct *info, void *context)
{
    if constexpr (std::is_base_of_v<ai::info_node, T> &&
                  !std::is_same_v<decltype(&T::unmash), decltype(&mash_virtual_base::unmash)>)
        self->T::unmash(info, context);
    else
        self->T::_unmash(info, context);
}

template<typename T>
int __fastcall native_mash_sizeof(T *)
{
    return sizeof(T);
}

uint32_t __fastcall native_mash_type(const mash_virtual_base *self)
{
    for (uint32_t i = 0; i < ORIGINAL_VTABLE_COUNT; ++i) {
        if (mash_virtual_base::vtable()[i] == bit_cast<void *>(self->m_vtbl)) {
            return i;
        }
    }
    report_unsupported_standalone_vtable(ORIGINAL_VTABLE_COUNT, self);
}

void __fastcall native_action_processor_destruct(ai::ai_action_processor_inode *self)
{
    self->destruct_mashed_class();
}

bool __fastcall native_action_processor_needs_advance(ai::ai_action_processor_inode *)
{
    return true;
}

void __fastcall native_action_processor_advance(
    ai::ai_action_processor_inode *self, void *, Float elapsed)
{
    self->frame_advance(elapsed);
}

void __fastcall native_action_processor_activate(
    ai::ai_action_processor_inode *self, void *, ai::ai_core *core)
{
    self->_activate(core);
}

void __fastcall native_action_processor_deactivate(ai::ai_action_processor_inode *)
{

}

void __fastcall native_action_processor_reset(ai::ai_action_processor_inode *self)
{
    self->_reset();
}

template<typename T>
int __fastcall native_scripted_state_mocomp(T *self)
{
    if constexpr (std::is_same_v<T, als::base_layer_scripted_state>) {
        return self->field_54;
    } else {
        return self->_get_mocomp_type();
    }
}

int __fastcall native_scripted_state_filter_count(
    als::scripted_state *, void *, als::animation_logic_system *, als::state_machine *)
{
    return 0;
}

int __fastcall native_scripted_state_filter(
    als::scripted_state *self, void *, int out,
    als::animation_logic_system *system, als::state_machine *machine, int index)
{
    return self->get_filter(out, system, machine, index);
}

void __fastcall native_scripted_state_implicit(
    als::scripted_state *self, void *, als::request_data *out,
    als::animation_logic_system *system, als::state_machine *machine)
{
    *out = self->_do_implicit_trans(system, machine);
}

void __fastcall native_scripted_state_explicit(
    als::scripted_state *self, void *, als::request_data *out,
    als::animation_logic_system *system, als::state_machine *machine, string_hash event)
{
    *out = self->_do_explicit_trans(system, machine, event);
}

void __fastcall native_scripted_state_layer(
    als::scripted_state *self, void *, als::request_data *out,
    als::animation_logic_system *system, als::state_machine *machine)
{
    *out = self->_do_layer_trans(system, machine);
}

void __fastcall native_scripted_state_post(
    als::scripted_state *self, void *, als::animation_logic_system *system,
    als::state_machine *machine, als::transition_post_handle handle)
{
    self->_do_post_trans(system, machine, handle);
}

string_hash *__fastcall native_scripted_state_animation(
    const als::scripted_state *self, void *, string_hash *out)
{
    *out = self->get_nal_anim_name();
    return out;
}

template<typename T>
void *native_mash_vtable()
{
    static std::array<void *, 96> table {};
    if (table[1] == nullptr) {
        table[1] = bit_cast<void *>(&native_mash_unmash<T>);
        table[0x0C / sizeof(void *)] = bit_cast<void *>(&native_mash_type);
        if constexpr (requires { T::g_vtbl; })
            table[0x18 / sizeof(void *)] = T::g_vtbl[0x18 / sizeof(void *)];
        table[0x1C / sizeof(void *)] = bit_cast<void *>(&native_mash_sizeof<T>);
        table[0x34 / sizeof(void *)] = bit_cast<void *>(&native_mash_sizeof<T>);
        if constexpr (std::is_base_of_v<ai::info_node, T>)
            table[0x2C / sizeof(void *)] = bit_cast<void *>(&native_mash_sizeof<T>);
        table[0x38 / sizeof(void *)] = bit_cast<void *>(&native_mash_sizeof<T>);
        table[0x4C / sizeof(void *)] = bit_cast<void *>(&native_mash_sizeof<T>);
        if constexpr (std::is_same_v<T, ai::ai_action_processor_inode>) {
            table[0] = bit_cast<void *>(&native_action_processor_destruct);
            table[0x18 / sizeof(void *)] = bit_cast<void *>(&native_action_processor_needs_advance);
            table[0x1C / sizeof(void *)] = bit_cast<void *>(&native_action_processor_advance);
            table[0x20 / sizeof(void *)] = bit_cast<void *>(&native_action_processor_activate);
            table[0x24 / sizeof(void *)] = bit_cast<void *>(&native_action_processor_deactivate);
            table[0x28 / sizeof(void *)] = bit_cast<void *>(&native_action_processor_reset);
            table[0x2C / sizeof(void *)] = bit_cast<void *>(&native_mash_sizeof<T>);
        }
        if constexpr (std::is_base_of_v<als::scripted_state, T>) {
            table[0x18 / sizeof(void *)] = bit_cast<void *>(&native_scripted_state_mocomp<T>);
            table[0x1C / sizeof(void *)] = bit_cast<void *>(&native_scripted_state_filter_count);
            table[0x20 / sizeof(void *)] = bit_cast<void *>(&native_scripted_state_filter);
            table[0x24 / sizeof(void *)] = bit_cast<void *>(&native_scripted_state_implicit);
            table[0x28 / sizeof(void *)] = bit_cast<void *>(&native_scripted_state_explicit);
            table[0x2C / sizeof(void *)] = bit_cast<void *>(&native_scripted_state_layer);
            table[0x30 / sizeof(void *)] = bit_cast<void *>(&native_scripted_state_post);
            table[0x34 / sizeof(void *)] = bit_cast<void *>(&native_scripted_state_animation);
        }
    }
    return table.data();
}


unsigned __fastcall ped_transition_type(ai::base_state *, void *) { return 169; }
bool __fastcall ped_transition_subclass(ai::base_state *, void *, unsigned type)
{
    return type == 567 || type == 573;
}
ai::state_trans_messages __fastcall ped_transition_advance(ai::base_state *, void *, Float)
{
    return static_cast<ai::state_trans_messages>(75);
}
void __fastcall ped_transition_message(ai::base_state *, void *, ai::state_trans_action *out,
                                      Float, ai::state_trans_messages)
{
    *out = {ai::NO_ACTION, string_hash{}, static_cast<ai::state_trans_messages>(75), nullptr};
}
void __fastcall ped_transition_check(ai::base_state *self, void *, ai::state_trans_action *out, Float elapsed)
{
    auto *core = self->get_core();
    auto *combat = static_cast<ai::combat_inode *>(core->get_info_node(ai::combat_inode::default_id, false));
    *out = {ai::NO_ACTION, string_hash{}, static_cast<ai::state_trans_messages>(75), nullptr};
    if (combat != nullptr) {
        auto needs_react = reinterpret_cast<bool (__fastcall *)(ai::combat_inode *, void *, Float)>(
            get_vfunc(combat->m_vtbl, 0xE0));
        if (needs_react(combat, nullptr, elapsed)) {
            out->the_action = ai::GOTO_STATE;
            out->field_4 = string_hash{"ped_hit_react"};
            if (core->field_6C->field_44)
                core->field_4C |= 1;
        }
    }
}
void *ped_transition_vtable()
{
    static const auto table = [] {
        std::array<void *, 14> result;
        std::copy_n(static_cast<void **>(ai::base_state::native_vtable()), result.size(), result.begin());
        result[3] = reinterpret_cast<void *>(&ped_transition_type);
        result[4] = reinterpret_cast<void *>(&ped_transition_subclass);
        result[8] = reinterpret_cast<void *>(&ped_transition_advance);
        result[11] = reinterpret_cast<void *>(&ped_transition_check);
        result[12] = reinterpret_cast<void *>(&ped_transition_message);
        return result;
    }();
    return const_cast<void **>(table.data());
}

unsigned __fastcall hostage_transition_type(ai::base_state *, void *) { return 293; }
void __fastcall hostage_transition_nodes(ai::base_state *, void *, ai::info_node_desc_list *list)
{
    list->add_entry({string_hash{"AI_HOSTAGE_VICTIM"}, 285});
}
void __fastcall hostage_transition_check(ai::base_state *self, void *, ai::state_trans_action *out, Float)
{
    auto *node = static_cast<ai::targetable_inode_285 *>(
        self->get_core()->get_info_node(string_hash{"AI_HOSTAGE_VICTIM"}, true));
    *out = {ai::NO_ACTION, string_hash{}, static_cast<ai::state_trans_messages>(75), nullptr};
    if (node->field_1C == 1) {
        out->the_action = ai::GOTO_STATE;
        out->field_4 = string_hash{"hostage_victim"};
    }
}
void *hostage_transition_vtable()
{
    static const auto table = [] {
        std::array<void *, 14> result;
        std::copy_n(static_cast<void **>(ai::base_state::native_vtable()), result.size(), result.begin());
        result[3] = reinterpret_cast<void *>(&hostage_transition_type);
        result[4] = reinterpret_cast<void *>(&ped_transition_subclass);
        result[8] = reinterpret_cast<void *>(&ped_transition_advance);
        result[9] = reinterpret_cast<void *>(&hostage_transition_nodes);
        result[11] = reinterpret_cast<void *>(&hostage_transition_check);
        result[12] = reinterpret_cast<void *>(&ped_transition_message);
        return result;
    }();
    return const_cast<void **>(table.data());
}

template<typename T>
void set_native_mash_vtable(T *object, uint32_t type)
{
    if (type >= ORIGINAL_VTABLE_COUNT || mash_virtual_base::vtable()[type] == nullptr) {
        report_unsupported_standalone_vtable(type, object);
    }
    *reinterpret_cast<std::intptr_t *>(object) =
        bit_cast<std::intptr_t>(mash_virtual_base::vtable()[type]);
}

template<typename T>
void *create_mash_class(uint32_t type)
{
    auto *object = new T {};
    set_native_mash_vtable(object, type);
    return object;
}

template<typename T>
void *create_mash_class_in_place(
    uint32_t type, mash_virtual_base *storage, int storage_size)
{
    assert(storage != nullptr);
    assert(storage_size >= static_cast<int>(sizeof(T)));
    if constexpr ((std::is_base_of_v<ai::info_node, T> ||
                   std::is_base_of_v<ai::base_state, T> ||
                   std::is_base_of_v<als::als_meta_anim_base, T>) &&
                  !std::is_aggregate_v<T> &&
                  std::is_constructible_v<T, from_mash_in_place_constructor *>) {
        storage = reinterpret_cast<mash_virtual_base *>(::new (static_cast<void *>(storage))
            T{static_cast<from_mash_in_place_constructor *>(nullptr)});
    }
    set_native_mash_vtable(storage, type);
    return storage;
}

template<typename T>
void *create_mash_class(
    uint32_t type, mash_virtual_base *storage, int storage_size)
{
    return storage != nullptr
        ? create_mash_class_in_place<T>(type, storage, storage_size)
        : create_mash_class<T>(type);
}

void *create_native_mash_class(
    uint32_t type, mash_virtual_base *storage = nullptr, int storage_size = 0)
{
    if (type < 10) {
        if (storage != nullptr)
            assert(storage_size >= static_cast<int>(native_enx::size(type)));
        unsigned size;
        return native_enx::construct(type, storage, &size);
    }
    if (storage != nullptr && type >= 545 && type <= 549) {
        return interaction::construct_native_in_place(type, storage, storage_size);
    }
    switch (type) {
    case 11:
        return create_mash_class<ai::ai_action_processor_inode>(type, storage, storage_size);
    case 53:
        return create_mash_class<ai::nugget_wait_state>(type, storage, storage_size);
    case 94:
        return create_mash_class<combo_system_move>(type, storage, storage_size);
    case 95:
        return create_mash_class<combo_system_move::dialation_info>(
            type, storage, storage_size);
    case 96:
        return create_mash_class<combo_system_move::link_info>(
            type, storage, storage_size);
    case 97:
        return create_mash_class<combo_system_move::range_info>(
            type, storage, storage_size);
    case 98:
        return create_mash_class<combo_system_move::requirements>(
            type, storage, storage_size);
    case 99:
        return create_mash_class<combo_system_move::results>(
            type, storage, storage_size);
    case 100:
        return create_mash_class<combo_system_move::target_info>(type, storage, storage_size);
    case 101:
        return create_mash_class<combo_system_move::trigger_info>(type, storage, storage_size);
    case 144:
        return create_mash_class<anim_key>(type, storage, storage_size);
    case 145:
        return create_mash_class<anim_record>(type, storage, storage_size);
    case 146:
        return create_mash_class<paired_anim_record>(type, storage, storage_size);
    case 147:
        return create_mash_class<enum_anim_key>(type, storage, storage_size);
    case 148:
        return create_mash_class<ai::interaction_inode>(type, storage, storage_size);
    case 149:
        return create_mash_class<als::meta_aimed_shot_vert>(type, storage, storage_size);
    case 150:
        return create_mash_class<ai::meta_anim_strength_test>(type, storage, storage_size);
    case 152:
        return create_mash_class<ai::strength_test_inode>(type, storage, storage_size);
    case 156:
        return create_mash_class<ai::nonpath_loco_inode>(type, storage, storage_size);
    case 157:
        return create_mash_class<ai::ped_avoidance_inode>(type, storage, storage_size);
    case 158:
        return create_mash_class<ai::ped_combat_inode>(type, storage, storage_size);
    case 159:
        return create_mash_class<ai::pedestrian_inode>(type, storage, storage_size);
    case 169:
        return create_mash_class<ai::base_state>(type, storage, storage_size);
    case 177:
        return create_mash_class<ai::pedestrian_idle_state>(type, storage, storage_size);
    case 181:
        return create_mash_class<plr_loco_crawl_state>(type, storage, storage_size);
    case 182:
        return create_mash_class<plr_loco_crawl_transition_state>(
            type, storage, storage_size);
    case 248:
        return create_mash_class<ai::spidey_combat_inode>(type, storage, storage_size);
    case 258:
        return create_mash_class<ai::ai_car_inode>(type, storage, storage_size);
    case 259:
        return create_mash_class<ai::drive_car_state>(type, storage, storage_size);
    case 285:
        return create_mash_class<ai::targetable_inode_285>(type, storage, storage_size);
    case 293:
        return create_mash_class<ai::base_state>(type, storage, storage_size);
    case 303:
        return create_mash_class<ai::jump_state>(type, storage, storage_size);
    case 304:
        return create_mash_class<ai::pole_swing_inode>(type, storage, storage_size);
    case 315:
        return create_mash_class<ai::std_puppet_inode>(type, storage, storage_size);
    case 316:
        return create_mash_class<ai::std_puppet_trans_state>(type, storage, storage_size);
    case 317:
        return create_mash_class<ai::run_state>(type, storage, storage_size);
    case 318:
        return create_mash_class<ai::swing_inode>(type, storage, storage_size);
    case 319:
        return create_mash_class<ai::swing_state>(type, storage, storage_size);
    case 323:
        return create_mash_class<ai::hero_base_state>(type, storage, storage_size);
    case 324:
        return create_mash_class<ai::spidey_base_state>(type, storage, storage_size);
    case 325:
        return create_mash_class<ai::venom_base_state>(type, storage, storage_size);
    case 326:
        return create_mash_class<ai::web_zip_inode>(type, storage, storage_size);
    case 327:
        return create_mash_class<ai::web_zip_state>(type, storage, storage_size);
    case 330:
        return create_mash_class<ai::launch_layer_state>(type, storage, storage_size);
    case 333:
        return create_mash_class<ai::als_inode>(type, storage, storage_size);
    case 336:
        return create_mash_class<ai::avoidance_inode>(type, storage, storage_size);
    case 338:
        return create_mash_class<ai::biped_layer_inode>(type, storage, storage_size);
    case 341:
        return create_mash_class<ai::quad_path_inode>(type, storage, storage_size);
    case 342:
        return create_mash_class<ai::combat_inode>(type, storage, storage_size);
    case 343:
        return create_mash_class<ai::damage_inode>(type, storage, storage_size);
    case 344:
        return create_mash_class<ai::combat_inode::incoming_move>(type, storage, storage_size);
    case 346:
        return create_mash_class<ai::player_combat_inode>(type, storage, storage_size);
    case 349:
        return create_mash_class<ai::base_full_target_inode>(type, storage, storage_size);
    case 351:
        return create_mash_class<ai::combat_target_inode>(type, storage, storage_size);
    case 356:
        return create_mash_class<ai::venom_combat_target_inode>(type, storage, storage_size);
    case 357:
        return create_mash_class<ai::cpu_controller_inode>(type, storage, storage_size);
    case 358:
        return create_mash_class<ai::player_controller_inode>(type, storage, storage_size);
    case 370:
        return create_mash_class<ai::std_default_state_set_base>(type, storage, storage_size);
    case 371:
        return create_mash_class<ai::std_default_trans_inode>(type, storage, storage_size);
    case 375:
        return create_mash_class<ai::std_fear_inode>(type, storage, storage_size);
    case 383:
        return create_mash_class<ai::glass_house_inode>(type, storage, storage_size);
    case 384:
        return create_mash_class<ai::hero_inode>(type, storage, storage_size);
    case 391:
        return create_mash_class<ai::loco_inode>(type, storage, storage_size);
    case 402:
        return create_mash_class<ai::physics_inode>(type, storage, storage_size);
    case 403:
        return create_mash_class<ai::prop_physics_inode>(type, storage, storage_size);
    case 408:
        return create_mash_class<ai::slave_inode>(type, storage, storage_size);
    case 410:
        return create_mash_class<ai::weapon_inode>(type, storage, storage_size);
    case 420:
        return create_mash_class<ai::track_field_inode>(type, storage, storage_size);
    case 421:
        if (storage != nullptr) {
            assert(storage_size >= static_cast<int>(sizeof(ai::traffic_base_state)));
            return ::new (static_cast<void *>(storage)) ai::traffic_base_state{
                static_cast<from_mash_in_place_constructor *>(nullptr)};
        }
        return new ai::traffic_base_state{};
    case 422:
        return create_mash_class<ai::traffic_inode>(type, storage, storage_size);
    case 456:
        return create_mash_class<ai::voice_box_inode>(type, storage, storage_size);
    case 483:
        return create_mash_class<als::layer_state_machine_shared>(
            type, storage, storage_size);
    case 484:
        return create_mash_class<als::state_machine_shared>(type, storage, storage_size);
    case 488:
        return create_mash_class<als::als_meta_anim_swing>(type, storage, storage_size);
    case 489:
        return create_mash_class<als::als_meta_linear_blend>(type, storage, storage_size);
    case 490:
        return create_mash_class<als::motion_compensator>(type, storage, storage_size);
    case 493:
        return create_mash_class<als::begin_biped_physics>(type, storage, storage_size);
    case 503:
        return create_mash_class<als::crawl_transition>(type, storage, storage_size);
    case 514:
        return create_mash_class<als::null_mocomp>(type, storage, storage_size);
    case 522:
        return create_mash_class<als::simple_orientation>(type, storage, storage_size);
    case 523:
        return create_mash_class<als::simple_orientation_ped>(type, storage, storage_size);
    case 525:
        return create_mash_class<als::use_anim_only>(type, storage, storage_size);
    case 530:
        return create_mash_class<als::base_layer_scripted_state>(
            type, storage, storage_size);
    case 531:
        return create_mash_class<als::scripted_category>(type, storage, storage_size);
    case 532:
        return create_mash_class<als::scripted_state>(type, storage, storage_size);
    case 533:
        return create_mash_class<als::scripted_trans_group>(type, storage, storage_size);
    case 535:
        return create_mash_class<ai::enhanced_state>(type, storage, storage_size);
    case 536:
        return create_mash_class<ai::signal_enhanced_state>(type, storage, storage_size);
    case 537:
        return create_mash_class<ai::info_node>(type, storage, storage_size);
    case 541:
        return create_mash_class<PanelQuad>(type, storage, storage_size);
    case 542:
        return create_mash_class<FEFloatingText>(type, storage, storage_size);
    case 543:
        return create_mash_class<FEMultiLineText>(type, storage, storage_size);
    case 544:
        return create_mash_class<FEText>(type, storage, storage_size);
    default:
        report_unsupported_standalone_vtable(type, nullptr);
    }
}
}

#if defined(OPENUSM_XBPACK_MODE) && !defined(TARGET_XBOX)
namespace {

struct xbox_type_mapping
{
    uint32_t xbox_type;
    uint32_t pc_type;
};

template<size_t Size>
bool translate_type(uint32_t xbox_type,
                    const xbox_type_mapping (&mappings)[Size],
                    uint32_t &pc_type)
{
    for (const auto &mapping : mappings) {
        if (mapping.xbox_type == xbox_type) {
            pc_type = mapping.pc_type;
            return true;
        }
    }

    return false;
}

constexpr xbox_type_mapping XBOX_V14_FACTORY_TYPES[] {
    {to_hash("run_state"), 0x13D},
    {to_hash("jump_state"), 0x12F},
    {to_hash("hit_react_state"), 0x171},
    {to_hash("plr_loco_crawl_state"), 0x0B5},
    {to_hash("debug_state"), 0x118},
    {to_hash("aimed_throw_state"), 0x140},
    {to_hash("plr_loco_crawl_transition_state"), 0x0B6},
    {to_hash("interaction_state"), 0x12B},
    {to_hash("put_down_state"), 0x12E},
    {to_hash("pick_up_state"), 0x12D},
    {to_hash("subdued_state"), 0x174},
    {to_hash("web_zip_state"), 0x147},
    {to_hash("venom_grapple_thrown_state"), 0x1B7},
    {to_hash("spidey_combat_state"), 0x116},
    {to_hash("venom_combat_state"), 0x117},
};
static_assert(sizeof(XBOX_V14_FACTORY_TYPES) / sizeof(XBOX_V14_FACTORY_TYPES[0]) == 15);

#ifdef OPENUSM_XBPACK_V10
struct v10_type_mapping
{
    uint32_t xbox_type;
    uint32_t pc_type;
    uint32_t pc_vtable;
};

constexpr v10_type_mapping V10_TYPES[] {
    {0x02E, 0x02F, 0x0087C6F8},
    {0x057, 0x05E, 0x0087B8BC},
    {0x05C, 0x063, 0x00879FC0},
    {0x05F, 0x066, 0x0087CBA8},
    {0x060, 0x067, 0x0087CC4C},
    {0x061, 0x069, 0x0087CA78},
    {0x06B, 0x073, 0x0087CA48},
    {0x06D, 0x075, 0x0087CBE8},
    {0x073, 0x07B, 0x0087CC18},
    {0x078, 0x080, 0x0087CCE0},
    {0x13D, 0x14D, 0x0087CEC0},
    {0x13E, 0x14E, 0x0087CEFC},
    {0x140, 0x150, 0x0087C048},
    {0x170, 0x180, 0x0087DAA4},
    {0x12E, 0x13E, 0x0087DB34},
    {0x12B, 0x13B, 0x0087DB04},
    {0x14E, 0x15F, 0x0087C410},
    {0x14F, 0x15F, 0x0087C410},
    {0x150, 0x160, 0x00874420},
    {0x151, 0x161, 0x0087C4B0},
    {0x154, 0x164, 0x0087C550},
    {0x16F, 0x17F, 0x0087DC44},
    {0x172, 0x182, 0x0087C68C},
    {0x136, 0x146, 0x0087DB64},
    {0x120, 0x130, 0x0087DAD4},
    {0x0E8, 0x0F8, 0x0087D700},
    {0x0EA, 0x0FA, 0x0087D8D4},
    {0x163, 0x173, 0x0087CC7C},
    {0x08D, 0x094, 0x0087CE10},
    {0x094, 0x09D, 0x0087CF38},
    {0x096, 0x09F, 0x0087CF80},
    {0x1B4, 0x1C8, 0x0087DDD8},
    {0x181, 0x192, 0x0087DC74},
    {0x182, 0x193, 0x0087DCA4},
    {0x185, 0x196, 0x0087DCD4},
    {0x17C, 0x18C, 0x0087C090},
    {0x189, 0x19A, 0x0087DD34},
    {0x08F, 0x098, 0x0087CE40},
    {0x093, 0x09C, 0x0087CFB4},
    {0x095, 0x09E, 0x0087CFF0},
    {0x0F2, 0x102, 0x0087D904},
    {0x10D, 0x11D, 0x0087DA74},
    {0x167, 0x177, 0x0087CA18},
    {0x156, 0x166, 0x0087D370},
    {0x157, 0x167, 0x0087DB94},
    {0x193, 0x1A4, 0x0087DD68},
    {0x187, 0x198, 0x0087DD04},
    {0x195, 0x1A6, 0x0087DDA8},
    {0x19D, 0x1AE, 0x0087DE08},
    {0x1A1, 0x1B3, 0x0087DE68},
    {0x1A6, 0x1B8, 0x0087DE38},
    {0x162, 0x172, 0x008750B8},
    {0x16D, 0x17D, 0x0087DC00},
    {0x148, 0x157, 0x0087BB9C},
    {0x146, 0x156, 0x0087BBD0},
    {0x149, 0x15A, 0x0087BD00},
    {0x14A, 0x15B, 0x0087BE38},
    {0x142, 0x152, 0x0087BFDC},
    {0x145, 0x155, 0x0087C018},
    {0x155, 0x165, 0x0087D3D0},
    {0x00B, 0x00B, 0x0087BB6C},
    {0x147, 0x158, 0x0087A1FC},
    {0x20B, 0x221, 0x0087553C},
    {0x20C, 0x222, 0x0087E380},
    {0x20D, 0x223, 0x0087B8D8},
    {0x1CF, 0x1E3, 0x0087E3A4},
    {0x1D0, 0x1E4, 0x0087B8F8},
    {0x1D1, 0x1E5, 0x00878340},
    {0x1D2, 0x1E7, 0x00875494},
    {0x1D3, 0x1E8, 0x0087B918},
    {0x1D4, 0x1E9, 0x0087B954},
};

bool translate_v10_factory_type(uint32_t xbox_type, uint32_t &pc_type)
{
    for (const auto &mapping : V10_TYPES) {
        if (mapping.xbox_type == xbox_type) {
            pc_type = mapping.pc_type;
            return true;
        }
    }

    return false;
}

bool translate_v10_vtable(uint32_t xbox_type, uint32_t &pc_vtable)
{
    for (const auto &mapping : V10_TYPES) {
        if (mapping.xbox_type == xbox_type) {
            pc_vtable = mapping.pc_vtable;
            return true;
        }
    }

    return false;
}
#endif

bool translate_factory_type(uint32_t xbox_type, uint32_t &pc_type)
{
#ifdef OPENUSM_XBPACK_V10
    if (translate_v10_factory_type(xbox_type, pc_type)) {
        return true;
    }
#endif

    constexpr uint32_t MAX_PC_FACTORY_TYPE = 0x229;
    if (xbox_type <= MAX_PC_FACTORY_TYPE) {
        pc_type = xbox_type;
        return true;
    }

    return translate_type(xbox_type, XBOX_V14_FACTORY_TYPES, pc_type);
}

void report_unsupported_type(const char *category,
                             uint32_t hash,
                             const void *object = nullptr,
                             const void *caller = nullptr)
{
    char message[224];
    std::snprintf(message,
                  sizeof(message),
                  "XBPACK unsupported %s hash 0x%08X object=%p caller=%p\n",
                  category,
                  hash,
                  object,
                  caller);

#if defined(_DEBUG)
    OutputDebugStringA(message);
    DebugBreak();
#else
    MessageBox(NULL, message, "openusm", MB_OK);
#endif
    std::abort();
}

extern "C" __attribute__((noinline, used)) void *__cdecl xbpack_create_subclass(
    uint32_t xbox_type)
{
    uint32_t pc_type = 0;
    if (!translate_factory_type(xbox_type, pc_type)) {
        report_unsupported_type("factory", xbox_type);
        return nullptr;
    }

    return mash_virtual_base::create_subclass_by_enum(
        static_cast<mash::virtual_types_enum>(pc_type));
}

constexpr xbox_type_mapping XBOX_V14_MOCOMP_TYPES[] {
    {to_hash("als::null_mocomp"), 0x202},
    {to_hash("als::use_anim_only"), 0x20D},
    {to_hash("als::use_anim_only_with_invis"), 0x20E},
    {to_hash("als::simple_orientation"), 0x20A},
    {to_hash("als::simple_orient_with_playback_speed"), 0x208},
    {to_hash("als::simple_orient_with_speed_adjust"), 0x209},
    {to_hash("als::simple_orientation_ped"), 0x20B},
    {to_hash("als::strafe_mocomp"), 0x20C},
    {to_hash("als::orientated_react"), 0x204},
    {to_hash("als::crawl_orient"), 0x1F6},
    {to_hash("als::crawl_zip_mocomp"), 0x1F8},
    {to_hash("als::crawl_corner_mocomp"), 0x1F4},
    {to_hash("als::crawl_land_mocomp"), 0x1F5},
    {to_hash("als::crawl_corner_int90_mocomp"), 0x1F3},
    {to_hash("als::set_orient_mocomp"), 0x207},
    {to_hash("als::pole_swing_mocomp"), 0x205},
    {to_hash("als::bounce_mocomp"), 0x1EE},
    {to_hash("als::y_facing_fixup"), 0x211},
    {to_hash("als::fall_mocomp"), 0x1FB},
    {to_hash("als::jump_mocomp"), 0x1FE},
    {to_hash("als::feed_mocomp"), 0x201},
    {to_hash("als::webzip_mocomp"), 0x210},
    {to_hash("als::orient_adaptive_blend_xz"), 0x203},
    {to_hash("als::direct_mocomp"), 0x1F9},
    {to_hash("als::flight_mocomp"), 0x1FC},
    {to_hash("als::electro_flight_mocomp"), 0x1FA},
    {to_hash("als::beetle_flight_mocomp"), 0x1EC},
    {to_hash("als::chopper_flight_mocomp"), 0x1EF},
    {to_hash("als::johnny_storm_flight_mocomp"), 0x1FD},
    {to_hash("als::move_and_face"), 0x1FF},
    {to_hash("als::move_and_face_no_anim_movement"), 0x200},
    {to_hash("als::combat_move_and_face"), 0x1F0},
    {to_hash("als::combat_move_and_ignore_face"), 0x1F1},
    {to_hash("als::action_move_and_face"), 0x1EB},
    {to_hash("als::constant_move_and_face"), 0x1F2},
    {to_hash("als::begin_biped_physics"), 0x1ED},
    {to_hash("als::velocity_orientation"), 0x20F},
    {to_hash("als::crawl_transition"), 0x1F7},
    {to_hash("als::relative_orientation"), 0x206},
};
static_assert(sizeof(XBOX_V14_MOCOMP_TYPES) / sizeof(XBOX_V14_MOCOMP_TYPES[0]) == 39);

bool translate_mocomp_type(uint32_t xbox_type, uint32_t &pc_type)
{
#ifdef OPENUSM_XBPACK_V10
    if (xbox_type >= 0x1D5 && xbox_type <= 0x1E8) {
        pc_type = xbox_type + 0x15;
        return true;
    }

    if (xbox_type >= 0x1E9 && xbox_type <= 0x1FB) {
        pc_type = xbox_type + 0x16;
        return true;
    }

    return false;
#else
    return translate_type(xbox_type, XBOX_V14_MOCOMP_TYPES, pc_type);
#endif
}

extern "C" __attribute__((noinline, used)) void *__cdecl xbpack_create_mocomp_in_place(
    uint32_t xbox_type,
    mash_virtual_base *storage,
    int max_size)
{
    uint32_t pc_type = 0;
    if (!translate_mocomp_type(xbox_type, pc_type)) {
        report_unsupported_type("mocomp", xbox_type);
        return nullptr;
    }

    return mash_virtual_base::create_subclass_by_enum_in_place(
        static_cast<mash::virtual_types_enum>(pc_type), storage, max_size);
}

} // namespace
#endif

mash_virtual_base::mash_virtual_base() {}

void *mash_virtual_base::operator new(size_t sz)
{
    return mem_alloc(sz);
}

void mash_virtual_base::operator delete(void *ptr, size_t sz)
{
    mem_dealloc(ptr, sz);
}

void *mash_virtual_base::create_subclass_by_enum(mash::virtual_types_enum a1)
{
    TRACE("mash_virtual_base::create_subclass_by_enum");
    if constexpr (STANDALONE_SYSTEM)
        return create_native_mash_class(static_cast<uint32_t>(a1));
    else
        return reinterpret_cast<void *>(CDECL_CALL(0x0042AB60, a1));
}

void *mash_virtual_base::create_subclass_by_enum_in_place(
    mash::virtual_types_enum a1, mash_virtual_base *a2, int a3)
{
    TRACE("mash_virtual_base::create_subclass_by_enum_in_place");
    if constexpr (STANDALONE_SYSTEM)
        return create_native_mash_class(static_cast<uint32_t>(a1), a2, a3);
    else
        return reinterpret_cast<void *>(CDECL_CALL(0x004227E0, a1, a2, a3));
}

void mash_virtual_base::destruct_mashed_class()
{
    ;
}

void mash_virtual_base::_unmash(mash_info_struct *, void *) {}

void mash_virtual_base::unmash(mash_info_struct *a2, void *a3)
    {
        void (__fastcall *func)(void *, int, mash_info_struct *, void *) = CAST(func, get_vfunc(m_vtbl, 0x4));
        func(this, 0, a2, a3);
    }

uint32_t mash_virtual_base::_get_virtual_type_enum() const
{
    return 573;
}

uint32_t mash_virtual_base::get_virtual_type_enum() const
{
    uint32_t(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0xC));
    return func(this);
}

bool mash_virtual_base::is_subclass_of(mash::virtual_types_enum) const
{
    return false;
}

bool mash_virtual_base::_is_or_is_subclass_of(mash::virtual_types_enum a2) const
{
    if (this->get_virtual_type_enum() == a2) {
        return true;
    }
    bool (__fastcall *func)(const void *, void *, mash::virtual_types_enum) =
        CAST(func, get_vfunc(m_vtbl, 0x10));
    return func(this, nullptr, a2);
}

bool mash_virtual_base::is_or_is_subclass_of(mash::virtual_types_enum a2) const
{
    bool(__fastcall * func)(const void *, void *edx, mash::virtual_types_enum) = CAST(func, get_vfunc(m_vtbl, 0x14));
    return func(this, nullptr, a2);
}

void mash_virtual_base::generate_vtable()
{
    if constexpr (STANDALONE_SYSTEM) {
        std::fill_n(vtable(), 1014, nullptr);
        for (unsigned type = 0; type < 10; ++type)
            vtable()[type] = native_enx::vtable(type);
        vtable()[11] = native_mash_vtable<ai::ai_action_processor_inode>();
        vtable()[53] = native_mash_vtable<ai::nugget_wait_state>();
        vtable()[94] = native_mash_vtable<combo_system_move>();
        vtable()[95] = native_mash_vtable<combo_system_move::dialation_info>();
        vtable()[96] = native_mash_vtable<combo_system_move::link_info>();
        vtable()[97] = native_mash_vtable<combo_system_move::range_info>();
        vtable()[98] = native_mash_vtable<combo_system_move::requirements>();
        vtable()[99] = native_mash_vtable<combo_system_move::results>();
        vtable()[100] = native_mash_vtable<combo_system_move::target_info>();
        vtable()[101] = native_mash_vtable<combo_system_move::trigger_info>();
        vtable()[144] = anim_key::native_vtable();
        vtable()[145] = anim_record::native_vtable();
        vtable()[146] = paired_anim_record::native_vtable();
        vtable()[147] = enum_anim_key::native_vtable();
        vtable()[148] = ai::interaction_inode::native_vtable();
        vtable()[149] = als::meta_aimed_shot_vert::native_vtable();
        vtable()[150] = ai::meta_anim_strength_test::native_vtable();
        vtable()[152] = ai::strength_test_inode::native_vtable();
        vtable()[156] = ai::nonpath_loco_inode::native_vtable();
        vtable()[157] = ai::ped_avoidance_inode::native_vtable();
        vtable()[158] = ai::ped_combat_inode::native_vtable();
        vtable()[159] = ai::pedestrian_inode::native_vtable();
        vtable()[177] = ai::pedestrian_idle_state::native_vtable();
        vtable()[181] = plr_loco_crawl_state::native_vtable();
        vtable()[182] = plr_loco_crawl_transition_state::native_vtable();
        vtable()[248] = ai::spidey_combat_inode::native_vtable();
        vtable()[258] = ai::ai_car_inode::native_vtable();
        vtable()[259] = ai::drive_car_state::native_vtable();
        vtable()[285] = ai::targetable_inode_285::native_vtable();
        vtable()[303] = ai::jump_state::native_vtable();
        vtable()[304] = ai::pole_swing_inode::native_vtable();
        vtable()[315] = ai::std_puppet_inode::native_vtable();
        vtable()[316] = ai::std_puppet_trans_state::native_vtable();
        vtable()[317] = ai::run_state::native_vtable();
        vtable()[318] = ai::swing_inode::native_vtable();
        vtable()[319] = ai::swing_state::native_vtable();
        vtable()[323] = ai::hero_base_state::native_vtable();
        vtable()[324] = ai::spidey_base_state::native_vtable();
        vtable()[325] = ai::venom_base_state::native_vtable();
        vtable()[326] = ai::web_zip_inode::native_vtable();
        vtable()[327] = ai::web_zip_state::native_vtable();
        vtable()[330] = ai::launch_layer_state::native_vtable();
        vtable()[333] = ai::als_inode::native_vtable();
        vtable()[336] = ai::avoidance_inode::native_vtable();
        vtable()[338] = ai::biped_layer_inode::native_vtable();
        vtable()[341] = ai::quad_path_inode::native_vtable();
        vtable()[342] = ai::combat_inode::native_vtable();
        vtable()[343] = ai::damage_inode::native_vtable();
        vtable()[344] = ai::combat_inode::incoming_move::native_vtable();
        vtable()[346] = ai::player_combat_inode::native_vtable();
        vtable()[349] = ai::base_full_target_inode::native_vtable();
        vtable()[351] = ai::combat_target_inode::native_vtable();
        vtable()[356] = ai::venom_combat_target_inode::native_vtable();
        vtable()[357] = ai::cpu_controller_inode::native_vtable();
        vtable()[358] = ai::player_controller_inode::native_vtable();
        vtable()[371] = ai::std_default_trans_inode::native_vtable();
        vtable()[375] = ai::std_fear_inode::native_vtable();
        vtable()[383] = ai::glass_house_inode::native_vtable();
        vtable()[384] = ai::hero_inode::native_vtable();
        vtable()[391] = ai::loco_inode::native_vtable();
        vtable()[402] = ai::physics_inode::native_vtable();
        vtable()[403] = ai::prop_physics_inode::native_vtable();
        vtable()[408] = ai::slave_inode::native_vtable();
        vtable()[410] = ai::weapon_inode::native_vtable();
        vtable()[420] = ai::track_field_inode::native_vtable();
        vtable()[421] = ai::traffic_base_state::native_vtable();
        vtable()[422] = ai::traffic_inode::native_vtable();
        vtable()[456] = ai::voice_box_inode::native_vtable();
        vtable()[483] = native_mash_vtable<als::layer_state_machine_shared>();
        vtable()[484] = native_mash_vtable<als::state_machine_shared>();
        vtable()[488] = als::als_meta_anim_swing::native_vtable();
        vtable()[489] = als::als_meta_linear_blend::native_vtable();
        vtable()[490] = als::motion_compensator::native_vtable(490);
        vtable()[493] = native_mash_vtable<als::begin_biped_physics>();
        vtable()[503] = als::crawl_transition::native_vtable();
        vtable()[514] = als::motion_compensator::native_vtable(514);
        vtable()[522] = als::simple_orientation::native_vtable();
        vtable()[523] = als::simple_orientation_ped::native_vtable();
        vtable()[525] = als::use_anim_only::native_vtable();
        vtable()[530] = native_mash_vtable<als::base_layer_scripted_state>();
        vtable()[531] = als::scripted_category::native_vtable();
        vtable()[532] = native_mash_vtable<als::scripted_state>();
        vtable()[533] = native_mash_vtable<als::scripted_trans_group>();
        vtable()[535] = ai::enhanced_state::native_vtable();
        vtable()[536] = ai::signal_enhanced_state::native_vtable();
        vtable()[537] = ai::info_node::native_vtable();
        vtable()[541] = native_mash_vtable<PanelQuad>();
        vtable()[542] = native_mash_vtable<FEFloatingText>();
        vtable()[543] = native_mash_vtable<FEMultiLineText>();
        vtable()[544] = native_mash_vtable<FEText>();
        vtable()[545] = box_region::native_vtable();
        vtable()[546] = generic_interaction::native_vtable();
        vtable()[547] = interaction::native_vtable();
        vtable()[548] = named_trigger_box_region::native_vtable();
        vtable()[549] = point_dist_region::native_vtable();
        vtable()[561] = ai::controller_inode::native_vtable();

        vtable()[169] = ped_transition_vtable();
        vtable()[293] = hostage_transition_vtable();
        vtable()[370] = ai::std_default_state_set_base::native_vtable();
    } else {
        CDECL_CALL(0x00432B60);
    }
#ifdef TARGET_XBOX
    {
        auto *v1 = new PanelQuad{};
        map_vtable.insert_or_assign(to_hash("PanelQuad"), v1);
    }

    {
        auto *v1 = new FEText{};
        map_vtable.insert_or_assign(to_hash("FEText"), v1);
    }

    {
        auto *v1 = new FEMultiLineText{};
        map_vtable.insert_or_assign(to_hash("FEMultiLineText"), v1);
    }

    {
        auto *v1 = new FEFloatingText {};
        map_vtable.insert_or_assign(to_hash("FEFloatingText"), v1);
    }

    {
        auto *v1 = new als::state_machine_shared {};
        map_vtable.insert_or_assign(to_hash("als::state_machine_shared"), v1);
    }

    {
        auto *v1 = new als::layer_state_machine_shared {};
        map_vtable.insert_or_assign(to_hash("als::layer_state_machine_shared"), v1);
    }

    {
        auto *v1 = new als::scripted_state {};
        map_vtable.insert_or_assign(to_hash("als::scripted_state"), v1);
    }

    {
        auto *v1 = new als::base_layer_scripted_state {};
        map_vtable.insert_or_assign(to_hash("als::base_layer_scripted_state"), v1);
    }

    {
        auto *v1 = new als::scripted_category {};
        map_vtable.insert_or_assign(to_hash("als::scripted_category"), v1);
    }

    {
        auto *v1 = new als::scripted_trans_group{};
        map_vtable.insert_or_assign(to_hash("als::scripted_trans_group"), v1);
    }

    {
        auto *v1 = new ai::meta_anim_interact{};
        map_vtable.insert_or_assign(to_hash("als::meta_anim_interact"), v1);
    }

    {
        auto *v1 = new ai::meta_anim_strength_test{};
        map_vtable.insert_or_assign(to_hash("als::meta_anim_strength_test"), v1);
    }

    {
        auto *v1 = new als::als_meta_linear_blend{};
        map_vtable.insert_or_assign(to_hash("als::als_meta_linear_blend"), v1);
    }

    {
        auto *v1 = new als::als_meta_anim_swing{};
        map_vtable.insert_or_assign(to_hash("als::als_meta_anim_swing"), v1);
    }

    {
        auto *v1 = new ai::spidey_base_state {};
        map_vtable.insert_or_assign(to_hash("spidey_base_state"), v1);
    }

    {
        auto *v1 = new ai::std_puppet_trans_state {};
        map_vtable.insert_or_assign(to_hash("std_puppet_trans_state"), v1);
    }

    {
        auto *v1 = new anim_key {};
        map_vtable.insert_or_assign(to_hash("anim_key"), v1);
    }

    {
        auto *v1 = new anim_record {};
        map_vtable.insert_or_assign(to_hash("anim_record"), v1);
    }
#endif
}

void *mash_virtual_base::construct_class_helper(void *a1)
{
    auto *object = static_cast<mash_virtual_base *>(a1);
    const auto type = object->get_virtual_type_enum();
    if constexpr (STANDALONE_SYSTEM) {
        return create_subclass_by_enum_in_place(
            static_cast<mash::virtual_types_enum>(type),
            object,
            0x7FFFFFFF);
    } else {
        auto *v1 = static_cast<mash_virtual_base *>(a1);
        auto v2 = v1->get_virtual_type_enum();

        sp_log("mash::virtual_types_enum = %u", v2);
        return reinterpret_cast<void *>(CDECL_CALL(0x0042A7C0, a1));
    }
}

void mash_virtual_base::fixup_vtable(void *a1)
{
    TRACE("mash_virtual_base::fixup_vtable");

#if defined(OPENUSM_XBPACK_MODE) && !defined(TARGET_XBOX)
    const auto hash = static_cast<uint32_t *>(a1)[0];
    uint32_t pc_vtable = 0;

#ifdef OPENUSM_XBPACK_V10
    if (translate_v10_vtable(hash, pc_vtable)) {
        static_cast<uint32_t *>(a1)[0] = pc_vtable;
        return;
    }

    switch (hash) {
    case 0x56:
        pc_vtable = 0x008737C0;
        break;
    case 0x57:
        pc_vtable = 0x0087B8BC;
        break;
    case 0x58:
        pc_vtable = 0x00873734;
        break;
    case 0x59:
        pc_vtable = 0x008737A4;
        break;
    case 0x5A:
        pc_vtable = 0x00873788;
        break;
    case 0x5B:
        pc_vtable = 0x0087B8A0;
        break;
    case 0x5C:
        pc_vtable = 0x00879FC0;
        break;
    case 0x5D:
        pc_vtable = 0x0087376C;
        break;
    case 0x5E:
        pc_vtable = 0x00873750;
        break;
    case 0x87:
        pc_vtable = 0x008738E8;
        break;
    case 0x88:
        pc_vtable = 0x00873928;
        break;
    case 0x89:
        pc_vtable = 0x00873948;
        break;
    case 0x8A:
        pc_vtable = 0x00873908;
        break;
    case 0x8B:
        pc_vtable = 0x00875560;
        break;
    case 0x8C:
        pc_vtable = 0x0087559C;
        break;
    case 0x1FC:
        pc_vtable = 0x0087E214;
        break;
    case 0x1FD:
        pc_vtable = 0x0087E250;
        break;
    case 0x1FE:
        pc_vtable = 0x0087E1D8;
        break;
    case 0x1FF:
        pc_vtable = 0x0087E1B8;
        break;
    case 0x207:
        pc_vtable = 0x0087B990;
        break;
    case 0x208:
        pc_vtable = 0x0087A0F0;
        break;
    case 0x209:
        pc_vtable = 0x0087AE58;
        break;
    case 0x20A:
        pc_vtable = 0x00879FE0;
        break;
    default:
        if (hash > 0x23D || vtable()[hash] == nullptr) {
            report_unsupported_type(
                "vtable", hash, a1, __builtin_return_address(0));
            return;
        }
        pc_vtable = bit_cast<uint32_t>(vtable()[hash]);
        break;
    }
#else
    switch (hash) {
    case to_hash("PanelQuad"):
        pc_vtable = 0x0087B990;
        break;
    case to_hash("FEFloatingText"):
        pc_vtable = 0x0087A0F0;
        break;
    case to_hash("FEMultiLineText"):
        pc_vtable = 0x0087AE58;
        break;
    case to_hash("FEText"):
        pc_vtable = 0x00879FE0;
        break;
    case to_hash("als::state_machine_shared"):
        pc_vtable = 0x0087B8F8;
        break;
    case to_hash("als::layer_state_machine_shared"):
        pc_vtable = 0x0087E3A4;
        break;
    case to_hash("als::scripted_state"):
        pc_vtable = 0x0087E1D8;
        break;
    case to_hash("als::base_layer_scripted_state"):
        pc_vtable = 0x0087E214;
        break;
    case to_hash("als::scripted_category"):
        pc_vtable = 0x0087E250;
        break;
    case to_hash("als::scripted_trans_group"):
        pc_vtable = 0x0087E1B8;
        break;
    case to_hash("als::meta_anim_interact"):
        pc_vtable = 0x00875560;
        break;
    case to_hash("als::meta_anim_strength_test"):
        pc_vtable = 0x0087559C;
        break;
    case to_hash("als::als_meta_linear_blend"):
        pc_vtable = 0x0087B954;
        break;
    case to_hash("als::als_meta_anim_swing"):
        pc_vtable = 0x0087B918;
        break;
    case to_hash("spidey_base_state"):
        pc_vtable = 0x00877534;
        break;
    case to_hash("venom_base_state"):
        pc_vtable = 0x00877570;
        break;
    case to_hash("std_puppet_trans_state"):
        pc_vtable = 0x008771E0;
        break;
    case to_hash("anim_key"):
        pc_vtable = 0x008738E8;
        break;
    case to_hash("anim_record"):
        pc_vtable = 0x00873928;
        break;
    case to_hash("als_inode"):
        pc_vtable = 0x0087CEC0;
        break;
    case to_hash("hero_inode"):
        pc_vtable = 0x0087DAA4;
        break;
    case to_hash("std_puppet_inode"):
        pc_vtable = 0x0087DB04;
        break;
    case to_hash("player_combat_target_inode"):
        pc_vtable = 0x0087C4B0;
        break;
    case to_hash("venom_combat_target_inode"):
        pc_vtable = 0x0087C550;
        break;
    case to_hash("glass_house_inode"):
        pc_vtable = 0x0087DC44;
        break;
    case to_hash("web_zip_inode"):
        pc_vtable = 0x0087DB64;
        break;
    case to_hash("swing_inode"):
        pc_vtable = 0x0087DB34;
        break;
    case to_hash("pole_swing_inode"):
        pc_vtable = 0x0087DAD4;
        break;
    case to_hash("player_combat_inode"):
        pc_vtable = 0x0087BD00;
        break;
    case to_hash("spidey_combat_inode"):
        pc_vtable = 0x0087D700;
        break;
    case to_hash("combat_inode"):
        pc_vtable = 0x0087BBD0;
        break;
    case to_hash("combat_inode::incoming_move"):
        pc_vtable = 0x0087A1FC;
        break;
    case to_hash("std_default_trans_inode"):
        pc_vtable = 0x0087CC7C;
        break;
    case to_hash("interaction_inode"):
        pc_vtable = 0x0087CE10;
        break;
    case to_hash("interaction"):
        pc_vtable = 0x0087B8D8;
        break;
    case to_hash("voice_box_inode"):
        pc_vtable = 0x0087DDD8;
        break;
    case to_hash("physics_inode"):
        pc_vtable = 0x0087DC74;
        break;
    case to_hash("strength_test_inode"):
        pc_vtable = 0x0087CE40;
        break;
    case to_hash("player_controller_inode"):
        pc_vtable = 0x0087D370;
        break;
    case to_hash("controller_inode"):
        pc_vtable = 0x0087D310;
        break;
    case to_hash("track_field_inode"):
        pc_vtable = 0x0087DD68;
        break;
    case to_hash("damage_inode"):
        pc_vtable = 0x0087BB9C;
        break;
    case to_hash("ai_action_processor_inode"):
        pc_vtable = 0x0087BB6C;
        break;
    case to_hash("prop_physics_inode"):
        pc_vtable = 0x0087DCA4;
        break;
    case to_hash("combo_system_move"):
        pc_vtable = 0x0087B8BC;
        break;
    case to_hash("combo_system_move::results"):
        pc_vtable = 0x00879FC0;
        break;
    case to_hash("combo_system_move::dialation_info"):
        pc_vtable = 0x00873734;
        break;
    case to_hash("combo_system_move::requirements"):
        pc_vtable = 0x0087B8A0;
        break;
    case to_hash("combo_system_move::trigger_info"):
        pc_vtable = 0x00873750;
        break;
    case to_hash("combo_system_move::target_info"):
        pc_vtable = 0x0087376C;
        break;
    case to_hash("combo_system_move::range_info"):
        pc_vtable = 0x00873788;
        break;
    case to_hash("combo_system_move::link_info"):
        pc_vtable = 0x008737A4;
        break;
    default:
        sp_log("Unsupported Xbox mash vtable hash 0x%08X", hash);
        assert(0 && "Unsupported Xbox mash vtable hash");
        return;
    }
#endif

    static_cast<uint32_t *>(a1)[0] = pc_vtable;

#elif defined(TARGET_XBOX)
    const auto hash = static_cast<uint32_t *>(a1)[0];

    static_cast<uint32_t *>(a1)[0] = map_vtable.at(hash)->m_vtbl;

#else
    const auto idx = static_cast<uint32_t *>(a1)[0];
    if constexpr (STANDALONE_SYSTEM) {
        if (idx >= ORIGINAL_VTABLE_COUNT || vtable()[idx] == nullptr) {
            report_unsupported_standalone_vtable(idx, a1);
        }
    }
    static_cast<uint32_t *>(a1)[0] = bit_cast<uint32_t>(vtable()[idx]);
#endif

}

void mash_virtual_base_patch()
{
    REDIRECT(0x00555726, mash_virtual_base::generate_vtable);

    REDIRECT(0x004B157A, mash_virtual_base::construct_class_helper);

    return;
}

#if defined(OPENUSM_XBPACK_MODE) && !defined(TARGET_XBOX)
void mash_virtual_base_xbpack_patch()
{
    SET_JUMP(0x0041F820, mash_virtual_base::fixup_vtable);

    REDIRECT(0x00498F5B, xbpack_create_mocomp_in_place);

#ifndef OPENUSM_XBPACK_V10
    REDIRECT(0x005BF5A6, xbpack_create_subclass);
    REDIRECT(0x005D328C, xbpack_create_subclass);
    REDIRECT(0x00687FF8, xbpack_create_subclass);
    REDIRECT(0x0069BC15, xbpack_create_subclass);
    REDIRECT(0x006A15E4, xbpack_create_subclass);
    REDIRECT(0x006C4C80, xbpack_create_subclass);
    REDIRECT(0x006C8780, xbpack_create_subclass);
#endif
}
#endif
