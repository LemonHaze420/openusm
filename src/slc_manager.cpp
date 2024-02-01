#include "slc_manager.h"

#include "debugutil.h"
#include "func_wrapper.h"
#include "memory.h"
#include "mission_manager.h"
#include "mission_stack_manager.h"
#include "utility.h"
#include "open_city_neighborhoods.h"
#include "os_developer_options.h"
#include "resource_key.h"
#include "resource_manager.h"
#include "resource_pack_group.h"
#include "script_manager.h"

#include "script_lib_anim.h"
#include "script_lib_entity.h"

#include "script_library_class.h"
#include "trace.h"
#include "variables.h"
#include "vm_stack.h"
#include "vm_thread.h"

#if !SLC_MANAGER_STANDALONE
Var<_std::vector<script_library_class *> *> slc_manager_class_array{0x00965EC8};
#else

static std::vector<script_library_class *> *g_slc_manager_class_array {nullptr};
Var<std::vector<script_library_class *> *>
    slc_manager_class_array {(int) &g_slc_manager_class_array};

#endif

void register_standard_script_libs() {
    CDECL_CALL(0x005AB800);
}

void construct_client_script_libs() {
    CDECL_CALL(0x0058F9C0);
}

void destruct_client_script_libs()
{
    CDECL_CALL(0x0058FA50);
}



#define BUILD_SLC_NAME(type) slc_ ## type ## _t

#define DECLARE_SLC(type, base_type, vtbl)                          \
    struct BUILD_SLC_NAME(type) : script_library_class {          \
        BUILD_SLC_NAME(type) ()                                   \
            : script_library_class(#type, 4u, #base_type, false) {  \
                m_vtbl = vtbl;                                      \
            }                                                       \
    }

DECLARE_SLC(pfx, "\0", 0x0089C878);

DECLARE_SLC(interactable_interface, "\0", 0x0089B5A8);

DECLARE_SLC(beam, entity, 0x0089AAF8);

DECLARE_SLC(cut_scene, "\0", 0x0089B7A8);

DECLARE_SLC(debug_menu_entry, "\0", 0x0089C6FC);

DECLARE_SLC(entity_list, "\0", 0x0089AA70);

DECLARE_SLC(entity_list_iterator, "\0", 0x0089AA78);

DECLARE_SLC(entity_tracker, "\0", 0x0089ABC4);

DECLARE_SLC(glamour_cam, "\0", 0x0089B778);

DECLARE_SLC(gun, item, 0x0089B57C);

DECLARE_SLC(item, entity, 0x0089AA44);

DECLARE_SLC(line_info, "\0", 0x0089B6FC);

DECLARE_SLC(mission_camera_marker, "\0", 0x0089C310);

DECLARE_SLC(mission_camera_transform_marker, "\0", 0x0089C328);

DECLARE_SLC(mission_transform_marker, "\0", 0x0089C31C);

DECLARE_SLC(num_list, "\0", 0x0089AA60);

DECLARE_SLC(num_list_iterator, "\0", 0x0089AA68);

DECLARE_SLC(panel, "\0", 0x0089B770);

DECLARE_SLC(polytube, entity, 0x0089C0EC);

DECLARE_SLC(script_controller, signaller, 0x0089BCCC);

DECLARE_SLC(signaller, "\0", 0x0089AAA8);

DECLARE_SLC(sound_inst, "\0", 0x0089B1EC);

DECLARE_SLC(str_list, "\0", 0x0089AA80);

DECLARE_SLC(str_list_iterator, "\0", 0x0089AA88);

DECLARE_SLC(switch_obj, entity, 0x0089B8D0);

DECLARE_SLC(tam, "\0", 0x0089C6C4);

DECLARE_SLC(taunt_entry, "\0", 0x0089BEBC);

DECLARE_SLC(taunt_exchange, "\0", 0x0089BEC4);

DECLARE_SLC(taunt_exchange_list, "\0", 0x0089AA90);

DECLARE_SLC(trigger, signaller, 0x0089B914);

DECLARE_SLC(vector3d, "\0", 0x0089A4CC);

DECLARE_SLC(position3d, vector3d, 0x0089A4D8);

DECLARE_SLC(posfacing3d, "\0", 0x0089A4E4);

DECLARE_SLC(vector3d_list, "\0", 0x0089AA50);

DECLARE_SLC(vector3d_list_iterator, "\0", 0x0089AA58);

DECLARE_SLC(string_hash, "\0", 0x0089A4F0);

DECLARE_SLC(critical_section, "\0", 0x0089C7DC);

DECLARE_SLC(district, signaller, 0x0089A4FC);

#undef DECLARE_SLC
#undef BUILD_SLC_NAME

#define BUILD_SLF_NAME(_KLASS, _TYPE) slf__ ## _KLASS ## __ ## _TYPE ## __t

#define BUILD_GLOBAL_SLF_NAME(type) slf__ ## type ## __t

#define DECLARE_GLOBAL_SLF_BEGIN(NAME, VTBL) \
    struct BUILD_GLOBAL_SLF_NAME(NAME) : script_library_class::function { \
        BUILD_GLOBAL_SLF_NAME(NAME) (const char *a3) : function(a3)  { \
            m_vtbl = VTBL; \
        }   \
            \
        bool operator()(vm_stack &stack, script_library_class::function::entry_t entry) const

#define DECLARE_GLOBAL_SLF_END() \
    };

DECLARE_GLOBAL_SLF_BEGIN(abs_delay__num, 0x0089A724)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(acos__num, 0x0089A91C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_2d_debug_str__vector3d__vector3d__num__str, 0x0089A860)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_2d_debug_str__vector3d__vector3d__num__str__num, 0x0089A858)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_3d_debug_str__vector3d__vector3d__num__str, 0x0089A850)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_civilian_info__vector3d__num__num__num, 0x0089C5BC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_civilian_info_entity__entity__num__num__num, 0x0089C5CC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_debug_cyl__vector3d__vector3d__num, 0x0089A774)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_debug_cyl__vector3d__vector3d__num__vector3d__num, 0x0089A77C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_debug_line__vector3d__vector3d, 0x0089A764)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_debug_line__vector3d__vector3d__vector3d__num, 0x0089A76C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_debug_sphere__vector3d__num, 0x0089A754)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_debug_sphere__vector3d__num__vector3d__num, 0x0089A75C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_glass_house__str, 0x0089A548)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_glass_house__str__num, 0x0089A550)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_glass_house__str__num__vector3d, 0x0089A560)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_glass_house__str__vector3d, 0x0089A558)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_to_console__str, 0x0089A834)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(add_traffic_model__num__str, 0x0089C5A4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(allow_suspend_thread__num, 0x0089A594)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(angle_between__vector3d__vector3d, 0x0089BA50)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(apply_donut_damage__vector3d__num__num__num__num__num, 0x0089A804)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(apply_radius_damage__vector3d__num__num__num__num, 0x0089A7FC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(apply_radius_subdue__vector3d__num__num__num__num, 0x0089A80C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(assert__num__str, 0x0089A518)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(attach_decal__str__vector3d__num__vector3d__entity, 0x0089A9FC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(begin_screen_recording__str__num, 0x0089B7B0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(blackscreen_off__num, 0x0089BCAC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(blackscreen_on__num, 0x0089BCA0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(bring_up_dialog_box__num__num, 0x0089BC28)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(bring_up_dialog_box_debug__str__num__str, 0x0089BC38)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(bring_up_dialog_box_title__num__num__num, 0x0089BC30)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(bring_up_medal_award_box__num, 0x0089BB10)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(bring_up_race_announcer, 0x0089BB08)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(calc_launch_vector__vector3d__vector3d__num__entity, 0x0089A98C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(can_load_pack__str, 0x0089C3F4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(chase_cam, 0x0089AF04)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(clear_all_grenades, 0x0089A95C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(clear_civilians_within_radius__vector3d__num, 0x0089C5E4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(clear_controls, 0x0089BD48)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(clear_debug_all, 0x0089A79C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(clear_debug_cyls, 0x0089A794)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(clear_debug_lines, 0x0089A78C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(clear_debug_spheres, 0x0089A784)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(clear_screen, 0x0089A944)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(clear_traffic_within_radius__vector3d__num, 0x0089C5DC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(col_check__vector3d__vector3d__num, 0x0089A868)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(console_exec__str, 0x0089A83C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(copy_vector3d_list__vector3d_list__vector3d_list, 0x0089BED4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(cos__num, 0x0089A90C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_beam, 0x0089ABB4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_credits, 0x0089BAE8)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_cut_scene__str, 0x0089B7C0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_debug_menu_entry__str, 0x0089C704)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_debug_menu_entry__str__str, 0x0089C70C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_decal__str__vector3d__num__vector3d, 0x0089A9F4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_entity__str, 0x0089AF0C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_entity__str__str, 0x0089AF14)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_entity_in_hero_region__str, 0x0089AF2C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_entity_list, 0x0089BFCC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_entity_tracker__entity, 0x0089C594)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_item__str, 0x0089B6B4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_line_info__vector3d__vector3d, 0x0089B708)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_lofi_stereo_sound_inst__str, 0x0089B818)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_num_list, 0x0089BF54)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_pfx__str, 0x0089C904)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_pfx__str__vector3d, 0x0089C90C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_polytube, 0x0089C228)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_polytube__str, 0x0089C230)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_progression_menu_entry__str__str, 0x0089C714)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_sound_inst, 0x0089B7F8)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_sound_inst__str, 0x0089B800)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_stompable_music_sound_inst__str, 0x0089B808)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_str_list, 0x0089C044)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_taunt_entry__entity__str__num, 0x0089C63C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_taunt_exchange__entity__entity__num__num__num__num, 0x0089C6B4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_taunt_exchange_list, 0x0089C0DC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_threat_assessment_meter, 0x0089C6CC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_time_limited_entity__str__num, 0x0089AF3C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_trigger__entity__num, 0x0089B970)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_trigger__str__vector3d__num, 0x0089B968)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_trigger__vector3d__num, 0x0089B960)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_unstompable_script_cutscene_sound_inst__str, 0x0089B810)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(create_vector3d_list, 0x0089BECC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(cross__vector3d__vector3d, 0x0089BA38)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(debug_breakpoint, 0x0089A510)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(debug_print__num__str, 0x0089A528)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(debug_print__num__vector3d__str, 0x0089A530)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(debug_print__str, 0x0089A520)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(debug_print_set_background_color__vector3d, 0x0089A538)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(delay__num, 0x0089A71C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(destroy_credits, 0x0089BAF0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(destroy_debug_menu_entry__debug_menu_entry, 0x0089C71C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(destroy_entity__entity, 0x0089AF34)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(destroy_entity_list__entity_list, 0x0089BFD4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(destroy_entity_tracker__entity_tracker, 0x0089C59C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(destroy_line_info__line_info, 0x0089B710)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(destroy_num_list__num_list, 0x0089BF5C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(destroy_pfx__pfx, 0x0089C914)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(destroy_str_list__str_list, 0x0089C04C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(destroy_taunt_entry__taunt_entry, 0x0089C644)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(destroy_taunt_exchange__taunt_exchange, 0x0089C6BC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(destroy_taunt_exchange_list__taunt_exchange_list, 0x0089C0E4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(destroy_threat_assessment_meter__tam, 0x0089C6D4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(destroy_trigger__trigger, 0x0089B978)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(destroy_vector3d_list__vector3d_list, 0x0089BEDC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(dilated_delay__num, 0x0089A72C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(disable_marky_cam__num, 0x0089A5F4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(disable_nearby_occlusion_only_obb__vector3d, 0x0089A5E4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(disable_player_shadows, 0x0089A614)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(disable_subtitles, 0x0089A93C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(disable_vibrator, 0x0089A7DC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(disable_zoom_map__num, 0x0089BBF0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(distance3d__vector3d__vector3d, 0x0089BA48)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(distance_chase_widget_set_pos__num, 0x0089BB88)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(distance_chase_widget_turn_off, 0x0089BB80)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(distance_chase_widget_turn_on__num__num, 0x0089BB78)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(distance_race_widget_set_boss_pos__num, 0x0089BBA8)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(distance_race_widget_set_hero_pos__num, 0x0089BBA0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(distance_race_widget_set_types__num__num, 0x0089BBB0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(distance_race_widget_turn_off, 0x0089BB98)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(distance_race_widget_turn_on, 0x0089BB90)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(district_id__str, 0x0089C47C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(district_name__num, 0x0089C484)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(dot__vector3d__vector3d, 0x0089BA30)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(dump_searchable_region_list__str, 0x0089A994)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_ai__num, 0x0089A6CC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_civilians__num, 0x0089C5EC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_controls__num, 0x0089BD40)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_entity_fading__num, 0x0089B05C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_interface__num, 0x0089A6C4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_marky_cam__num, 0x0089A5BC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_mini_map__num, 0x0089BBE8)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_nearby_occlusion_only_obb__vector3d, 0x0089A5DC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_obb__vector3d__num, 0x0089A588)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_pause__num, 0x0089A6AC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_physics__num, 0x0089A6DC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_player_shadows, 0x0089A61C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_pois__num, 0x0089A6EC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_quad_path_connector__district__num__district__num__num, 0x0089A578)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_subtitles, 0x0089A934)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_tokens_of_type__num__num, 0x0089B54C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_traffic__num, 0x0089C5FC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_user_camera__num, 0x0089A5D4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(enable_vibrator, 0x0089A7E4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(end_current_patrol, 0x0089C4FC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(end_cut_scenes, 0x0089B7C8)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(end_screen_recording, 0x0089B7B8)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(entity_col_check__entity__entity, 0x0089A88C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(entity_exists__str, 0x0089AF24)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(entity_get_entity_tracker__entity, 0x0089B034)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(entity_has_entity_tracker__entity, 0x0089B02C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(exit_water__entity, 0x0089A604)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(find_closest_point_on_a_path_to_point__vector3d, 0x0089A570)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(find_district_for_point__vector3d, 0x0089A81C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(find_entities_in_radius__entity_list__vector3d__num__num, 0x0089B46C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(find_entity__str, 0x0089AF1C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(find_innermost_district__vector3d, 0x0089A824)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(find_outermost_district__vector3d, 0x0089A82C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(find_trigger__entity, 0x0089B950)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(find_trigger__str, 0x0089B948)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(find_trigger_in_district__district__str, 0x0089B958)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(float_random__num, 0x0089A74C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(force_mission__district__str__num, 0x0089C3B4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(force_streamer_refresh, 0x0089C4AC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(format_time_string__num, 0x0089BC74)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(freeze_hero__num, 0x0089A5FC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(game_ini_get_flag__str, 0x0089A97C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(game_time_advance__num__num, 0x0089C4DC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_all_execs_thread_count__str, 0x0089A9EC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_all_instances_thread_count__str, 0x0089A9E4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_attacker_entity, 0x0089AA24)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_attacker_member, 0x0089AA2C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_available_stack_size, 0x0089C4EC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_character_packname_list, 0x0089C354)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_closest_point_on_lane_with_facing__num__vector3d__vector3d_list, 0x0089C61C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_col_hit_ent, 0x0089A884)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_col_hit_norm, 0x0089A87C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_col_hit_pos, 0x0089A874)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_control_state__num, 0x0089A7F4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_control_trigger__num, 0x0089A7EC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_current_instance_thread_count__str, 0x0089A9DC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_current_view_cam_pos, 0x0089A5B4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_current_view_cam_x_facing, 0x0089A59C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_current_view_cam_y_facing, 0x0089A5A4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_current_view_cam_z_facing, 0x0089A5AC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_fog_color, 0x0089A8F4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_fog_distance, 0x0089A8FC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_game_info_num__str, 0x0089A8C4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_game_info_str__str, 0x0089A8D4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_glam_cam__num, 0x0089B780)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_global_time_dilation, 0x0089A894)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()


struct slf__get_ini_flag__str__t : script_library_class::function {
    slf__get_ini_flag__str__t(const char *a3) : function(a3) {
        m_vtbl = 0x0089A94C;
    }   

    struct parms_t {
        const char *str;
    };
        
    bool operator()(vm_stack &stack, script_library_class::function::entry_t entry) const
    {
        TRACE("slf__get_ini_flag__str__t::operator()");

        (void)entry;

        SLF_PARMS;
        mString v5 {parms->str};
        auto result = (float) os_developer_options::instance()->get_flag(v5);

        SLF_RETURN;

        SLF_DONE;
    }
};

DECLARE_GLOBAL_SLF_BEGIN(get_ini_num__str, 0x0089A954)
{
    (void) entry;
    TRACE("slf__get_ini_num__str__t::operator()");

    auto *v3 = stack.SP - 4;
    stack.SP = v3;
    mString v5 {*(const char **)v3};
    auto v7 = (float)os_developer_options::instance()->get_int(v5);
    *(float *)stack.SP = v7;
    stack.SP += 4;
    return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_int_num__num, 0x0089A92C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_mission_camera_marker__num, 0x0089C414)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_mission_camera_transform_marker__num, 0x0089C454)
{
    (void) stack;
    (void) entry;
    return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_mission_entity, 0x0089C38C)
{
    (void) entry;
    TRACE("slf__get_mission_entity__t::operator()");

    *(DWORD *)stack.SP = (int)mission_manager::s_inst()->get_mission_key_entity();
    stack.SP += 4;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_mission_key_posfacing3d, 0x0089C36C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_mission_key_position, 0x0089C364)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_mission_marker__num, 0x0089C40C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_mission_nums, 0x0089C3AC)
{
    (void) entry;

    *(DWORD *)stack.SP = (int)mission_manager::s_inst()->get_mission_nums();
    stack.SP += 4;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_mission_patrol_waypoint, 0x0089C384)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_mission_positions, 0x0089C39C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_mission_strings, 0x0089C3A4)
{
    (void) entry;

    *(DWORD *)stack.SP = (int)mission_manager::s_inst()->get_mission_strings();
    stack.SP += 4;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_mission_transform_marker__num, 0x0089C42C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_mission_trigger, 0x0089C394)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_missions_key_position_by_index__district__str__num, 0x0089C3BC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_missions_nums_by_index__district__str__num__num_list, 0x0089C3CC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_missions_patrol_waypoint_by_index__district__str__num, 0x0089C3C4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_neighborhood_name__num, 0x0089C54C)
{
    (void) entry;

    auto *v2 = stack.SP - 4;
    stack.SP = v2;
    *(DWORD *)stack.SP = (int)get_neighborhood_name((neighborhood_e)*(float *)v2);
    stack.SP += 4;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_num_free_slots__str, 0x0089C3E4)
{
    (void) entry;

    auto *v3 = stack.SP - 4;
    stack.SP = v3;
    auto **v4 = (const char **)v3;
    if ( mission_stack_manager::s_inst()->waiting_for_push_or_pop() ) {
        return false;
    }

    mString v7 {*v4};
    auto *pack_group = mission_stack_manager::s_inst()->get_pack_group(v7);
    auto num_free_slots = 0.0f;
    if ( pack_group ) {
        num_free_slots = (float)pack_group->get_num_free_slots();
    }

    *(float *)stack.SP = num_free_slots;
    stack.SP += 4;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_num_mission_transform_marker, 0x0089C434)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_pack_group__str, 0x0089C3EC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_pack_size__str, 0x0089C4E4)
{
    (void) entry;

    auto *v3 = stack.SP - 4;
    stack.SP = v3;
    auto the_key = create_resource_key_from_path(*(const char **)v3, RESOURCE_KEY_TYPE_PACK);
    resource_pack_location a2 {};
    if (!resource_manager::get_pack_file_stats(the_key, &a2, nullptr, nullptr)) {
        mString v7 {*(const char **)v3};
        auto v5 = "Packfile " + v7 + " not found in amalgapak. Perhaps you need to repack it?";
        auto *t = stack.get_thread();
        t->slf_error(v5);
    }

    auto size = (double)a2.loc.m_size;
    if ( a2.loc.m_size < 0 ) {
        size += 4.2949673e9;
    }

    *(float *)stack.SP = size;
    stack.SP += 4;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_patrol_difficulty__str, 0x0089C534)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_patrol_node_position_by_index__str__num, 0x0089C52C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_patrol_start_position__str, 0x0089C524)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_patrol_unlock_threshold__str, 0x0089C53C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_platform, 0x0089A508)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_render_opt_num__str, 0x0089A8E4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_spider_reflexes_spiderman_time_dilation, 0x0089CAC4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_spider_reflexes_world_time_dilation, 0x0089CAD4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_time_inc, 0x0089A7A4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_time_of_day, 0x0089A974)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_time_of_day_rate, 0x0089A96C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_token_index_from_id__num__num, 0x0089B554)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(get_traffic_spawn_point_near_camera__vector3d_list, 0x0089AA98)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(greater_than_or_equal_rounded__num__num, 0x0089BC90)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(hard_break, 0x0089AA1C)
{
    (void) stack;
    (void) entry;

    debug_print_va("Debug break requested by script.");
    __debugbreak();
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(has_substring__str__str, 0x0089A580)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(hero, 0x0089AED4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(hero_exists, 0x0089AEDC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(hero_type, 0x0089AEE4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(hide_controller_gauge, 0x0089BB20)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(initialize_encounter_object, 0x0089AA0C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(initialize_encounter_objects, 0x0089AA04)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(insert_pack__str, 0x0089C3D4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(invoke_pause_menu_unlockables, 0x0089BC98)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_ai_enabled, 0x0089A6D4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_cut_scene_playing, 0x0089B7D0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_district_loaded__num, 0x0089C49C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_hero_frozen, 0x0089A60C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_hero_peter_parker, 0x0089AEFC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_hero_spidey, 0x0089AEEC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_hero_venom, 0x0089AEF4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_marky_cam_enabled, 0x0089A5C4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_mission_active, 0x0089C50C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_mission_loading, 0x0089C514)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_pack_available__str, 0x0089C404)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_pack_loaded__str, 0x0089C3FC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_pack_pushed__str, 0x0089C34C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_path_graph_inside_glass_house__str, 0x0089AAA0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_patrol_active, 0x0089C504)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_patrol_node_empty__num, 0x0089C544)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_paused, 0x0089A6B4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_physics_enabled, 0x0089A6E4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_point_inside_glass_house__vector3d, 0x0089A540)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_point_under_water__vector3d, 0x0089AA3C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(is_user_camera_enabled, 0x0089A5CC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(load_anim__str, 0x0089AAF0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(load_level__str__vector3d, 0x0089A8AC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(lock_all_districts, 0x0089C4C4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(lock_district__num, 0x0089C494)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(lock_mission_manager__num, 0x0089C51C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(los_check__vector3d__vector3d, 0x0089A814)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(lower_hotpursuit_indicator_level, 0x0089BAE0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(malor__vector3d__num, 0x0089A984)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(normal__vector3d, 0x0089BA40)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(pause_game__num, 0x0089A6BC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(play_credits, 0x0089BAF8)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(play_prerender__str, 0x0089A8B4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(pop_pack__str, 0x0089C344)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(post_message__str__num, 0x0089A73C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(pre_roll_all_pfx__num, 0x0089AA34)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(press_controller_gauge__num, 0x0089BB28)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(press_controller_gauge__num__num__num, 0x0089BB30)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(purge_district__num, 0x0089C4BC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(push_pack__str, 0x0089C334)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(push_pack_into_district_slot__str, 0x0089C33C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(raise_hotpursuit_indicator_level, 0x0089BAD8)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(random__num, 0x0089A744)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(remove_civilian_info__num, 0x0089C5C4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(remove_civilian_info_entity__entity__num, 0x0089C5D4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(remove_glass_house__str, 0x0089A568)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(remove_item_entity_from_world__entity, 0x0089AFFC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(remove_pack__str, 0x0089C3DC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(remove_traffic_model__num, 0x0089C5AC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(reset_externed_alses, 0x0089B064)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_all_anchors_activated__num, 0x0089CAEC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_blur__num, 0x0089A63C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_blur_blend_mode__num, 0x0089A664)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_blur_color__vector3d, 0x0089A644)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_blur_offset__num__num, 0x0089A654)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_blur_rot__num, 0x0089A65C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_blur_scale__num__num, 0x0089A64C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_clear_color__vector3d, 0x0089A6F4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_current_mission_objective_caption__num, 0x0089CADC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_default_traffic_hitpoints__num, 0x0089C614)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_dialog_box_flavor__num, 0x0089BC5C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_dialog_box_lockout_time__num, 0x0089BC6C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_engine_property__str__num, 0x0089A99C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_fov__num, 0x0089A62C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_game_info_num__str__num, 0x0089A8BC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_game_info_str__str__str, 0x0089A8CC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_global_time_dilation__num, 0x0089A89C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_marky_cam_lookat__vector3d, 0x0089A5EC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_max_streaming_distance__num, 0x0089C4B4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_mission_key_pos_facing__vector3d__vector3d, 0x0089C37C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_mission_key_position__vector3d, 0x0089C374)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_mission_text__num, 0x0089BBF8)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_mission_text_box_flavor__num, 0x0089BC64)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_mission_text_debug__str, 0x0089BC00)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_parking_density__num, 0x0089C60C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_pedestrian_density__num, 0x0089C5F4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_render_opt_num__str__num, 0x0089A8DC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_score_widget_score__num, 0x0089BAC8)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_sound_category_volume__num__num__num, 0x0089A9D4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_spider_reflexes_blur__num, 0x0089A674)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_spider_reflexes_blur_blend_mode__num, 0x0089A69C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_spider_reflexes_blur_color__vector3d, 0x0089A67C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_spider_reflexes_blur_offset__num__num, 0x0089A68C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_spider_reflexes_blur_rot__num, 0x0089A694)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_spider_reflexes_blur_scale__num__num, 0x0089A684)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_spider_reflexes_hero_meter_depletion_rate__num, 0x0089CAB4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_spider_reflexes_spiderman_time_dilation__num, 0x0089CABC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_spider_reflexes_world_time_dilation__num, 0x0089CACC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_state_of_the_story_caption__num, 0x0089CAE4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_target_info__entity__vector3d__vector3d, 0x0089B6C4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_time_of_day__num, 0x0089A964)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_traffic_density__num, 0x0089C604)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_traffic_model_usage__num__num, 0x0089C5B4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_vibration_resume__num, 0x0089A7D4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_whoosh_interp_rate__num, 0x0089B504)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_whoosh_pitch_range__num__num, 0x0089B4FC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_whoosh_speed_range__num__num, 0x0089B4EC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_whoosh_volume_range__num__num, 0x0089B4F4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(set_zoom__num, 0x0089A624)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(show_controller_gauge, 0x0089BB18)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(show_hotpursuit_indicator__num, 0x0089BAD0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(show_score_widget__num, 0x0089BAC0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(shut_up_all_ai_voice_boxes, 0x0089B50C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(sin__num, 0x0089A904)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(sin_cos__num, 0x0089A924)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(soft_load__num, 0x0089BCBC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(soft_save__num, 0x0089BCB4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_add_hero_points__num, 0x0089CAA4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_bank_stylepoints, 0x0089C91C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_break_web, 0x0089C9E4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_camera_add_shake__num__num__num, 0x0089CB34)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_camera_autocorrect__num, 0x0089C924)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_camera_clear_fixedstatic, 0x0089CAFC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_camera_enable_combat__num, 0x0089CB24)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_camera_enable_lookaround__num, 0x0089CB1C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_camera_set_fixedstatic__vector3d__vector3d, 0x0089CAF4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_camera_set_follow__entity, 0x0089CB2C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_camera_set_hero_underwater__num, 0x0089CB3C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_camera_set_interpolation_time__num, 0x0089CB14)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_camera_set_lockon_min_distance__num, 0x0089CB04)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_camera_set_lockon_y_offset__num, 0x0089CB0C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_charged_jump, 0x0089C98C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_enable_control_button__num__num, 0x0089CA94)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_enable_lockon__num, 0x0089C9AC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_engage_lockon__num, 0x0089C9B4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_engage_lockon__num__entity, 0x0089C9BC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_get_hero_points, 0x0089CA9C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_get_max_zip_length, 0x0089C9DC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_get_spidey_sense_level, 0x0089C99C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_is_crawling, 0x0089C934)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_is_falling, 0x0089C964)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_is_jumping, 0x0089C96C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_is_on_ceiling, 0x0089C944)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_is_on_ground, 0x0089C94C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_is_on_wall, 0x0089C93C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_is_running, 0x0089C95C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_is_sprint_crawling, 0x0089C984)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_is_sprinting, 0x0089C974)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_is_swinging, 0x0089C954)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_is_wallsprinting, 0x0089C97C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_lock_spider_reflexes_off, 0x0089CA24)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_lock_spider_reflexes_on, 0x0089CA1C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_lockon_camera_engaged, 0x0089CA0C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_lockon_mode_engaged, 0x0089CA04)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_set_camera_target__entity, 0x0089CA14)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_set_desired_mode__num__vector3d__vector3d, 0x0089C9EC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_set_health_beep_min_max_cooldown_time__num__num, 0x0089C9F4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_set_health_beep_threshold__num, 0x0089C9FC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_set_hero_meter_empty_rate__num, 0x0089CB44)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_set_max_height__num, 0x0089C9CC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_set_max_zip_length__num, 0x0089C9D4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_set_min_height__num, 0x0089C9C4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_set_spidey_sense_level__num, 0x0089C994)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_set_swing_anchor_max_sticky_time__num, 0x0089C9A4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_subtract_hero_points__num, 0x0089CAAC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_td_set_alternating_wall_run_occurrence_threshold__num, 0x0089CA74)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_td_set_alternating_wall_run_time_threshold__num, 0x0089CA6C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_td_set_big_air_height_threshold__num, 0x0089CA34)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_td_set_continuous_air_swings_threshold__num, 0x0089CA4C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_td_set_gain_altitude_height_threshold__num, 0x0089CA54)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_td_set_near_miss_trigger_radius__num, 0x0089CA84)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_td_set_near_miss_velocity_threshold__num, 0x0089CA8C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_td_set_orbit_min_radius_threshold__num, 0x0089CA3C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_td_set_soft_landing_velocity_threshold__num, 0x0089CA5C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_td_set_super_speed_speed_threshold__num, 0x0089CA7C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_td_set_swinging_wall_run_time_threshold__num, 0x0089CA64)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_td_set_wall_sprint_time_threshold__num, 0x0089CA44)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_unlock_spider_reflexes, 0x0089CA2C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spiderman_wait_add_threat__entity__str__num__num, 0x0089CB4C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(spidey_can_see__vector3d, 0x0089C92C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(sqrt__num, 0x0089A914)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(start_patrol__str, 0x0089C4F4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(stop_all_sounds, 0x0089B514)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(stop_credits, 0x0089BB00)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(stop_vibration, 0x0089A7CC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(subtitle__num__num__num__num__num__num, 0x0089BCC4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(swap_hero_costume__str, 0x0089C4D4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(text_width__str, 0x0089A7AC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(timer_widget_get_count_up, 0x0089BB70)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(timer_widget_get_time, 0x0089BB60)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(timer_widget_set_count_up__num, 0x0089BB68)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(timer_widget_set_time__num, 0x0089BB58)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(timer_widget_start, 0x0089BB48)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(timer_widget_stop, 0x0089BB50)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(timer_widget_turn_off, 0x0089BB40)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(timer_widget_turn_on, 0x0089BB38)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(to_beam__entity, 0x0089ABBC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(to_gun__entity, 0x0089B5A0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(to_item__entity, 0x0089B6BC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(to_polytube__entity, 0x0089C238)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(to_switch__entity, 0x0089B8E4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(trace__str, 0x0089AA14)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(trigger_is_valid__trigger, 0x0089B9B8)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(turn_off_boss_health, 0x0089BBD0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(turn_off_hero_health, 0x0089BBD8)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(turn_off_mission_text, 0x0089BC20)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(turn_off_third_party_health, 0x0089BBE0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(turn_on_boss_health__num__entity, 0x0089BBB8)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(turn_on_hero_health__num__entity, 0x0089BBC0)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(turn_on_third_party_health__num__entity, 0x0089BBC8)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(unload_script, 0x0089C35C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(unlock_all_exterior_districts, 0x0089C4CC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(unlock_district__num, 0x0089C48C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(vibrate_controller__num, 0x0089A7C4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(vibrate_controller__num__num, 0x0089A7BC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(vibrate_controller__num__num__num__num__num__num, 0x0089A7B4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(vo_delay__num__num__num__num, 0x0089A734)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(wait_animate_fog_color__vector3d__num, 0x0089A6FC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(wait_animate_fog_distance__num__num, 0x0089A704)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(wait_animate_fog_distances__num__num__num, 0x0089A70C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(wait_change_blur__num__vector3d__num__num__num__num__num__num, 0x0089A66C)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(wait_change_spider_reflexes_blur__num__vector3d__num__num__num__num__num__num, 0x0089A6A4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(wait_for_streamer_to_reach_equilibrium, 0x0089C4A4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(wait_fps_test__num__num__vector3d__vector3d, 0x0089A8EC)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(wait_frame, 0x0089A714)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(wait_set_global_time_dilation__num__num, 0x0089A8A4)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(wait_set_zoom__num__num, 0x0089A634)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

DECLARE_GLOBAL_SLF_BEGIN(write_to_file__str__str, 0x0089A844)
{
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_GLOBAL_SLF_END()

#undef DECLARE_GLOBAL_SLF_BEGIN
#undef DECLARE_GLOBAL_SLF_END

#define DECLARE_SLF_BEGIN(KLASS, NAME, VTBL) \
    struct BUILD_SLF_NAME(KLASS, NAME) : script_library_class::function { \
        BUILD_SLF_NAME(KLASS, NAME) (script_library_class *slc, const char *a3) : function(slc, a3)  { \
            m_vtbl = VTBL; \
        } \
    \
        bool operator()(vm_stack &stack, script_library_class::function::entry_t entry) const

#define DECLARE_SLF_END() \
    };


DECLARE_SLF_BEGIN(beam, add_alpha_effect__num__num__num__num__num__num, 0x0089AB9C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, add_color_effect__num__num__num__num__num__num__num__num__num__num, 0x0089AB8C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, add_width_effect__num__num__num__num__num__num, 0x0089AB94){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, get_impact_normal, 0x0089AB1C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, get_impact_point, 0x0089AB14){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, hit_hero, 0x0089AB0C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, hit_world, 0x0089AB04){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, kill_all_effects__num, 0x0089ABAC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, kill_effect__num__num, 0x0089ABA4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, set_additive__num, 0x0089AB64){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, set_collide_beamable__num, 0x0089AB54){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, set_collide_hero__num, 0x0089AB44){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, set_collide_world__num, 0x0089AB4C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, set_color__num__num__num__num, 0x0089AB34){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, set_detect_stealth__num, 0x0089AB3C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, set_material__str, 0x0089AB84){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, set_max_length__num, 0x0089AB2C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, set_no_collision, 0x0089AB5C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, set_point_to_point__vector3d__vector3d, 0x0089AB6C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, set_thickness__num, 0x0089AB24){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, set_tiles_per_meter__num, 0x0089AB7C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(beam, set_uv_anim__num__num, 0x0089AB74){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(critical_section, critical_section__num, 0x0089C7E8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(cut_scene, wait_play, 0x0089B7D8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(cut_scene, wait_play__entity_list, 0x0089B7E0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, get_handler, 0x0089C744){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, get_id, 0x0089C734){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, get_max_value, 0x0089C784){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, get_min_value, 0x0089C774){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, get_name, 0x0089C724){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, get_step_scale, 0x0089C7A4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, get_step_size, 0x0089C794){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, get_value, 0x0089C754){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, get_value_type, 0x0089C764){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, set_handler__str, 0x0089C74C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, set_id__num, 0x0089C73C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, set_max_value__num, 0x0089C78C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, set_min_value__num, 0x0089C77C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, set_name__str, 0x0089C72C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, set_step_scale__num, 0x0089C7AC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, set_step_size__num, 0x0089C79C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, set_value__num, 0x0089C75C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(debug_menu_entry, set_value_type__num, 0x0089C76C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, contains_point__vector3d, 0x0089C838){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, district__num, 0x0089C7F0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, district__str, 0x0089C7F8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, get_id, 0x0089C818){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, get_name, 0x0089C850){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, get_neighborhood, 0x0089C868){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, get_num_missions__str, 0x0089C860){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, get_variant, 0x0089C840){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, get_variant_count, 0x0089C848){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, is_in_neighborhood__num, 0x0089C870){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, is_loaded, 0x0089C820){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, is_locked, 0x0089C828){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, lock, 0x0089C800){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, purge, 0x0089C810){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, set_variant__num, 0x0089C858){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, unlock, 0x0089C808){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(district, wait_for_load, 0x0089C830){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_list, add__entity, 0x0089BFE4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_list, begin, 0x0089BFFC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_list, end, 0x0089C004){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_list, get_index__num, 0x0089C00C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_list, remove__entity, 0x0089BFEC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_list, remove__entity_list_iterator, 0x0089BFF4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_list, set_index__num__entity, 0x0089C014){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_list, size, 0x0089BFDC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_list_iterator, get_entity, 0x0089C03C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_list_iterator, operator_not_equals__entity_list_iterator, 0x0089C034){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_list_iterator, operator_plus_plus, 0x0089C01C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_list_iterator, operator_minus_munus, 0x0089C024){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_list_iterator, operator_equals_equals__entity_list_iterator, 0x0089C02C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_tracker, get_entity, 0x0089C55C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_tracker, get_mini_map_active, 0x0089C56C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_tracker, get_poi_active, 0x0089C58C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_tracker, set_entity__entity, 0x0089C554){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_tracker, set_health_widget_active__num, 0x0089C584){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_tracker, set_mini_map_active__num, 0x0089C564){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_tracker, set_poi_active__num, 0x0089C57C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(entity_tracker, set_poi_icon__num, 0x0089C574){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(glamour_cam, set_angle__num__num__num__num, 0x0089B788){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(glamour_cam, set_look_at_bone__str, 0x0089B7A0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(glamour_cam, set_position_bone__str, 0x0089B798){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(glamour_cam, set_target__entity, 0x0089B790){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(gun, get_blaster_beam, 0x0089B588){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(gun, get_muzzle_pos, 0x0089B590){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(gun, set_target_info__entity__vector3d__vector3d, 0x0089B598){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(interactable_interface, add_box_trigger__string_hash__num__vector3d__vector3d, 0x0089B61C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(interactable_interface, add_named_box_trigger__string_hash__str, 0x0089B624){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(interactable_interface, add_point_trigger__string_hash__num__vector3d__num, 0x0089B614){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(interactable_interface, create_interaction__string_hash__num, 0x0089B5E4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(interactable_interface, create_interaction__string_hash__num__str, 0x0089B5EC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(interactable_interface, disable_interaction__string_hash, 0x0089B5DC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(interactable_interface, enable_interaction__string_hash, 0x0089B5D4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(interactable_interface, nondirectional__string_hash, 0x0089B604){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(interactable_interface, one_shot__string_hash, 0x0089B5FC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(interactable_interface, set_activation_button__string_hash__num, 0x0089B5F4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(interactable_interface, set_approach__string_hash__num, 0x0089B60C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, draw, 0x0089B6A4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, get_count, 0x0089B63C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, get_last_grenade_armed, 0x0089B674){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, get_last_grenade_detonated, 0x0089B67C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, get_last_grenade_spawned, 0x0089B66C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, get_launch_force, 0x0089B64C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, get_launch_vec, 0x0089B65C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, get_owner, 0x0089B684){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, get_visual_item, 0x0089B664){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, holster, 0x0089B69C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, is_a_grenade, 0x0089B6E4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, is_a_gun, 0x0089B6DC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, is_a_melee, 0x0089B6F4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, is_a_rocket, 0x0089B6EC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, set_count__num, 0x0089B634){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, set_damage__num, 0x0089B6CC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, set_drawn_limb__str, 0x0089B68C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, set_holster_limb__str, 0x0089B694){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, set_launch_force__num, 0x0089B644){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, set_launch_vec__vector3d, 0x0089B654){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, use, 0x0089B6AC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(item, yank_remove__vector3d__num, 0x0089B6D4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(line_info, check_collision__num__num, 0x0089B718){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(line_info, clear_collision, 0x0089B720){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(line_info, debug_render__num, 0x0089B768){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(line_info, did_collide, 0x0089B760){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(line_info, get_end_pos, 0x0089B740){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(line_info, get_hit_entity, 0x0089B758){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(line_info, get_hit_normal, 0x0089B750){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(line_info, get_hit_pos, 0x0089B748){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(line_info, get_start_pos, 0x0089B730){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(line_info, set_end_pos__vector3d, 0x0089B738){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(line_info, set_start_pos__vector3d, 0x0089B728){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(mission_camera_marker, get_base_position, 0x0089C41C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(mission_camera_marker, get_camera_position, 0x0089C424){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(mission_camera_transform_marker, get_base_position, 0x0089C45C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(mission_camera_transform_marker, get_camera_position, 0x0089C464){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(mission_camera_transform_marker, get_forward, 0x0089C46C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(mission_camera_transform_marker, get_up, 0x0089C474){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(mission_transform_marker, get_base_position, 0x0089C43C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(mission_transform_marker, get_forward, 0x0089C444){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(mission_transform_marker, get_up, 0x0089C44C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(num_list, add__num, 0x0089BF6C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(num_list, begin, 0x0089BF84){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(num_list, end, 0x0089BF8C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(num_list, get_index__num, 0x0089BF94){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(num_list, remove__num, 0x0089BF74){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(num_list, remove__num_list_iterator, 0x0089BF7C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(num_list, set_index__num__num, 0x0089BF9C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(num_list, size, 0x0089BF64){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(num_list_iterator, get_num, 0x0089BFC4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(num_list_iterator, operator_not_equals__num_list_iterator, 0x0089BFBC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(num_list_iterator, operator_plus_plus, 0x0089BFA4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(num_list_iterator, operator_minus_munus, 0x0089BFAC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(num_list_iterator, operator_equals_equals__num_list_iterator, 0x0089BFB4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(pfx, get_abs_position, 0x0089C8A4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(pfx, get_parent, 0x0089C884){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(pfx, get_position, 0x0089C894){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(pfx, get_rel_position, 0x0089C8B4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(pfx, get_rotation, 0x0089C8C4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(pfx, get_scale, 0x0089C8D4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(pfx, get_state__num, 0x0089C8E4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(pfx, get_visible, 0x0089C8F4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(pfx, set_abs_position__vector3d, 0x0089C8AC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(pfx, set_parent__entity, 0x0089C88C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(pfx, set_position__vector3d, 0x0089C89C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(pfx, set_rel_position__vector3d, 0x0089C8BC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(pfx, set_rotation__vector3d, 0x0089C8CC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(pfx, set_scale__vector3d, 0x0089C8DC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(pfx, set_state__num__num, 0x0089C8EC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(pfx, set_visible__num, 0x0089C8FC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, add_control_pt__vector3d, 0x0089C108){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, build__num__num, 0x0089C140){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, clear, 0x0089C0F8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, clear_simulations, 0x0089C220){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, create_tentacle, 0x0089C240){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, destroy_tentacle, 0x0089C248){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, get_control_pt__num, 0x0089C118){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, get_curve_pt__num, 0x0089C128){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, get_ifl_frame, 0x0089C1A0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, get_max_length, 0x0089C1D8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, get_num_control_pts, 0x0089C130){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, get_num_curve_pts, 0x0089C138){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, get_num_sides, 0x0089C1B8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, get_tiles_per_meter, 0x0089C1C8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, get_tube_radius, 0x0089C1A8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, ifl_lock__num, 0x0089C190){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, ifl_play, 0x0089C198){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, kill_pt_anim__num__num, 0x0089C208){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, rebuild, 0x0089C148){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, reserve_control_pts__num, 0x0089C100){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_additive__num, 0x0089C120){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_begin_material__str__num, 0x0089C168){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_begin_material_ifl__str__num, 0x0089C180){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_blend_mode__num, 0x0089C188){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_control_pt__num__vector3d, 0x0089C110){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_end_material__str__num, 0x0089C160){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_end_material_ifl__str__num, 0x0089C178){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_force_start__num, 0x0089C150){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_material__str, 0x0089C158){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_material_ifl__str, 0x0089C170){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_max_length__num, 0x0089C1E0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_num_sides__num, 0x0089C1C0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_pt_anim__num__vector3d__num__num, 0x0089C1F8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_random_pt_anim__num__num__num__num, 0x0089C200){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_start_v__num, 0x0089C1F0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_tiles_per_meter__num, 0x0089C1D0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, set_tube_radius__num, 0x0089C1B0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, simulate_slack__vector3d__vector3d__num, 0x0089C210){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, simulate_tether__entity__entity__num, 0x0089C218){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_add_dangle_engine__num__num, 0x0089C2A8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_add_follow_engine__num__num__vector3d__num, 0x0089C2B0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_add_slack_engine__num__num, 0x0089C2B8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_add_strike_engine__vector3d__num__num, 0x0089C298){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_add_swirl_strike_engine__vector3d__num__num__num__num, 0x0089C2A0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_create_line__vector3d__num, 0x0089C280){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_engine_running__num, 0x0089C290){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_get_end_position, 0x0089C260){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_get_engine_drag__num, 0x0089C2F0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_get_engine_gravity__num, 0x0089C300){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_get_engine_length__num, 0x0089C2D0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_get_engine_slack_percent__num, 0x0089C2E0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_get_num_positions, 0x0089C268){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_get_position__num, 0x0089C278){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_kill_engine__num, 0x0089C288){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_set_collide__num, 0x0089C250){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_set_end_position__vector3d, 0x0089C258){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_set_engine_attach_ent__num__entity, 0x0089C2C0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_set_engine_drag__num__num, 0x0089C2F8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_set_engine_gravity__num__vector3d, 0x0089C308){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_set_engine_length__num__num, 0x0089C2D8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_set_engine_slack_percent__num__num, 0x0089C2E8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_set_engine_target__num__vector3d, 0x0089C2C8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, tentacle_set_position__num__vector3d, 0x0089C270){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(polytube, v_animate__num, 0x0089C1E8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(posfacing3d, get_facing, 0x0089BA98){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(posfacing3d, get_position, 0x0089BAA8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(posfacing3d, get_rotation, 0x0089BAA0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(posfacing3d, operator_not_equals__posfacing3d, 0x0089BAB8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(posfacing3d, operator_plus__posfacing3d, 0x0089BA88){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(posfacing3d, operator_multiply__posfacing3d, 0x0089BA90){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(posfacing3d, operator_equals_equals__posfacing3d, 0x0089BAB0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(posfacing3d, posfacing3d__num__num__num__num, 0x0089BA68){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(posfacing3d, posfacing3d__num__num__num__vector3d, 0x0089BA78){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(posfacing3d, posfacing3d__vector3d__num, 0x0089BA70){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(posfacing3d, posfacing3d__vector3d__vector3d, 0x0089BA80){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(position3d, operator_assign__vector3d, 0x0089BA60){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(position3d, position3d__num__num__num, 0x0089BA58){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(script_controller, is_button_pressed__num, 0x0089BCD8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(script_controller, is_down_pressed, 0x0089BCE8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(script_controller, is_left_pressed, 0x0089BCF0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(script_controller, is_lstick_down_pressed, 0x0089BD28){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(script_controller, is_lstick_left_pressed, 0x0089BD30){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(script_controller, is_lstick_right_pressed, 0x0089BD38){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(script_controller, is_lstick_up_pressed, 0x0089BD20){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(script_controller, is_right_pressed, 0x0089BCF8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(script_controller, is_rstick_down_pressed, 0x0089BD08){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(script_controller, is_rstick_left_pressed, 0x0089BD10){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(script_controller, is_rstick_right_pressed, 0x0089BD18){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(script_controller, is_rstick_up_pressed, 0x0089BD00){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(script_controller, is_up_pressed, 0x0089BCE0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(signaller, clear_callback__str, 0x0089B7F0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(signaller, clear_callbacks, 0x0089B7E8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, dampen_guard, 0x0089B880){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, fade_in__num__num, 0x0089B898){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, fade_out__num, 0x0089B8A0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, fade_to__num__num, 0x0089B890){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, get_doppler, 0x0089B878){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, get_entity, 0x0089B848){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, get_max_distance, 0x0089B870){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, get_min_distance, 0x0089B868){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, get_pitch, 0x0089B860){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, get_source, 0x0089B828){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, get_status, 0x0089B888){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, get_volume, 0x0089B858){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, is_valid, 0x0089B820){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, play__num, 0x0089B8B0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, play_3d__vector3d__num, 0x0089B8B8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, queue__num, 0x0089B8A8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, set_entity__entity, 0x0089B850){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, set_preregistered_source__str, 0x0089B830){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, set_sound_group__str, 0x0089B840){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, set_source__str, 0x0089B838){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, stop__num, 0x0089B8C8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(sound_inst, wait__num, 0x0089B8C0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(str_list, add__str, 0x0089C05C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(str_list, begin, 0x0089C074){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(str_list, end, 0x0089C07C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(str_list, get_index__num, 0x0089C084){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(str_list, remove__str, 0x0089C064){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(str_list, remove__str_list_iterator, 0x0089C06C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(str_list, set_index__num__str, 0x0089C08C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(str_list, size, 0x0089C054){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(str_list_iterator, get_str, 0x0089C0B4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(str_list_iterator, operator_not_equals__str_list_iterator, 0x0089C0AC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(str_list_iterator, operator_plus_plus, 0x0089C094){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(str_list_iterator, operator_minus_munus, 0x0089C09C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(str_list_iterator, operator_equals_equals__str_list_iterator, 0x0089C0A4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(string_hash, operator_equals_equals__str, 0x0089C7D4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(string_hash, operator_equals_equals__string_hash, 0x0089C7CC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(string_hash, set__str, 0x0089C7C4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(string_hash, string_hash__str, 0x0089C7B4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(string_hash, to_string, 0x0089C7BC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(switch_obj, get_state, 0x0089B8DC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(switch_obj, set_alarm__num, 0x0089B904){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(switch_obj, set_single_use__num, 0x0089B90C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(switch_obj, set_state__num, 0x0089B8F4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(switch_obj, set_web_targetable__num, 0x0089B8FC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(switch_obj, who_pressed_me, 0x0089B8EC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(tam, set_entity__entity, 0x0089C6DC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(tam, set_pos__vector3d, 0x0089C6E4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(tam, set_state__num, 0x0089C6EC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(tam, set_type__num, 0x0089C6F4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_entry, get_entity, 0x0089C62C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_entry, get_flags, 0x0089C634){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_entry, get_taunt_name, 0x0089C624){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange, get_entity_to_do_hurting, 0x0089C654){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange, get_entity_to_hurt, 0x0089C64C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange, get_last_hit_pt_value, 0x0089C664){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange, get_max_times_to_play, 0x0089C684){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange, get_min_damage, 0x0089C65C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange, get_normalized_weight, 0x0089C6A4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange, get_taunt_entry__num, 0x0089C694){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange, get_times_played, 0x0089C674){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange, get_total_taunts, 0x0089C68C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange, get_weight, 0x0089C69C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange, increment_times_played, 0x0089C67C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange, set_last_hit_pt_value__num, 0x0089C66C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange, set_normalized_weight__num, 0x0089C6AC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange_list, add__taunt_exchange, 0x0089C0BC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange_list, clear, 0x0089C0D4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange_list, get_index__num, 0x0089C0C4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(taunt_exchange_list, size, 0x0089C0CC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(trigger, contains__entity, 0x0089B938){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(trigger, get_abs_position, 0x0089B920){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(trigger, get_radius, 0x0089B9A0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(trigger, get_triggered_ent, 0x0089B928){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(trigger, get_triggered_entity_list, 0x0089B930){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(trigger, is_point_radius_trigger, 0x0089B9B0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(trigger, set_active__num, 0x0089B940){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(trigger, set_multiple_entrance__num, 0x0089B988){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(trigger, set_position__vector3d, 0x0089B9A8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(trigger, set_radius__num, 0x0089B998){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(trigger, set_sees_dead_people__num, 0x0089B990){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(trigger, set_use_any_char__num, 0x0089B980){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d, length, 0x0089B9F8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d, length2, 0x0089BA00){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d, operator_not_equals__vector3d, 0x0089B9F0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d, operator_multiply__num, 0x0089B9D8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d, operator_plus__vector3d, 0x0089B9C8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d, operator_minus__vector3d, 0x0089B9D0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d, operator_divide__num, 0x0089B9E0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d, operator_equals_equals__vector3d, 0x0089B9E8){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d, vector3d__num__num__num, 0x0089B9C0){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d, x, 0x0089BA18){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d, xy_norm, 0x0089BA08){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d, xz_norm, 0x0089BA10){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d, y, 0x0089BA20){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d, z, 0x0089BA28){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d_list, add__vector3d, 0x0089BEEC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d_list, begin, 0x0089BF04){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d_list, clear, 0x0089BF1C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d_list, end, 0x0089BF0C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d_list, get_index__num, 0x0089BF14){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d_list, remove__vector3d, 0x0089BEF4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d_list, remove__vector3d_list_iterator, 0x0089BEFC){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d_list, set_index__num__vector3d, 0x0089BF24){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d_list, size, 0x0089BEE4){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d_list_iterator, get_vector3d, 0x0089BF4C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d_list_iterator, operator_not_equals__vector3d_list_iterator, 0x0089BF44){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d_list_iterator, operator_plus_plus, 0x0089BF2C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d_list_iterator, operator_minus_munus, 0x0089BF34){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

DECLARE_SLF_BEGIN(vector3d_list_iterator, operator_equals_equals__vector3d_list_iterator, 0x0089BF3C){
    (void) stack;
    (void) entry;
	return true;
}
DECLARE_SLF_END()

#undef DECLARE_SLF_BEGIN
#undef DECLARE_SLF_END

void chuck_register_script_libs()
{
    TRACE("chuck_register_script_libs");

    if constexpr (1) {
        std::vector<script_library_class *> classes(39u);
        auto class_idx = 0u;

#define CREATE_SLC(TYPE) \
    classes[class_idx++] = new (mem_alloc(sizeof(TYPE))) TYPE {}

        CREATE_SLC(slc_pfx_t);
        CREATE_SLC(slc_interactable_interface_t);

        slc_anim = new (mem_alloc(sizeof(slc_anim_t))) slc_anim_t {"anim", 4};
        classes[class_idx++] = slc_anim;

        CREATE_SLC(slc_beam_t);

        slc_entity = new (mem_alloc(sizeof(slc_entity_t))) slc_entity_t {"entity", 4, "signaller"};
        classes[class_idx++] = slc_entity;

        CREATE_SLC(slc_cut_scene_t);
        CREATE_SLC(slc_debug_menu_entry_t);
        CREATE_SLC(slc_entity_list_t);
        CREATE_SLC(slc_entity_list_iterator_t);
        CREATE_SLC(slc_entity_tracker_t);
        CREATE_SLC(slc_glamour_cam_t);
        CREATE_SLC(slc_gun_t);
        CREATE_SLC(slc_item_t);
        CREATE_SLC(slc_line_info_t);
        CREATE_SLC(slc_mission_camera_marker_t);
        CREATE_SLC(slc_mission_camera_transform_marker_t);
        CREATE_SLC(slc_mission_transform_marker_t);
        CREATE_SLC(slc_num_list_t);
        CREATE_SLC(slc_num_list_iterator_t);
        CREATE_SLC(slc_panel_t);
        CREATE_SLC(slc_polytube_t);
        CREATE_SLC(slc_script_controller_t);
        CREATE_SLC(slc_signaller_t);
        CREATE_SLC(slc_sound_inst_t);
        CREATE_SLC(slc_str_list_t);
        CREATE_SLC(slc_str_list_iterator_t);
        CREATE_SLC(slc_switch_obj_t);
        CREATE_SLC(slc_tam_t);
        CREATE_SLC(slc_taunt_entry_t);
        CREATE_SLC(slc_taunt_exchange_t);
        CREATE_SLC(slc_taunt_exchange_list_t);
        CREATE_SLC(slc_trigger_t);
        CREATE_SLC(slc_vector3d_t);
        CREATE_SLC(slc_position3d_t);
        CREATE_SLC(slc_posfacing3d_t);
        CREATE_SLC(slc_vector3d_list_t);
        CREATE_SLC(slc_vector3d_list_iterator_t);
        CREATE_SLC(slc_string_hash_t);
        CREATE_SLC(slc_critical_section_t);
        CREATE_SLC(slc_district_t);

        if (!g_is_the_packer() && !script_manager::using_chuck_old_fashioned()) {
            slc_manager::un_mash_all_funcs();
        }

        register_anim_lib();

        register_entity_lib();

#undef CREATE_SLC

#define CREATE_GLOBAL_SLF(TYPE, NAME)              \
    new (mem_alloc(sizeof(BUILD_GLOBAL_SLF_NAME(TYPE)))) \
        BUILD_GLOBAL_SLF_NAME(TYPE) {NAME}

        {
            CREATE_GLOBAL_SLF(abs_delay__num, "abs_delay(num)");
            CREATE_GLOBAL_SLF(acos__num, "acos(num)");
            CREATE_GLOBAL_SLF(add_2d_debug_str__vector3d__vector3d__num__str, "add_2d_debug_str(vector3d,vector3d,num,str)");
            CREATE_GLOBAL_SLF(add_2d_debug_str__vector3d__vector3d__num__str__num, "add_2d_debug_str(vector3d,vector3d,num,str,num)");
            CREATE_GLOBAL_SLF(add_3d_debug_str__vector3d__vector3d__num__str, "add_3d_debug_str(vector3d,vector3d,num,str)");
            CREATE_GLOBAL_SLF(add_civilian_info__vector3d__num__num__num, "add_civilian_info(vector3d,num,num,num)");
            CREATE_GLOBAL_SLF(add_civilian_info_entity__entity__num__num__num, "add_civilian_info_entity(entity,num,num,num)");
            CREATE_GLOBAL_SLF(add_debug_cyl__vector3d__vector3d__num, "add_debug_cyl(vector3d,vector3d,num)");
            CREATE_GLOBAL_SLF(add_debug_cyl__vector3d__vector3d__num__vector3d__num, "add_debug_cyl(vector3d,vector3d,num,vector3d,num)");
            CREATE_GLOBAL_SLF(add_debug_line__vector3d__vector3d, "add_debug_line(vector3d,vector3d)");
            CREATE_GLOBAL_SLF(add_debug_line__vector3d__vector3d__vector3d__num, "add_debug_line(vector3d,vector3d,vector3d,num)");
            CREATE_GLOBAL_SLF(add_debug_sphere__vector3d__num, "add_debug_sphere(vector3d,num)");
            CREATE_GLOBAL_SLF(add_debug_sphere__vector3d__num__vector3d__num, "add_debug_sphere(vector3d,num,vector3d,num)");
            CREATE_GLOBAL_SLF(add_glass_house__str, "add_glass_house(str)");
            CREATE_GLOBAL_SLF(add_glass_house__str__num, "add_glass_house(str,num)");
            CREATE_GLOBAL_SLF(add_glass_house__str__num__vector3d, "add_glass_house(str,num,vector3d)");
            CREATE_GLOBAL_SLF(add_glass_house__str__vector3d, "add_glass_house(str,vector3d)");
            CREATE_GLOBAL_SLF(add_to_console__str, "add_to_console(str)");
            CREATE_GLOBAL_SLF(add_traffic_model__num__str, "add_traffic_model(num,str)");
            CREATE_GLOBAL_SLF(allow_suspend_thread__num, "allow_suspend_thread(num)");
            CREATE_GLOBAL_SLF(angle_between__vector3d__vector3d, "angle_between(vector3d,vector3d)");
            CREATE_GLOBAL_SLF(apply_donut_damage__vector3d__num__num__num__num__num, "apply_donut_damage(vector3d,num,num,num,num,num)");
            CREATE_GLOBAL_SLF(apply_radius_damage__vector3d__num__num__num__num, "apply_radius_damage(vector3d,num,num,num,num)");
            CREATE_GLOBAL_SLF(apply_radius_subdue__vector3d__num__num__num__num, "apply_radius_subdue(vector3d,num,num,num,num)");
            CREATE_GLOBAL_SLF(assert__num__str, "assert(num,str)");
            CREATE_GLOBAL_SLF(attach_decal__str__vector3d__num__vector3d__entity, "attach_decal(str,vector3d,num,vector3d,entity)");
            CREATE_GLOBAL_SLF(begin_screen_recording__str__num, "begin_screen_recording(str,num)");
            CREATE_GLOBAL_SLF(blackscreen_off__num, "blackscreen_off(num)");
            CREATE_GLOBAL_SLF(blackscreen_on__num, "blackscreen_on(num)");
            CREATE_GLOBAL_SLF(bring_up_dialog_box__num__num, "bring_up_dialog_box(num,num,...)");
            CREATE_GLOBAL_SLF(bring_up_dialog_box_debug__str__num__str, "bring_up_dialog_box_debug(str,num,str)");
            CREATE_GLOBAL_SLF(bring_up_dialog_box_title__num__num__num, "bring_up_dialog_box_title(num,num,num,...)");
            CREATE_GLOBAL_SLF(bring_up_medal_award_box__num, "bring_up_medal_award_box(num)");
            CREATE_GLOBAL_SLF(bring_up_race_announcer, "bring_up_race_announcer()");
            CREATE_GLOBAL_SLF(calc_launch_vector__vector3d__vector3d__num__entity, "calc_launch_vector(vector3d,vector3d,num,entity)");
            CREATE_GLOBAL_SLF(can_load_pack__str, "can_load_pack(str)");
            CREATE_GLOBAL_SLF(chase_cam, "chase_cam()");
            CREATE_GLOBAL_SLF(clear_all_grenades, "clear_all_grenades()");
            CREATE_GLOBAL_SLF(clear_civilians_within_radius__vector3d__num, "clear_civilians_within_radius(vector3d,num)");
            CREATE_GLOBAL_SLF(clear_controls, "clear_controls()");
            CREATE_GLOBAL_SLF(clear_debug_all, "clear_debug_all()");
            CREATE_GLOBAL_SLF(clear_debug_cyls, "clear_debug_cyls()");
            CREATE_GLOBAL_SLF(clear_debug_lines, "clear_debug_lines()");
            CREATE_GLOBAL_SLF(clear_debug_spheres, "clear_debug_spheres()");
            CREATE_GLOBAL_SLF(clear_screen, "clear_screen()");
            CREATE_GLOBAL_SLF(clear_traffic_within_radius__vector3d__num, "clear_traffic_within_radius(vector3d,num)");
            CREATE_GLOBAL_SLF(col_check__vector3d__vector3d__num, "col_check(vector3d,vector3d,num)");
            CREATE_GLOBAL_SLF(console_exec__str, "console_exec(str)");
            CREATE_GLOBAL_SLF(copy_vector3d_list__vector3d_list__vector3d_list, "copy_vector3d_list(vector3d_list,vector3d_list)");
            CREATE_GLOBAL_SLF(cos__num, "cos(num)");
            CREATE_GLOBAL_SLF(create_beam, "create_beam()");
            CREATE_GLOBAL_SLF(create_credits, "create_credits()");
            CREATE_GLOBAL_SLF(create_cut_scene__str, "create_cut_scene(str)");
            CREATE_GLOBAL_SLF(create_debug_menu_entry__str, "create_debug_menu_entry(str)");
            CREATE_GLOBAL_SLF(create_debug_menu_entry__str__str, "create_debug_menu_entry(str,str)");
            CREATE_GLOBAL_SLF(create_decal__str__vector3d__num__vector3d, "create_decal(str,vector3d,num,vector3d)");
            CREATE_GLOBAL_SLF(create_entity__str, "create_entity(str)");
            CREATE_GLOBAL_SLF(create_entity__str__str, "create_entity(str,str)");
            CREATE_GLOBAL_SLF(create_entity_in_hero_region__str, "create_entity_in_hero_region(str)");
            CREATE_GLOBAL_SLF(create_entity_list, "create_entity_list()");
            CREATE_GLOBAL_SLF(create_entity_tracker__entity, "create_entity_tracker(entity)");
            CREATE_GLOBAL_SLF(create_item__str, "create_item(str)");
            CREATE_GLOBAL_SLF(create_line_info__vector3d__vector3d, "create_line_info(vector3d,vector3d)");
            CREATE_GLOBAL_SLF(create_lofi_stereo_sound_inst__str, "create_lofi_stereo_sound_inst(str)");
            CREATE_GLOBAL_SLF(create_num_list, "create_num_list()");
            CREATE_GLOBAL_SLF(create_pfx__str, "create_pfx(str)");
            CREATE_GLOBAL_SLF(create_pfx__str__vector3d, "create_pfx(str,vector3d)");
            CREATE_GLOBAL_SLF(create_polytube, "create_polytube()");
            CREATE_GLOBAL_SLF(create_polytube__str, "create_polytube(str)");
            CREATE_GLOBAL_SLF(create_progression_menu_entry__str__str, "create_progression_menu_entry(str,str)");
            CREATE_GLOBAL_SLF(create_sound_inst, "create_sound_inst()");
            CREATE_GLOBAL_SLF(create_sound_inst__str, "create_sound_inst(str)");
            CREATE_GLOBAL_SLF(create_stompable_music_sound_inst__str, "create_stompable_music_sound_inst(str)");
            CREATE_GLOBAL_SLF(create_str_list, "create_str_list()");
            CREATE_GLOBAL_SLF(create_taunt_entry__entity__str__num, "create_taunt_entry(entity,str,num)");
            CREATE_GLOBAL_SLF(create_taunt_exchange__entity__entity__num__num__num__num, "create_taunt_exchange(entity,entity,num,num,num,num,...)");
            CREATE_GLOBAL_SLF(create_taunt_exchange_list, "create_taunt_exchange_list()");
            CREATE_GLOBAL_SLF(create_threat_assessment_meter, "create_threat_assessment_meter()");
            CREATE_GLOBAL_SLF(create_time_limited_entity__str__num, "create_time_limited_entity(str,num)");
            CREATE_GLOBAL_SLF(create_trigger__entity__num, "create_trigger(entity,num)");
            CREATE_GLOBAL_SLF(create_trigger__str__vector3d__num, "create_trigger(str,vector3d,num)");
            CREATE_GLOBAL_SLF(create_trigger__vector3d__num, "create_trigger(vector3d,num)");
            CREATE_GLOBAL_SLF(create_unstompable_script_cutscene_sound_inst__str, "create_unstompable_script_cutscene_sound_inst(str)");
            CREATE_GLOBAL_SLF(create_vector3d_list, "create_vector3d_list()");
            CREATE_GLOBAL_SLF(cross__vector3d__vector3d, "cross(vector3d,vector3d)");
            CREATE_GLOBAL_SLF(debug_breakpoint, "debug_breakpoint()");
            CREATE_GLOBAL_SLF(debug_print__num__str, "debug_print(num,str)");
            CREATE_GLOBAL_SLF(debug_print__num__vector3d__str, "debug_print(num,vector3d,str)");
            CREATE_GLOBAL_SLF(debug_print__str, "debug_print(str)");
            CREATE_GLOBAL_SLF(debug_print_set_background_color__vector3d, "debug_print_set_background_color(vector3d)");
            CREATE_GLOBAL_SLF(delay__num, "delay(num)");
            CREATE_GLOBAL_SLF(destroy_credits, "destroy_credits()");
            CREATE_GLOBAL_SLF(destroy_debug_menu_entry__debug_menu_entry, "destroy_debug_menu_entry(debug_menu_entry)");
            CREATE_GLOBAL_SLF(destroy_entity__entity, "destroy_entity(entity)");
            CREATE_GLOBAL_SLF(destroy_entity_list__entity_list, "destroy_entity_list(entity_list)");
            CREATE_GLOBAL_SLF(destroy_entity_tracker__entity_tracker, "destroy_entity_tracker(entity_tracker)");
            CREATE_GLOBAL_SLF(destroy_line_info__line_info, "destroy_line_info(line_info)");
            CREATE_GLOBAL_SLF(destroy_num_list__num_list, "destroy_num_list(num_list)");
            CREATE_GLOBAL_SLF(destroy_pfx__pfx, "destroy_pfx(pfx)");
            CREATE_GLOBAL_SLF(destroy_str_list__str_list, "destroy_str_list(str_list)");
            CREATE_GLOBAL_SLF(destroy_taunt_entry__taunt_entry, "destroy_taunt_entry(taunt_entry)");
            CREATE_GLOBAL_SLF(destroy_taunt_exchange__taunt_exchange, "destroy_taunt_exchange(taunt_exchange)");
            CREATE_GLOBAL_SLF(destroy_taunt_exchange_list__taunt_exchange_list, "destroy_taunt_exchange_list(taunt_exchange_list)");
            CREATE_GLOBAL_SLF(destroy_threat_assessment_meter__tam, "destroy_threat_assessment_meter(tam)");
            CREATE_GLOBAL_SLF(destroy_trigger__trigger, "destroy_trigger(trigger)");
            CREATE_GLOBAL_SLF(destroy_vector3d_list__vector3d_list, "destroy_vector3d_list(vector3d_list)");
            CREATE_GLOBAL_SLF(dilated_delay__num, "dilated_delay(num)");
            CREATE_GLOBAL_SLF(disable_marky_cam__num, "disable_marky_cam(num)");
            CREATE_GLOBAL_SLF(disable_nearby_occlusion_only_obb__vector3d, "disable_nearby_occlusion_only_obb(vector3d)");
            CREATE_GLOBAL_SLF(disable_player_shadows, "disable_player_shadows()");
            CREATE_GLOBAL_SLF(disable_subtitles, "disable_subtitles()");
            CREATE_GLOBAL_SLF(disable_vibrator, "disable_vibrator()");
            CREATE_GLOBAL_SLF(disable_zoom_map__num, "disable_zoom_map(num)");
            CREATE_GLOBAL_SLF(distance3d__vector3d__vector3d, "distance3d(vector3d,vector3d)");
            CREATE_GLOBAL_SLF(distance_chase_widget_set_pos__num, "distance_chase_widget_set_pos(num)");
            CREATE_GLOBAL_SLF(distance_chase_widget_turn_off, "distance_chase_widget_turn_off()");
            CREATE_GLOBAL_SLF(distance_chase_widget_turn_on__num__num, "distance_chase_widget_turn_on(num,num)");
            CREATE_GLOBAL_SLF(distance_race_widget_set_boss_pos__num, "distance_race_widget_set_boss_pos(num)");
            CREATE_GLOBAL_SLF(distance_race_widget_set_hero_pos__num, "distance_race_widget_set_hero_pos(num)");
            CREATE_GLOBAL_SLF(distance_race_widget_set_types__num__num, "distance_race_widget_set_types(num,num)");
            CREATE_GLOBAL_SLF(distance_race_widget_turn_off, "distance_race_widget_turn_off()");
            CREATE_GLOBAL_SLF(distance_race_widget_turn_on, "distance_race_widget_turn_on()");
            CREATE_GLOBAL_SLF(district_id__str, "district_id(str)");
            CREATE_GLOBAL_SLF(district_name__num, "district_name(num)");
            CREATE_GLOBAL_SLF(dot__vector3d__vector3d, "dot(vector3d,vector3d)");
            CREATE_GLOBAL_SLF(dump_searchable_region_list__str, "dump_searchable_region_list(str)");
            CREATE_GLOBAL_SLF(enable_ai__num, "enable_ai(num)");
            CREATE_GLOBAL_SLF(enable_civilians__num, "enable_civilians(num)");
            CREATE_GLOBAL_SLF(enable_controls__num, "enable_controls(num)");
            CREATE_GLOBAL_SLF(enable_entity_fading__num, "enable_entity_fading(num)");
            CREATE_GLOBAL_SLF(enable_interface__num, "enable_interface(num)");
            CREATE_GLOBAL_SLF(enable_marky_cam__num, "enable_marky_cam(num)");
            CREATE_GLOBAL_SLF(enable_mini_map__num, "enable_mini_map(num)");
            CREATE_GLOBAL_SLF(enable_nearby_occlusion_only_obb__vector3d, "enable_nearby_occlusion_only_obb(vector3d)");
            CREATE_GLOBAL_SLF(enable_obb__vector3d__num, "enable_obb(vector3d,num)");
            CREATE_GLOBAL_SLF(enable_pause__num, "enable_pause(num)");
            CREATE_GLOBAL_SLF(enable_physics__num, "enable_physics(num)");
            CREATE_GLOBAL_SLF(enable_player_shadows, "enable_player_shadows()");
            CREATE_GLOBAL_SLF(enable_pois__num, "enable_pois(num)");
            CREATE_GLOBAL_SLF(enable_quad_path_connector__district__num__district__num__num, "enable_quad_path_connector(district,num,district,num,num)");
            CREATE_GLOBAL_SLF(enable_subtitles, "enable_subtitles()");
            CREATE_GLOBAL_SLF(enable_tokens_of_type__num__num, "enable_tokens_of_type(num,num)");
            CREATE_GLOBAL_SLF(enable_traffic__num, "enable_traffic(num)");
            CREATE_GLOBAL_SLF(enable_user_camera__num, "enable_user_camera(num)");
            CREATE_GLOBAL_SLF(enable_vibrator, "enable_vibrator()");
            CREATE_GLOBAL_SLF(end_current_patrol, "end_current_patrol()");
            CREATE_GLOBAL_SLF(end_cut_scenes, "end_cut_scenes()");
            CREATE_GLOBAL_SLF(end_screen_recording, "end_screen_recording()");
            CREATE_GLOBAL_SLF(entity_col_check__entity__entity, "entity_col_check(entity,entity)");
            CREATE_GLOBAL_SLF(entity_exists__str, "entity_exists(str)");
            CREATE_GLOBAL_SLF(entity_get_entity_tracker__entity, "entity_get_entity_tracker(entity)");
            CREATE_GLOBAL_SLF(entity_has_entity_tracker__entity, "entity_has_entity_tracker(entity)");
            CREATE_GLOBAL_SLF(exit_water__entity, "exit_water(entity)");
            CREATE_GLOBAL_SLF(find_closest_point_on_a_path_to_point__vector3d, "find_closest_point_on_a_path_to_point(vector3d)");
            CREATE_GLOBAL_SLF(find_district_for_point__vector3d, "find_district_for_point(vector3d)");
            CREATE_GLOBAL_SLF(find_entities_in_radius__entity_list__vector3d__num__num, "find_entities_in_radius(entity_list,vector3d,num,num)");
            CREATE_GLOBAL_SLF(find_entity__str, "find_entity(str)");
            CREATE_GLOBAL_SLF(find_innermost_district__vector3d, "find_innermost_district(vector3d)");
            CREATE_GLOBAL_SLF(find_outermost_district__vector3d, "find_outermost_district(vector3d)");
            CREATE_GLOBAL_SLF(find_trigger__entity, "find_trigger(entity)");
            CREATE_GLOBAL_SLF(find_trigger__str, "find_trigger(str)");
            CREATE_GLOBAL_SLF(find_trigger_in_district__district__str, "find_trigger_in_district(district,str)");
            CREATE_GLOBAL_SLF(float_random__num, "float_random(num)");
            CREATE_GLOBAL_SLF(force_mission__district__str__num, "force_mission(district,str,num)");
            CREATE_GLOBAL_SLF(force_streamer_refresh, "force_streamer_refresh()");
            CREATE_GLOBAL_SLF(format_time_string__num, "format_time_string(num)");
            CREATE_GLOBAL_SLF(freeze_hero__num, "freeze_hero(num)");
            CREATE_GLOBAL_SLF(game_ini_get_flag__str, "game_ini_get_flag(str)");
            CREATE_GLOBAL_SLF(game_time_advance__num__num, "game_time_advance(num,num)");
            CREATE_GLOBAL_SLF(get_all_execs_thread_count__str, "get_all_execs_thread_count(str)");
            CREATE_GLOBAL_SLF(get_all_instances_thread_count__str, "get_all_instances_thread_count(str)");
            CREATE_GLOBAL_SLF(get_attacker_entity, "get_attacker_entity()");
            CREATE_GLOBAL_SLF(get_attacker_member, "get_attacker_member()");
            CREATE_GLOBAL_SLF(get_available_stack_size, "get_available_stack_size()");
            CREATE_GLOBAL_SLF(get_character_packname_list, "get_character_packname_list()");
            CREATE_GLOBAL_SLF(get_closest_point_on_lane_with_facing__num__vector3d__vector3d_list, "get_closest_point_on_lane_with_facing(num,vector3d,vector3d_list)");
            CREATE_GLOBAL_SLF(get_col_hit_ent, "get_col_hit_ent()");
            CREATE_GLOBAL_SLF(get_col_hit_norm, "get_col_hit_norm()");
            CREATE_GLOBAL_SLF(get_col_hit_pos, "get_col_hit_pos()");
            CREATE_GLOBAL_SLF(get_control_state__num, "get_control_state(num)");
            CREATE_GLOBAL_SLF(get_control_trigger__num, "get_control_trigger(num)");
            CREATE_GLOBAL_SLF(get_current_instance_thread_count__str, "get_current_instance_thread_count(str)");
            CREATE_GLOBAL_SLF(get_current_view_cam_pos, "get_current_view_cam_pos()");
            CREATE_GLOBAL_SLF(get_current_view_cam_x_facing, "get_current_view_cam_x_facing()");
            CREATE_GLOBAL_SLF(get_current_view_cam_y_facing, "get_current_view_cam_y_facing()");
            CREATE_GLOBAL_SLF(get_current_view_cam_z_facing, "get_current_view_cam_z_facing()");
            CREATE_GLOBAL_SLF(get_fog_color, "get_fog_color()");
            CREATE_GLOBAL_SLF(get_fog_distance, "get_fog_distance()");
            CREATE_GLOBAL_SLF(get_game_info_num__str, "get_game_info_num(str)");
            CREATE_GLOBAL_SLF(get_game_info_str__str, "get_game_info_str(str)");
            CREATE_GLOBAL_SLF(get_glam_cam__num, "get_glam_cam(num)");
            CREATE_GLOBAL_SLF(get_global_time_dilation, "get_global_time_dilation()");
            CREATE_GLOBAL_SLF(get_ini_flag__str, "get_ini_flag(str)");
            CREATE_GLOBAL_SLF(get_ini_num__str, "get_ini_num(str)");
            CREATE_GLOBAL_SLF(get_int_num__num, "get_int_num(num)");
            CREATE_GLOBAL_SLF(get_mission_camera_marker__num, "get_mission_camera_marker(num)");
            CREATE_GLOBAL_SLF(get_mission_camera_transform_marker__num, "get_mission_camera_transform_marker(num)");
            CREATE_GLOBAL_SLF(get_mission_entity, "get_mission_entity()");
            CREATE_GLOBAL_SLF(get_mission_key_posfacing3d, "get_mission_key_posfacing3d()");
            CREATE_GLOBAL_SLF(get_mission_key_position, "get_mission_key_position()");
            CREATE_GLOBAL_SLF(get_mission_marker__num, "get_mission_marker(num)");
            CREATE_GLOBAL_SLF(get_mission_nums, "get_mission_nums()");
            CREATE_GLOBAL_SLF(get_mission_patrol_waypoint, "get_mission_patrol_waypoint()");
            CREATE_GLOBAL_SLF(get_mission_positions, "get_mission_positions()");
            CREATE_GLOBAL_SLF(get_mission_strings, "get_mission_strings()");
            CREATE_GLOBAL_SLF(get_mission_transform_marker__num, "get_mission_transform_marker(num)");
            CREATE_GLOBAL_SLF(get_mission_trigger, "get_mission_trigger()");
            CREATE_GLOBAL_SLF(get_missions_key_position_by_index__district__str__num, "get_missions_key_position_by_index(district,str,num)");
            CREATE_GLOBAL_SLF(get_missions_nums_by_index__district__str__num__num_list, "get_missions_nums_by_index(district,str,num,num_list)");
            CREATE_GLOBAL_SLF(get_missions_patrol_waypoint_by_index__district__str__num, "get_missions_patrol_waypoint_by_index(district,str,num)");
            CREATE_GLOBAL_SLF(get_neighborhood_name__num, "get_neighborhood_name(num)");
            CREATE_GLOBAL_SLF(get_num_free_slots__str, "get_num_free_slots(str)");
            CREATE_GLOBAL_SLF(get_num_mission_transform_marker, "get_num_mission_transform_marker()");
            CREATE_GLOBAL_SLF(get_pack_group__str, "get_pack_group(str)");
            CREATE_GLOBAL_SLF(get_pack_size__str, "get_pack_size(str)");
            CREATE_GLOBAL_SLF(get_patrol_difficulty__str, "get_patrol_difficulty(str)");
            CREATE_GLOBAL_SLF(get_patrol_node_position_by_index__str__num, "get_patrol_node_position_by_index(str,num)");
            CREATE_GLOBAL_SLF(get_patrol_start_position__str, "get_patrol_start_position(str)");
            CREATE_GLOBAL_SLF(get_patrol_unlock_threshold__str, "get_patrol_unlock_threshold(str)");
            CREATE_GLOBAL_SLF(get_platform, "get_platform()");
            CREATE_GLOBAL_SLF(get_render_opt_num__str, "get_render_opt_num(str)");
            CREATE_GLOBAL_SLF(get_spider_reflexes_spiderman_time_dilation, "get_spider_reflexes_spiderman_time_dilation()");
            CREATE_GLOBAL_SLF(get_spider_reflexes_world_time_dilation, "get_spider_reflexes_world_time_dilation()");
            CREATE_GLOBAL_SLF(get_time_inc, "get_time_inc()");
            CREATE_GLOBAL_SLF(get_time_of_day, "get_time_of_day()");
            CREATE_GLOBAL_SLF(get_time_of_day_rate, "get_time_of_day_rate()");
            CREATE_GLOBAL_SLF(get_token_index_from_id__num__num, "get_token_index_from_id(num,num)");
            CREATE_GLOBAL_SLF(get_traffic_spawn_point_near_camera__vector3d_list, "get_traffic_spawn_point_near_camera(vector3d_list)");
            CREATE_GLOBAL_SLF(greater_than_or_equal_rounded__num__num, "greater_than_or_equal_rounded(num,num)");
            CREATE_GLOBAL_SLF(hard_break, "hard_break()");
            CREATE_GLOBAL_SLF(has_substring__str__str, "has_substring(str,str)");
            CREATE_GLOBAL_SLF(hero, "hero()");
            CREATE_GLOBAL_SLF(hero_exists, "hero_exists()");
            CREATE_GLOBAL_SLF(hero_type, "hero_type()");
            CREATE_GLOBAL_SLF(hide_controller_gauge, "hide_controller_gauge()");
            CREATE_GLOBAL_SLF(initialize_encounter_object, "initialize_encounter_object()");
            CREATE_GLOBAL_SLF(initialize_encounter_objects, "initialize_encounter_objects()");
            CREATE_GLOBAL_SLF(insert_pack__str, "insert_pack(str)");
            CREATE_GLOBAL_SLF(invoke_pause_menu_unlockables, "invoke_pause_menu_unlockables()");
            CREATE_GLOBAL_SLF(is_ai_enabled, "is_ai_enabled()");
            CREATE_GLOBAL_SLF(is_cut_scene_playing, "is_cut_scene_playing()");
            CREATE_GLOBAL_SLF(is_district_loaded__num, "is_district_loaded(num)");
            CREATE_GLOBAL_SLF(is_hero_frozen, "is_hero_frozen()");
            CREATE_GLOBAL_SLF(is_hero_peter_parker, "is_hero_peter_parker()");
            CREATE_GLOBAL_SLF(is_hero_spidey, "is_hero_spidey()");
            CREATE_GLOBAL_SLF(is_hero_venom, "is_hero_venom()");
            CREATE_GLOBAL_SLF(is_marky_cam_enabled, "is_marky_cam_enabled()");
            CREATE_GLOBAL_SLF(is_mission_active, "is_mission_active()");
            CREATE_GLOBAL_SLF(is_mission_loading, "is_mission_loading()");
            CREATE_GLOBAL_SLF(is_pack_available__str, "is_pack_available(str)");
            CREATE_GLOBAL_SLF(is_pack_loaded__str, "is_pack_loaded(str)");
            CREATE_GLOBAL_SLF(is_pack_pushed__str, "is_pack_pushed(str)");
            CREATE_GLOBAL_SLF(is_path_graph_inside_glass_house__str, "is_path_graph_inside_glass_house(str)");
            CREATE_GLOBAL_SLF(is_patrol_active, "is_patrol_active()");
            CREATE_GLOBAL_SLF(is_patrol_node_empty__num, "is_patrol_node_empty(num)");
            CREATE_GLOBAL_SLF(is_paused, "is_paused()");
            CREATE_GLOBAL_SLF(is_physics_enabled, "is_physics_enabled()");
            CREATE_GLOBAL_SLF(is_point_inside_glass_house__vector3d, "is_point_inside_glass_house(vector3d)");
            CREATE_GLOBAL_SLF(is_point_under_water__vector3d, "is_point_under_water(vector3d)");
            CREATE_GLOBAL_SLF(is_user_camera_enabled, "is_user_camera_enabled()");
            CREATE_GLOBAL_SLF(load_anim__str, "load_anim(str)");
            CREATE_GLOBAL_SLF(load_level__str__vector3d, "load_level(str,vector3d)");
            CREATE_GLOBAL_SLF(lock_all_districts, "lock_all_districts()");
            CREATE_GLOBAL_SLF(lock_district__num, "lock_district(num)");
            CREATE_GLOBAL_SLF(lock_mission_manager__num, "lock_mission_manager(num)");
            CREATE_GLOBAL_SLF(los_check__vector3d__vector3d, "los_check(vector3d,vector3d)");
            CREATE_GLOBAL_SLF(lower_hotpursuit_indicator_level, "lower_hotpursuit_indicator_level()");
            CREATE_GLOBAL_SLF(malor__vector3d__num, "malor(vector3d,num)");
            CREATE_GLOBAL_SLF(normal__vector3d, "normal(vector3d)");
            CREATE_GLOBAL_SLF(pause_game__num, "pause_game(num)");
            CREATE_GLOBAL_SLF(play_credits, "play_credits()");
            CREATE_GLOBAL_SLF(play_prerender__str, "play_prerender(str)");
            CREATE_GLOBAL_SLF(pop_pack__str, "pop_pack(str)");
            CREATE_GLOBAL_SLF(post_message__str__num, "post_message(str,num)");
            CREATE_GLOBAL_SLF(pre_roll_all_pfx__num, "pre_roll_all_pfx(num)");
            CREATE_GLOBAL_SLF(press_controller_gauge__num, "press_controller_gauge(num)");
            CREATE_GLOBAL_SLF(press_controller_gauge__num__num__num, "press_controller_gauge(num,num,num)");
            CREATE_GLOBAL_SLF(purge_district__num, "purge_district(num)");
            CREATE_GLOBAL_SLF(push_pack__str, "push_pack(str)");
            CREATE_GLOBAL_SLF(push_pack_into_district_slot__str, "push_pack_into_district_slot(str)");
            CREATE_GLOBAL_SLF(raise_hotpursuit_indicator_level, "raise_hotpursuit_indicator_level()");
            CREATE_GLOBAL_SLF(random__num, "random(num)");
            CREATE_GLOBAL_SLF(remove_civilian_info__num, "remove_civilian_info(num)");
            CREATE_GLOBAL_SLF(remove_civilian_info_entity__entity__num, "remove_civilian_info_entity(entity,num)");
            CREATE_GLOBAL_SLF(remove_glass_house__str, "remove_glass_house(str)");
            CREATE_GLOBAL_SLF(remove_item_entity_from_world__entity, "remove_item_entity_from_world(entity)");
            CREATE_GLOBAL_SLF(remove_pack__str, "remove_pack(str)");
            CREATE_GLOBAL_SLF(remove_traffic_model__num, "remove_traffic_model(num)");
            CREATE_GLOBAL_SLF(reset_externed_alses, "reset_externed_alses()");
            CREATE_GLOBAL_SLF(set_all_anchors_activated__num, "set_all_anchors_activated(num)");
            CREATE_GLOBAL_SLF(set_blur__num, "set_blur(num)");
            CREATE_GLOBAL_SLF(set_blur_blend_mode__num, "set_blur_blend_mode(num)");
            CREATE_GLOBAL_SLF(set_blur_color__vector3d, "set_blur_color(vector3d)");
            CREATE_GLOBAL_SLF(set_blur_offset__num__num, "set_blur_offset(num,num)");
            CREATE_GLOBAL_SLF(set_blur_rot__num, "set_blur_rot(num)");
            CREATE_GLOBAL_SLF(set_blur_scale__num__num, "set_blur_scale(num,num)");
            CREATE_GLOBAL_SLF(set_clear_color__vector3d, "set_clear_color(vector3d)");
            CREATE_GLOBAL_SLF(set_current_mission_objective_caption__num, "set_current_mission_objective_caption(num)");
            CREATE_GLOBAL_SLF(set_default_traffic_hitpoints__num, "set_default_traffic_hitpoints(num)");
            CREATE_GLOBAL_SLF(set_dialog_box_flavor__num, "set_dialog_box_flavor(num)");
            CREATE_GLOBAL_SLF(set_dialog_box_lockout_time__num, "set_dialog_box_lockout_time(num)");
            CREATE_GLOBAL_SLF(set_engine_property__str__num, "set_engine_property(str,num)");
            CREATE_GLOBAL_SLF(set_fov__num, "set_fov(num)");
            CREATE_GLOBAL_SLF(set_game_info_num__str__num, "set_game_info_num(str,num)");
            CREATE_GLOBAL_SLF(set_game_info_str__str__str, "set_game_info_str(str,str)");
            CREATE_GLOBAL_SLF(set_global_time_dilation__num, "set_global_time_dilation(num)");
            CREATE_GLOBAL_SLF(set_marky_cam_lookat__vector3d, "set_marky_cam_lookat(vector3d)");
            CREATE_GLOBAL_SLF(set_max_streaming_distance__num, "set_max_streaming_distance(num)");
            CREATE_GLOBAL_SLF(set_mission_key_pos_facing__vector3d__vector3d, "set_mission_key_pos_facing(vector3d,vector3d)");
            CREATE_GLOBAL_SLF(set_mission_key_position__vector3d, "set_mission_key_position(vector3d)");
            CREATE_GLOBAL_SLF(set_mission_text__num, "set_mission_text(num,...)");
            CREATE_GLOBAL_SLF(set_mission_text_box_flavor__num, "set_mission_text_box_flavor(num)");
            CREATE_GLOBAL_SLF(set_mission_text_debug__str, "set_mission_text_debug(str)");
            CREATE_GLOBAL_SLF(set_parking_density__num, "set_parking_density(num)");
            CREATE_GLOBAL_SLF(set_pedestrian_density__num, "set_pedestrian_density(num)");
            CREATE_GLOBAL_SLF(set_render_opt_num__str__num, "set_render_opt_num(str,num)");
            CREATE_GLOBAL_SLF(set_score_widget_score__num, "set_score_widget_score(num)");
            CREATE_GLOBAL_SLF(set_sound_category_volume__num__num__num, "set_sound_category_volume(num,num,num)");
            CREATE_GLOBAL_SLF(set_spider_reflexes_blur__num, "set_spider_reflexes_blur(num)");
            CREATE_GLOBAL_SLF(set_spider_reflexes_blur_blend_mode__num, "set_spider_reflexes_blur_blend_mode(num)");
            CREATE_GLOBAL_SLF(set_spider_reflexes_blur_color__vector3d, "set_spider_reflexes_blur_color(vector3d)");
            CREATE_GLOBAL_SLF(set_spider_reflexes_blur_offset__num__num, "set_spider_reflexes_blur_offset(num,num)");
            CREATE_GLOBAL_SLF(set_spider_reflexes_blur_rot__num, "set_spider_reflexes_blur_rot(num)");
            CREATE_GLOBAL_SLF(set_spider_reflexes_blur_scale__num__num, "set_spider_reflexes_blur_scale(num,num)");
            CREATE_GLOBAL_SLF(set_spider_reflexes_hero_meter_depletion_rate__num, "set_spider_reflexes_hero_meter_depletion_rate(num)");
            CREATE_GLOBAL_SLF(set_spider_reflexes_spiderman_time_dilation__num, "set_spider_reflexes_spiderman_time_dilation(num)");
            CREATE_GLOBAL_SLF(set_spider_reflexes_world_time_dilation__num, "set_spider_reflexes_world_time_dilation(num)");
            CREATE_GLOBAL_SLF(set_state_of_the_story_caption__num, "set_state_of_the_story_caption(num)");
            CREATE_GLOBAL_SLF(set_target_info__entity__vector3d__vector3d, "set_target_info(entity,vector3d,vector3d)");
            CREATE_GLOBAL_SLF(set_time_of_day__num, "set_time_of_day(num)");
            CREATE_GLOBAL_SLF(set_traffic_density__num, "set_traffic_density(num)");
            CREATE_GLOBAL_SLF(set_traffic_model_usage__num__num, "set_traffic_model_usage(num,num)");
            CREATE_GLOBAL_SLF(set_vibration_resume__num, "set_vibration_resume(num)");
            CREATE_GLOBAL_SLF(set_whoosh_interp_rate__num, "set_whoosh_interp_rate(num)");
            CREATE_GLOBAL_SLF(set_whoosh_pitch_range__num__num, "set_whoosh_pitch_range(num,num)");
            CREATE_GLOBAL_SLF(set_whoosh_speed_range__num__num, "set_whoosh_speed_range(num,num)");
            CREATE_GLOBAL_SLF(set_whoosh_volume_range__num__num, "set_whoosh_volume_range(num,num)");
            CREATE_GLOBAL_SLF(set_zoom__num, "set_zoom(num)");
            CREATE_GLOBAL_SLF(show_controller_gauge, "show_controller_gauge()");
            CREATE_GLOBAL_SLF(show_hotpursuit_indicator__num, "show_hotpursuit_indicator(num)");
            CREATE_GLOBAL_SLF(show_score_widget__num, "show_score_widget(num)");
            CREATE_GLOBAL_SLF(shut_up_all_ai_voice_boxes, "shut_up_all_ai_voice_boxes()");
            CREATE_GLOBAL_SLF(sin__num, "sin(num)");
            CREATE_GLOBAL_SLF(sin_cos__num, "sin_cos(num)");
            CREATE_GLOBAL_SLF(soft_load__num, "soft_load(num)");
            CREATE_GLOBAL_SLF(soft_save__num, "soft_save(num)");
            CREATE_GLOBAL_SLF(spiderman_add_hero_points__num, "spiderman_add_hero_points(num)");
            CREATE_GLOBAL_SLF(spiderman_bank_stylepoints, "spiderman_bank_stylepoints()");
            CREATE_GLOBAL_SLF(spiderman_break_web, "spiderman_break_web()");
            CREATE_GLOBAL_SLF(spiderman_camera_add_shake__num__num__num, "spiderman_camera_add_shake(num,num,num)");
            CREATE_GLOBAL_SLF(spiderman_camera_autocorrect__num, "spiderman_camera_autocorrect(num)");
            CREATE_GLOBAL_SLF(spiderman_camera_clear_fixedstatic, "spiderman_camera_clear_fixedstatic()");
            CREATE_GLOBAL_SLF(spiderman_camera_enable_combat__num, "spiderman_camera_enable_combat(num)");
            CREATE_GLOBAL_SLF(spiderman_camera_enable_lookaround__num, "spiderman_camera_enable_lookaround(num)");
            CREATE_GLOBAL_SLF(spiderman_camera_set_fixedstatic__vector3d__vector3d, "spiderman_camera_set_fixedstatic(vector3d,vector3d)");
            CREATE_GLOBAL_SLF(spiderman_camera_set_follow__entity, "spiderman_camera_set_follow(entity)");
            CREATE_GLOBAL_SLF(spiderman_camera_set_hero_underwater__num, "spiderman_camera_set_hero_underwater(num)");
            CREATE_GLOBAL_SLF(spiderman_camera_set_interpolation_time__num, "spiderman_camera_set_interpolation_time(num)");
            CREATE_GLOBAL_SLF(spiderman_camera_set_lockon_min_distance__num, "spiderman_camera_set_lockon_min_distance(num)");
            CREATE_GLOBAL_SLF(spiderman_camera_set_lockon_y_offset__num, "spiderman_camera_set_lockon_y_offset(num)");
            CREATE_GLOBAL_SLF(spiderman_charged_jump, "spiderman_charged_jump()");
            CREATE_GLOBAL_SLF(spiderman_enable_control_button__num__num, "spiderman_enable_control_button(num,num)");
            CREATE_GLOBAL_SLF(spiderman_enable_lockon__num, "spiderman_enable_lockon(num)");
            CREATE_GLOBAL_SLF(spiderman_engage_lockon__num, "spiderman_engage_lockon(num)");
            CREATE_GLOBAL_SLF(spiderman_engage_lockon__num__entity, "spiderman_engage_lockon(num,entity)");
            CREATE_GLOBAL_SLF(spiderman_get_hero_points, "spiderman_get_hero_points()");
            CREATE_GLOBAL_SLF(spiderman_get_max_zip_length, "spiderman_get_max_zip_length()");
            CREATE_GLOBAL_SLF(spiderman_get_spidey_sense_level, "spiderman_get_spidey_sense_level()");
            CREATE_GLOBAL_SLF(spiderman_is_crawling, "spiderman_is_crawling()");
            CREATE_GLOBAL_SLF(spiderman_is_falling, "spiderman_is_falling()");
            CREATE_GLOBAL_SLF(spiderman_is_jumping, "spiderman_is_jumping()");
            CREATE_GLOBAL_SLF(spiderman_is_on_ceiling, "spiderman_is_on_ceiling()");
            CREATE_GLOBAL_SLF(spiderman_is_on_ground, "spiderman_is_on_ground()");
            CREATE_GLOBAL_SLF(spiderman_is_on_wall, "spiderman_is_on_wall()");
            CREATE_GLOBAL_SLF(spiderman_is_running, "spiderman_is_running()");
            CREATE_GLOBAL_SLF(spiderman_is_sprint_crawling, "spiderman_is_sprint_crawling()");
            CREATE_GLOBAL_SLF(spiderman_is_sprinting, "spiderman_is_sprinting()");
            CREATE_GLOBAL_SLF(spiderman_is_swinging, "spiderman_is_swinging()");
            CREATE_GLOBAL_SLF(spiderman_is_wallsprinting, "spiderman_is_wallsprinting()");
            CREATE_GLOBAL_SLF(spiderman_lock_spider_reflexes_off, "spiderman_lock_spider_reflexes_off()");
            CREATE_GLOBAL_SLF(spiderman_lock_spider_reflexes_on, "spiderman_lock_spider_reflexes_on()");
            CREATE_GLOBAL_SLF(spiderman_lockon_camera_engaged, "spiderman_lockon_camera_engaged()");
            CREATE_GLOBAL_SLF(spiderman_lockon_mode_engaged, "spiderman_lockon_mode_engaged()");
            CREATE_GLOBAL_SLF(spiderman_set_camera_target__entity, "spiderman_set_camera_target(entity)");
            CREATE_GLOBAL_SLF(spiderman_set_desired_mode__num__vector3d__vector3d, "spiderman_set_desired_mode(num,vector3d,vector3d)");
            CREATE_GLOBAL_SLF(spiderman_set_health_beep_min_max_cooldown_time__num__num, "spiderman_set_health_beep_min_max_cooldown_time(num,num)");
            CREATE_GLOBAL_SLF(spiderman_set_health_beep_threshold__num, "spiderman_set_health_beep_threshold(num)");
            CREATE_GLOBAL_SLF(spiderman_set_hero_meter_empty_rate__num, "spiderman_set_hero_meter_empty_rate(num)");
            CREATE_GLOBAL_SLF(spiderman_set_max_height__num, "spiderman_set_max_height(num)");
            CREATE_GLOBAL_SLF(spiderman_set_max_zip_length__num, "spiderman_set_max_zip_length(num)");
            CREATE_GLOBAL_SLF(spiderman_set_min_height__num, "spiderman_set_min_height(num)");
            CREATE_GLOBAL_SLF(spiderman_set_spidey_sense_level__num, "spiderman_set_spidey_sense_level(num)");
            CREATE_GLOBAL_SLF(spiderman_set_swing_anchor_max_sticky_time__num, "spiderman_set_swing_anchor_max_sticky_time(num)");
            CREATE_GLOBAL_SLF(spiderman_subtract_hero_points__num, "spiderman_subtract_hero_points(num)");
            CREATE_GLOBAL_SLF(spiderman_td_set_alternating_wall_run_occurrence_threshold__num, "spiderman_td_set_alternating_wall_run_occurrence_threshold(num)");
            CREATE_GLOBAL_SLF(spiderman_td_set_alternating_wall_run_time_threshold__num, "spiderman_td_set_alternating_wall_run_time_threshold(num)");
            CREATE_GLOBAL_SLF(spiderman_td_set_big_air_height_threshold__num, "spiderman_td_set_big_air_height_threshold(num)");
            CREATE_GLOBAL_SLF(spiderman_td_set_continuous_air_swings_threshold__num, "spiderman_td_set_continuous_air_swings_threshold(num)");
            CREATE_GLOBAL_SLF(spiderman_td_set_gain_altitude_height_threshold__num, "spiderman_td_set_gain_altitude_height_threshold(num)");
            CREATE_GLOBAL_SLF(spiderman_td_set_near_miss_trigger_radius__num, "spiderman_td_set_near_miss_trigger_radius(num)");
            CREATE_GLOBAL_SLF(spiderman_td_set_near_miss_velocity_threshold__num, "spiderman_td_set_near_miss_velocity_threshold(num)");
            CREATE_GLOBAL_SLF(spiderman_td_set_orbit_min_radius_threshold__num, "spiderman_td_set_orbit_min_radius_threshold(num)");
            CREATE_GLOBAL_SLF(spiderman_td_set_soft_landing_velocity_threshold__num, "spiderman_td_set_soft_landing_velocity_threshold(num)");
            CREATE_GLOBAL_SLF(spiderman_td_set_super_speed_speed_threshold__num, "spiderman_td_set_super_speed_speed_threshold(num)");
            CREATE_GLOBAL_SLF(spiderman_td_set_swinging_wall_run_time_threshold__num, "spiderman_td_set_swinging_wall_run_time_threshold(num)");
            CREATE_GLOBAL_SLF(spiderman_td_set_wall_sprint_time_threshold__num, "spiderman_td_set_wall_sprint_time_threshold(num)");
            CREATE_GLOBAL_SLF(spiderman_unlock_spider_reflexes, "spiderman_unlock_spider_reflexes()");
            CREATE_GLOBAL_SLF(spiderman_wait_add_threat__entity__str__num__num, "spiderman_wait_add_threat(entity,str,num,num)");
            CREATE_GLOBAL_SLF(spidey_can_see__vector3d, "spidey_can_see(vector3d)");
            CREATE_GLOBAL_SLF(sqrt__num, "sqrt(num)");
            CREATE_GLOBAL_SLF(start_patrol__str, "start_patrol(str)");
            CREATE_GLOBAL_SLF(stop_all_sounds, "stop_all_sounds()");
            CREATE_GLOBAL_SLF(stop_credits, "stop_credits()");
            CREATE_GLOBAL_SLF(stop_vibration, "stop_vibration()");
            CREATE_GLOBAL_SLF(subtitle__num__num__num__num__num__num, "subtitle(num,num,num,num,num,num)");
            CREATE_GLOBAL_SLF(swap_hero_costume__str, "swap_hero_costume(str)");
            CREATE_GLOBAL_SLF(text_width__str, "text_width(str)");
            CREATE_GLOBAL_SLF(timer_widget_get_count_up, "timer_widget_get_count_up()");
            CREATE_GLOBAL_SLF(timer_widget_get_time, "timer_widget_get_time()");
            CREATE_GLOBAL_SLF(timer_widget_set_count_up__num, "timer_widget_set_count_up(num)");
            CREATE_GLOBAL_SLF(timer_widget_set_time__num, "timer_widget_set_time(num)");
            CREATE_GLOBAL_SLF(timer_widget_start, "timer_widget_start()");
            CREATE_GLOBAL_SLF(timer_widget_stop, "timer_widget_stop()");
            CREATE_GLOBAL_SLF(timer_widget_turn_off, "timer_widget_turn_off()");
            CREATE_GLOBAL_SLF(timer_widget_turn_on, "timer_widget_turn_on()");
            CREATE_GLOBAL_SLF(to_beam__entity, "to_beam(entity)");
            CREATE_GLOBAL_SLF(to_gun__entity, "to_gun(entity)");
            CREATE_GLOBAL_SLF(to_item__entity, "to_item(entity)");
            CREATE_GLOBAL_SLF(to_polytube__entity, "to_polytube(entity)");
            CREATE_GLOBAL_SLF(to_switch__entity, "to_switch(entity)");
            CREATE_GLOBAL_SLF(trace__str, "trace(str)");
            CREATE_GLOBAL_SLF(trigger_is_valid__trigger, "trigger_is_valid(trigger)");
            CREATE_GLOBAL_SLF(turn_off_boss_health, "turn_off_boss_health()");
            CREATE_GLOBAL_SLF(turn_off_hero_health, "turn_off_hero_health()");
            CREATE_GLOBAL_SLF(turn_off_mission_text, "turn_off_mission_text()");
            CREATE_GLOBAL_SLF(turn_off_third_party_health, "turn_off_third_party_health()");
            CREATE_GLOBAL_SLF(turn_on_boss_health__num__entity, "turn_on_boss_health(num,entity)");
            CREATE_GLOBAL_SLF(turn_on_hero_health__num__entity, "turn_on_hero_health(num,entity)");
            CREATE_GLOBAL_SLF(turn_on_third_party_health__num__entity, "turn_on_third_party_health(num,entity)");
            CREATE_GLOBAL_SLF(unload_script, "unload_script()");
            CREATE_GLOBAL_SLF(unlock_all_exterior_districts, "unlock_all_exterior_districts()");
            CREATE_GLOBAL_SLF(unlock_district__num, "unlock_district(num)");
            CREATE_GLOBAL_SLF(vibrate_controller__num, "vibrate_controller(num)");
            CREATE_GLOBAL_SLF(vibrate_controller__num__num, "vibrate_controller(num,num)");
            CREATE_GLOBAL_SLF(vibrate_controller__num__num__num__num__num__num, "vibrate_controller(num,num,num,num,num,num)");
            CREATE_GLOBAL_SLF(vo_delay__num__num__num__num, "vo_delay(num,num,num,num)");
            CREATE_GLOBAL_SLF(wait_animate_fog_color__vector3d__num, "wait_animate_fog_color(vector3d,num)");
            CREATE_GLOBAL_SLF(wait_animate_fog_distance__num__num, "wait_animate_fog_distance(num,num)");
            CREATE_GLOBAL_SLF(wait_animate_fog_distances__num__num__num, "wait_animate_fog_distances(num,num,num)");
            CREATE_GLOBAL_SLF(wait_change_blur__num__vector3d__num__num__num__num__num__num, "wait_change_blur(num,vector3d,num,num,num,num,num,num)");
            CREATE_GLOBAL_SLF(wait_change_spider_reflexes_blur__num__vector3d__num__num__num__num__num__num, "wait_change_spider_reflexes_blur(num,vector3d,num,num,num,num,num,num)");
            CREATE_GLOBAL_SLF(wait_for_streamer_to_reach_equilibrium, "wait_for_streamer_to_reach_equilibrium()");
            CREATE_GLOBAL_SLF(wait_fps_test__num__num__vector3d__vector3d, "wait_fps_test(num,num,vector3d,vector3d)");
            CREATE_GLOBAL_SLF(wait_frame, "wait_frame()");
            CREATE_GLOBAL_SLF(wait_set_global_time_dilation__num__num, "wait_set_global_time_dilation(num,num)");
            CREATE_GLOBAL_SLF(wait_set_zoom__num__num, "wait_set_zoom(num,num)");
            CREATE_GLOBAL_SLF(write_to_file__str__str, "write_to_file(str,str)");
        }

#undef CREATE_GLOBAL_SLF

#define CREATE_SLF(KLASS, TYPE, NAME)                                              \
    new (mem_alloc(sizeof(BUILD_SLF_NAME(KLASS, TYPE)))) BUILD_SLF_NAME(KLASS, TYPE) {slc, NAME}

        script_library_class *slc = nullptr;
        {
            slc = classes[3];
            CREATE_SLF(beam, add_alpha_effect__num__num__num__num__num__num, "add_alpha_effect(num,num,num,num,num,num)");
            CREATE_SLF(beam, add_color_effect__num__num__num__num__num__num__num__num__num__num, "add_color_effect(num,num,num,num,num,num,num,num,num,num)");
            CREATE_SLF(beam, add_width_effect__num__num__num__num__num__num, "add_width_effect(num,num,num,num,num,num)");
            CREATE_SLF(beam, get_impact_normal, "get_impact_normal()");
            CREATE_SLF(beam, get_impact_point, "get_impact_point()");
            CREATE_SLF(beam, hit_hero, "hit_hero()");
            CREATE_SLF(beam, hit_world, "hit_world()");
            CREATE_SLF(beam, kill_all_effects__num, "kill_all_effects(num)");
            CREATE_SLF(beam, kill_effect__num__num, "kill_effect(num,num)");
            CREATE_SLF(beam, set_additive__num, "set_additive(num)");
            CREATE_SLF(beam, set_collide_beamable__num, "set_collide_beamable(num)");
            CREATE_SLF(beam, set_collide_hero__num, "set_collide_hero(num)");
            CREATE_SLF(beam, set_collide_world__num, "set_collide_world(num)");
            CREATE_SLF(beam, set_color__num__num__num__num, "set_color(num,num,num,num)");
            CREATE_SLF(beam, set_detect_stealth__num, "set_detect_stealth(num)");
            CREATE_SLF(beam, set_material__str, "set_material(str)");
            CREATE_SLF(beam, set_max_length__num, "set_max_length(num)");
            CREATE_SLF(beam, set_no_collision, "set_no_collision()");
            CREATE_SLF(beam, set_point_to_point__vector3d__vector3d, "set_point_to_point(vector3d,vector3d)");
            CREATE_SLF(beam, set_thickness__num, "set_thickness(num)");
            CREATE_SLF(beam, set_tiles_per_meter__num, "set_tiles_per_meter(num)");
            CREATE_SLF(beam, set_uv_anim__num__num, "set_uv_anim(num,num)");

            slc = classes[38];
            CREATE_SLF(critical_section, critical_section__num, "critical_section(num)");

            slc = classes[5];
            CREATE_SLF(cut_scene, wait_play, "wait_play()");
            CREATE_SLF(cut_scene, wait_play__entity_list, "wait_play(entity_list)");

            slc = classes[6];
            CREATE_SLF(debug_menu_entry, get_handler, "get_handler()");
            CREATE_SLF(debug_menu_entry, get_id, "get_id()");
            CREATE_SLF(debug_menu_entry, get_max_value, "get_max_value()");
            CREATE_SLF(debug_menu_entry, get_min_value, "get_min_value()");
            CREATE_SLF(debug_menu_entry, get_name, "get_name()");
            CREATE_SLF(debug_menu_entry, get_step_scale, "get_step_scale()");
            CREATE_SLF(debug_menu_entry, get_step_size, "get_step_size()");
            CREATE_SLF(debug_menu_entry, get_value, "get_value()");
            CREATE_SLF(debug_menu_entry, get_value_type, "get_value_type()");
            CREATE_SLF(debug_menu_entry, set_handler__str, "set_handler(str)");
            CREATE_SLF(debug_menu_entry, set_id__num, "set_id(num)");
            CREATE_SLF(debug_menu_entry, set_max_value__num, "set_max_value(num)");
            CREATE_SLF(debug_menu_entry, set_min_value__num, "set_min_value(num)");
            CREATE_SLF(debug_menu_entry, set_name__str, "set_name(str)");
            CREATE_SLF(debug_menu_entry, set_step_scale__num, "set_step_scale(num)");
            CREATE_SLF(debug_menu_entry, set_step_size__num, "set_step_size(num)");
            CREATE_SLF(debug_menu_entry, set_value__num, "set_value(num)");
            CREATE_SLF(debug_menu_entry, set_value_type__num, "set_value_type(num)");

            slc = classes[39];
            CREATE_SLF(district, contains_point__vector3d, "contains_point(vector3d)");
            CREATE_SLF(district, district__num, "district(num)");
            CREATE_SLF(district, district__str, "district(str)");
            CREATE_SLF(district, get_id, "get_id()");
            CREATE_SLF(district, get_name, "get_name()");
            CREATE_SLF(district, get_neighborhood, "get_neighborhood()");
            CREATE_SLF(district, get_num_missions__str, "get_num_missions(str)");
            CREATE_SLF(district, get_variant, "get_variant()");
            CREATE_SLF(district, get_variant_count, "get_variant_count()");
            CREATE_SLF(district, is_in_neighborhood__num, "is_in_neighborhood(num)");
            CREATE_SLF(district, is_loaded, "is_loaded()");
            CREATE_SLF(district, is_locked, "is_locked()");
            CREATE_SLF(district, lock, "lock()");
            CREATE_SLF(district, purge, "purge()");
            CREATE_SLF(district, set_variant__num, "set_variant(num)");
            CREATE_SLF(district, unlock, "unlock()");
            CREATE_SLF(district, wait_for_load, "wait_for_load()");

            slc = classes[4];

            slc = classes[7];
            CREATE_SLF(entity_list, add__entity, "add(entity)");
            CREATE_SLF(entity_list, begin, "begin()");
            CREATE_SLF(entity_list, end, "end()");
            CREATE_SLF(entity_list, get_index__num, "get_index(num)");
            CREATE_SLF(entity_list, remove__entity, "remove(entity)");
            CREATE_SLF(entity_list, remove__entity_list_iterator, "remove(entity_list_iterator)");
            CREATE_SLF(entity_list, set_index__num__entity, "set_index(num,entity)");
            CREATE_SLF(entity_list, size, "size()");

            slc = classes[8];
            CREATE_SLF(entity_list_iterator, get_entity, "get_entity()");
            CREATE_SLF(entity_list_iterator, operator_not_equals__entity_list_iterator, "operator!=(entity_list_iterator)");
            CREATE_SLF(entity_list_iterator, operator_plus_plus, "operator++()");
            CREATE_SLF(entity_list_iterator, operator_minus_munus, "operator--()");
            CREATE_SLF(entity_list_iterator, operator_equals_equals__entity_list_iterator, "operator==(entity_list_iterator)");

            slc = classes[9];
            CREATE_SLF(entity_tracker, get_entity, "get_entity()");
            CREATE_SLF(entity_tracker, get_mini_map_active, "get_mini_map_active()");
            CREATE_SLF(entity_tracker, get_poi_active, "get_poi_active()");
            CREATE_SLF(entity_tracker, set_entity__entity, "set_entity(entity)");
            CREATE_SLF(entity_tracker, set_health_widget_active__num, "set_health_widget_active(num)");
            CREATE_SLF(entity_tracker, set_mini_map_active__num, "set_mini_map_active(num)");
            CREATE_SLF(entity_tracker, set_poi_active__num, "set_poi_active(num)");
            CREATE_SLF(entity_tracker, set_poi_icon__num, "set_poi_icon(num)");

            slc = classes[10];
            CREATE_SLF(glamour_cam, set_angle__num__num__num__num, "set_angle(num,num,num,num)");
            CREATE_SLF(glamour_cam, set_look_at_bone__str, "set_look_at_bone(str)");
            CREATE_SLF(glamour_cam, set_position_bone__str, "set_position_bone(str)");
            CREATE_SLF(glamour_cam, set_target__entity, "set_target(entity)");

            slc = classes[11];
            CREATE_SLF(gun, get_blaster_beam, "get_blaster_beam()");
            CREATE_SLF(gun, get_muzzle_pos, "get_muzzle_pos()");
            CREATE_SLF(gun, set_target_info__entity__vector3d__vector3d, "set_target_info(entity,vector3d,vector3d)");

            slc = classes[1];
            CREATE_SLF(interactable_interface, add_box_trigger__string_hash__num__vector3d__vector3d, "add_box_trigger(string_hash,num,vector3d,vector3d)");
            CREATE_SLF(interactable_interface, add_named_box_trigger__string_hash__str, "add_named_box_trigger(string_hash,str)");
            CREATE_SLF(interactable_interface, add_point_trigger__string_hash__num__vector3d__num, "add_point_trigger(string_hash,num,vector3d,num)");
            CREATE_SLF(interactable_interface, create_interaction__string_hash__num, "create_interaction(string_hash,num)");
            CREATE_SLF(interactable_interface, create_interaction__string_hash__num__str, "create_interaction(string_hash,num,str)");
            CREATE_SLF(interactable_interface, disable_interaction__string_hash, "disable_interaction(string_hash)");
            CREATE_SLF(interactable_interface, enable_interaction__string_hash, "enable_interaction(string_hash)");
            CREATE_SLF(interactable_interface, nondirectional__string_hash, "nondirectional(string_hash)");
            CREATE_SLF(interactable_interface, one_shot__string_hash, "one_shot(string_hash)");
            CREATE_SLF(interactable_interface, set_activation_button__string_hash__num, "set_activation_button(string_hash,num)");
            CREATE_SLF(interactable_interface, set_approach__string_hash__num, "set_approach(string_hash,num)");

            slc = classes[12];
            CREATE_SLF(item, draw, "draw()");
            CREATE_SLF(item, get_count, "get_count()");
            CREATE_SLF(item, get_last_grenade_armed, "get_last_grenade_armed()");
            CREATE_SLF(item, get_last_grenade_detonated, "get_last_grenade_detonated()");
            CREATE_SLF(item, get_last_grenade_spawned, "get_last_grenade_spawned()");
            CREATE_SLF(item, get_launch_force, "get_launch_force()");
            CREATE_SLF(item, get_launch_vec, "get_launch_vec()");
            CREATE_SLF(item, get_owner, "get_owner()");
            CREATE_SLF(item, get_visual_item, "get_visual_item()");
            CREATE_SLF(item, holster, "holster()");
            CREATE_SLF(item, is_a_grenade, "is_a_grenade()");
            CREATE_SLF(item, is_a_gun, "is_a_gun()");
            CREATE_SLF(item, is_a_melee, "is_a_melee()");
            CREATE_SLF(item, is_a_rocket, "is_a_rocket()");
            CREATE_SLF(item, set_count__num, "set_count(num)");
            CREATE_SLF(item, set_damage__num, "set_damage(num)");
            CREATE_SLF(item, set_drawn_limb__str, "set_drawn_limb(str)");
            CREATE_SLF(item, set_holster_limb__str, "set_holster_limb(str)");
            CREATE_SLF(item, set_launch_force__num, "set_launch_force(num)");
            CREATE_SLF(item, set_launch_vec__vector3d, "set_launch_vec(vector3d)");
            CREATE_SLF(item, use, "use()");
            CREATE_SLF(item, yank_remove__vector3d__num, "yank_remove(vector3d,num)");

            slc = classes[13];
            CREATE_SLF(line_info, check_collision__num__num, "check_collision(num,num)");
            CREATE_SLF(line_info, clear_collision, "clear_collision()");
            CREATE_SLF(line_info, debug_render__num, "debug_render(num)");
            CREATE_SLF(line_info, did_collide, "did_collide()");
            CREATE_SLF(line_info, get_end_pos, "get_end_pos()");
            CREATE_SLF(line_info, get_hit_entity, "get_hit_entity()");
            CREATE_SLF(line_info, get_hit_normal, "get_hit_normal()");
            CREATE_SLF(line_info, get_hit_pos, "get_hit_pos()");
            CREATE_SLF(line_info, get_start_pos, "get_start_pos()");
            CREATE_SLF(line_info, set_end_pos__vector3d, "set_end_pos(vector3d)");
            CREATE_SLF(line_info, set_start_pos__vector3d, "set_start_pos(vector3d)");

            slc = classes[14];
            CREATE_SLF(mission_camera_marker, get_base_position, "get_base_position()");
            CREATE_SLF(mission_camera_marker, get_camera_position, "get_camera_position()");

            slc = classes[15];
            CREATE_SLF(mission_camera_transform_marker, get_base_position, "get_base_position()");
            CREATE_SLF(mission_camera_transform_marker, get_camera_position, "get_camera_position()");
            CREATE_SLF(mission_camera_transform_marker, get_forward, "get_forward()");
            CREATE_SLF(mission_camera_transform_marker, get_up, "get_up()");

            slc = classes[16];
            CREATE_SLF(mission_transform_marker, get_base_position, "get_base_position()");
            CREATE_SLF(mission_transform_marker, get_forward, "get_forward()");
            CREATE_SLF(mission_transform_marker, get_up, "get_up()");

            slc = classes[17];
            CREATE_SLF(num_list, add__num, "add(num)");
            CREATE_SLF(num_list, begin, "begin()");
            CREATE_SLF(num_list, end, "end()");
            CREATE_SLF(num_list, get_index__num, "get_index(num)");
            CREATE_SLF(num_list, remove__num, "remove(num)");
            CREATE_SLF(num_list, remove__num_list_iterator, "remove(num_list_iterator)");
            CREATE_SLF(num_list, set_index__num__num, "set_index(num,num)");
            CREATE_SLF(num_list, size, "size()");

            slc = classes[18];
            CREATE_SLF(num_list_iterator, get_num, "get_num()");
            CREATE_SLF(num_list_iterator, operator_not_equals__num_list_iterator, "operator!=(num_list_iterator)");
            CREATE_SLF(num_list_iterator, operator_plus_plus, "operator++()");
            CREATE_SLF(num_list_iterator, operator_minus_munus, "operator--()");
            CREATE_SLF(num_list_iterator, operator_equals_equals__num_list_iterator, "operator==(num_list_iterator)");

            slc = classes[0];
            CREATE_SLF(pfx, get_abs_position, "get_abs_position()");
            CREATE_SLF(pfx, get_parent, "get_parent()");
            CREATE_SLF(pfx, get_position, "get_position()");
            CREATE_SLF(pfx, get_rel_position, "get_rel_position()");
            CREATE_SLF(pfx, get_rotation, "get_rotation()");
            CREATE_SLF(pfx, get_scale, "get_scale()");
            CREATE_SLF(pfx, get_state__num, "get_state(num)");
            CREATE_SLF(pfx, get_visible, "get_visible()");
            CREATE_SLF(pfx, set_abs_position__vector3d, "set_abs_position(vector3d)");
            CREATE_SLF(pfx, set_parent__entity, "set_parent(entity)");
            CREATE_SLF(pfx, set_position__vector3d, "set_position(vector3d)");
            CREATE_SLF(pfx, set_rel_position__vector3d, "set_rel_position(vector3d)");
            CREATE_SLF(pfx, set_rotation__vector3d, "set_rotation(vector3d)");
            CREATE_SLF(pfx, set_scale__vector3d, "set_scale(vector3d)");
            CREATE_SLF(pfx, set_state__num__num, "set_state(num,num)");
            CREATE_SLF(pfx, set_visible__num, "set_visible(num)");

            slc = classes[20];
            CREATE_SLF(polytube, add_control_pt__vector3d, "add_control_pt(vector3d)");
            CREATE_SLF(polytube, build__num__num, "build(num,num)");
            CREATE_SLF(polytube, clear, "clear()");
            CREATE_SLF(polytube, clear_simulations, "clear_simulations()");
            CREATE_SLF(polytube, create_tentacle, "create_tentacle()");
            CREATE_SLF(polytube, destroy_tentacle, "destroy_tentacle()");
            CREATE_SLF(polytube, get_control_pt__num, "get_control_pt(num)");
            CREATE_SLF(polytube, get_curve_pt__num, "get_curve_pt(num)");
            CREATE_SLF(polytube, get_ifl_frame, "get_ifl_frame()");
            CREATE_SLF(polytube, get_max_length, "get_max_length()");
            CREATE_SLF(polytube, get_num_control_pts, "get_num_control_pts()");
            CREATE_SLF(polytube, get_num_curve_pts, "get_num_curve_pts()");
            CREATE_SLF(polytube, get_num_sides, "get_num_sides()");
            CREATE_SLF(polytube, get_tiles_per_meter, "get_tiles_per_meter()");
            CREATE_SLF(polytube, get_tube_radius, "get_tube_radius()");
            CREATE_SLF(polytube, ifl_lock__num, "ifl_lock(num)");
            CREATE_SLF(polytube, ifl_play, "ifl_play()");
            CREATE_SLF(polytube, kill_pt_anim__num__num, "kill_pt_anim(num,num)");
            CREATE_SLF(polytube, rebuild, "rebuild()");
            CREATE_SLF(polytube, reserve_control_pts__num, "reserve_control_pts(num)");
            CREATE_SLF(polytube, set_additive__num, "set_additive(num)");
            CREATE_SLF(polytube, set_begin_material__str__num, "set_begin_material(str,num)");
            CREATE_SLF(polytube, set_begin_material_ifl__str__num, "set_begin_material_ifl(str,num)");
            CREATE_SLF(polytube, set_blend_mode__num, "set_blend_mode(num)");
            CREATE_SLF(polytube, set_control_pt__num__vector3d, "set_control_pt(num,vector3d)");
            CREATE_SLF(polytube, set_end_material__str__num, "set_end_material(str,num)");
            CREATE_SLF(polytube, set_end_material_ifl__str__num, "set_end_material_ifl(str,num)");
            CREATE_SLF(polytube, set_force_start__num, "set_force_start(num)");
            CREATE_SLF(polytube, set_material__str, "set_material(str)");
            CREATE_SLF(polytube, set_material_ifl__str, "set_material_ifl(str)");
            CREATE_SLF(polytube, set_max_length__num, "set_max_length(num)");
            CREATE_SLF(polytube, set_num_sides__num, "set_num_sides(num)");
            CREATE_SLF(polytube, set_pt_anim__num__vector3d__num__num, "set_pt_anim(num,vector3d,num,num)");
            CREATE_SLF(polytube, set_random_pt_anim__num__num__num__num, "set_random_pt_anim(num,num,num,num)");
            CREATE_SLF(polytube, set_start_v__num, "set_start_v(num)");
            CREATE_SLF(polytube, set_tiles_per_meter__num, "set_tiles_per_meter(num)");
            CREATE_SLF(polytube, set_tube_radius__num, "set_tube_radius(num)");
            CREATE_SLF(polytube, simulate_slack__vector3d__vector3d__num, "simulate_slack(vector3d,vector3d,num)");
            CREATE_SLF(polytube, simulate_tether__entity__entity__num, "simulate_tether(entity,entity,num)");
            CREATE_SLF(polytube, tentacle_add_dangle_engine__num__num, "tentacle_add_dangle_engine(num,num)");
            CREATE_SLF(polytube, tentacle_add_follow_engine__num__num__vector3d__num, "tentacle_add_follow_engine(num,num,vector3d,num)");
            CREATE_SLF(polytube, tentacle_add_slack_engine__num__num, "tentacle_add_slack_engine(num,num)");
            CREATE_SLF(polytube, tentacle_add_strike_engine__vector3d__num__num, "tentacle_add_strike_engine(vector3d,num,num)");
            CREATE_SLF(polytube, tentacle_add_swirl_strike_engine__vector3d__num__num__num__num, "tentacle_add_swirl_strike_engine(vector3d,num,num,num,num)");
            CREATE_SLF(polytube, tentacle_create_line__vector3d__num, "tentacle_create_line(vector3d,num)");
            CREATE_SLF(polytube, tentacle_engine_running__num, "tentacle_engine_running(num)");
            CREATE_SLF(polytube, tentacle_get_end_position, "tentacle_get_end_position()");
            CREATE_SLF(polytube, tentacle_get_engine_drag__num, "tentacle_get_engine_drag(num)");
            CREATE_SLF(polytube, tentacle_get_engine_gravity__num, "tentacle_get_engine_gravity(num)");
            CREATE_SLF(polytube, tentacle_get_engine_length__num, "tentacle_get_engine_length(num)");
            CREATE_SLF(polytube, tentacle_get_engine_slack_percent__num, "tentacle_get_engine_slack_percent(num)");
            CREATE_SLF(polytube, tentacle_get_num_positions, "tentacle_get_num_positions()");
            CREATE_SLF(polytube, tentacle_get_position__num, "tentacle_get_position(num)");
            CREATE_SLF(polytube, tentacle_kill_engine__num, "tentacle_kill_engine(num)");
            CREATE_SLF(polytube, tentacle_set_collide__num, "tentacle_set_collide(num)");
            CREATE_SLF(polytube, tentacle_set_end_position__vector3d, "tentacle_set_end_position(vector3d)");
            CREATE_SLF(polytube, tentacle_set_engine_attach_ent__num__entity, "tentacle_set_engine_attach_ent(num,entity)");
            CREATE_SLF(polytube, tentacle_set_engine_drag__num__num, "tentacle_set_engine_drag(num,num)");
            CREATE_SLF(polytube, tentacle_set_engine_gravity__num__vector3d, "tentacle_set_engine_gravity(num,vector3d)");
            CREATE_SLF(polytube, tentacle_set_engine_length__num__num, "tentacle_set_engine_length(num,num)");
            CREATE_SLF(polytube, tentacle_set_engine_slack_percent__num__num, "tentacle_set_engine_slack_percent(num,num)");
            CREATE_SLF(polytube, tentacle_set_engine_target__num__vector3d, "tentacle_set_engine_target(num,vector3d)");
            CREATE_SLF(polytube, tentacle_set_position__num__vector3d, "tentacle_set_position(num,vector3d)");
            CREATE_SLF(polytube, v_animate__num, "v_animate(num)");

            slc = classes[34];
            CREATE_SLF(posfacing3d, get_facing, "get_facing()");
            CREATE_SLF(posfacing3d, get_position, "get_position()");
            CREATE_SLF(posfacing3d, get_rotation, "get_rotation()");
            CREATE_SLF(posfacing3d, operator_not_equals__posfacing3d, "operator!=(posfacing3d)");
            CREATE_SLF(posfacing3d, operator_plus__posfacing3d, "operator+(posfacing3d)");
            CREATE_SLF(posfacing3d, operator_multiply__posfacing3d, "operator-(posfacing3d)");
            CREATE_SLF(posfacing3d, operator_equals_equals__posfacing3d, "operator==(posfacing3d)");
            CREATE_SLF(posfacing3d, posfacing3d__num__num__num__num, "posfacing3d(num,num,num,num)");
            CREATE_SLF(posfacing3d, posfacing3d__num__num__num__vector3d, "posfacing3d(num,num,num,vector3d)");
            CREATE_SLF(posfacing3d, posfacing3d__vector3d__num, "posfacing3d(vector3d,num)");
            CREATE_SLF(posfacing3d, posfacing3d__vector3d__vector3d, "posfacing3d(vector3d,vector3d)");

            slc = classes[33];
            CREATE_SLF(position3d, operator_assign__vector3d, "operator=(vector3d)");
            CREATE_SLF(position3d, position3d__num__num__num, "position3d(num,num,num)");

            slc = classes[21];
            CREATE_SLF(script_controller, is_button_pressed__num, "is_button_pressed(num)");
            CREATE_SLF(script_controller, is_down_pressed, "is_down_pressed()");
            CREATE_SLF(script_controller, is_left_pressed, "is_left_pressed()");
            CREATE_SLF(script_controller, is_lstick_down_pressed, "is_lstick_down_pressed()");
            CREATE_SLF(script_controller, is_lstick_left_pressed, "is_lstick_left_pressed()");
            CREATE_SLF(script_controller, is_lstick_right_pressed, "is_lstick_right_pressed()");
            CREATE_SLF(script_controller, is_lstick_up_pressed, "is_lstick_up_pressed()");
            CREATE_SLF(script_controller, is_right_pressed, "is_right_pressed()");
            CREATE_SLF(script_controller, is_rstick_down_pressed, "is_rstick_down_pressed()");
            CREATE_SLF(script_controller, is_rstick_left_pressed, "is_rstick_left_pressed()");
            CREATE_SLF(script_controller, is_rstick_right_pressed, "is_rstick_right_pressed()");
            CREATE_SLF(script_controller, is_rstick_up_pressed, "is_rstick_up_pressed()");
            CREATE_SLF(script_controller, is_up_pressed, "is_up_pressed()");

            slc = classes[22];
            CREATE_SLF(signaller, clear_callback__str, "clear_callback(str)");
            CREATE_SLF(signaller, clear_callbacks, "clear_callbacks()");

            slc = classes[23];
            CREATE_SLF(sound_inst, dampen_guard, "dampen_guard()");
            CREATE_SLF(sound_inst, fade_in__num__num, "fade_in(num,num)");
            CREATE_SLF(sound_inst, fade_out__num, "fade_out(num)");
            CREATE_SLF(sound_inst, fade_to__num__num, "fade_to(num,num)");
            CREATE_SLF(sound_inst, get_doppler, "get_doppler()");
            CREATE_SLF(sound_inst, get_entity, "get_entity()");
            CREATE_SLF(sound_inst, get_max_distance, "get_max_distance()");
            CREATE_SLF(sound_inst, get_min_distance, "get_min_distance()");
            CREATE_SLF(sound_inst, get_pitch, "get_pitch()");
            CREATE_SLF(sound_inst, get_source, "get_source()");
            CREATE_SLF(sound_inst, get_status, "get_status()");
            CREATE_SLF(sound_inst, get_volume, "get_volume()");
            CREATE_SLF(sound_inst, is_valid, "is_valid()");
            CREATE_SLF(sound_inst, play__num, "play(num)");
            CREATE_SLF(sound_inst, play_3d__vector3d__num, "play_3d(vector3d,num)");
            CREATE_SLF(sound_inst, queue__num, "queue(num)");
            CREATE_SLF(sound_inst, set_entity__entity, "set_entity(entity)");
            CREATE_SLF(sound_inst, set_preregistered_source__str, "set_preregistered_source(str)");
            CREATE_SLF(sound_inst, set_sound_group__str, "set_sound_group(str)");
            CREATE_SLF(sound_inst, set_source__str, "set_source(str)");
            CREATE_SLF(sound_inst, stop__num, "stop(num)");
            CREATE_SLF(sound_inst, wait__num, "wait(num)");

            slc = classes[24];
            CREATE_SLF(str_list, add__str, "add(str)");
            CREATE_SLF(str_list, begin, "begin()");
            CREATE_SLF(str_list, end, "end()");
            CREATE_SLF(str_list, get_index__num, "get_index(num)");
            CREATE_SLF(str_list, remove__str, "remove(str)");
            CREATE_SLF(str_list, remove__str_list_iterator, "remove(str_list_iterator)");
            CREATE_SLF(str_list, set_index__num__str, "set_index(num,str)");
            CREATE_SLF(str_list, size, "size()");

            slc = classes[25];
            CREATE_SLF(str_list_iterator, get_str, "get_str()");
            CREATE_SLF(str_list_iterator, operator_not_equals__str_list_iterator, "operator!=(str_list_iterator)");
            CREATE_SLF(str_list_iterator, operator_plus_plus, "operator++()");
            CREATE_SLF(str_list_iterator, operator_minus_munus, "operator--()");
            CREATE_SLF(str_list_iterator, operator_equals_equals__str_list_iterator, "operator==(str_list_iterator)");

            slc = classes[37];
            CREATE_SLF(string_hash, operator_equals_equals__str, "operator==(str)");
            CREATE_SLF(string_hash, operator_equals_equals__string_hash, "operator==(string_hash)");
            CREATE_SLF(string_hash, set__str, "set(str)");
            CREATE_SLF(string_hash, string_hash__str, "string_hash(str)");
            CREATE_SLF(string_hash, to_string, "to_string()");

            slc = classes[26];
            CREATE_SLF(switch_obj, get_state, "get_state()");
            CREATE_SLF(switch_obj, set_alarm__num, "set_alarm(num)");
            CREATE_SLF(switch_obj, set_single_use__num, "set_single_use(num)");
            CREATE_SLF(switch_obj, set_state__num, "set_state(num)");
            CREATE_SLF(switch_obj, set_web_targetable__num, "set_web_targetable(num)");
            CREATE_SLF(switch_obj, who_pressed_me, "who_pressed_me()");

            slc = classes[27];
            CREATE_SLF(tam, set_entity__entity, "set_entity(entity)");
            CREATE_SLF(tam, set_pos__vector3d, "set_pos(vector3d)");
            CREATE_SLF(tam, set_state__num, "set_state(num)");
            CREATE_SLF(tam, set_type__num, "set_type(num)");

            slc = classes[28];
            CREATE_SLF(taunt_entry, get_entity, "get_entity()");
            CREATE_SLF(taunt_entry, get_flags, "get_flags()");
            CREATE_SLF(taunt_entry, get_taunt_name, "get_taunt_name()");

            slc = classes[29];
            CREATE_SLF(taunt_exchange, get_entity_to_do_hurting, "get_entity_to_do_hurting()");
            CREATE_SLF(taunt_exchange, get_entity_to_hurt, "get_entity_to_hurt()");
            CREATE_SLF(taunt_exchange, get_last_hit_pt_value, "get_last_hit_pt_value()");
            CREATE_SLF(taunt_exchange, get_max_times_to_play, "get_max_times_to_play()");
            CREATE_SLF(taunt_exchange, get_min_damage, "get_min_damage()");
            CREATE_SLF(taunt_exchange, get_normalized_weight, "get_normalized_weight()");
            CREATE_SLF(taunt_exchange, get_taunt_entry__num, "get_taunt_entry(num)");
            CREATE_SLF(taunt_exchange, get_times_played, "get_times_played()");
            CREATE_SLF(taunt_exchange, get_total_taunts, "get_total_taunts()");
            CREATE_SLF(taunt_exchange, get_weight, "get_weight()");
            CREATE_SLF(taunt_exchange, increment_times_played, "increment_times_played()");
            CREATE_SLF(taunt_exchange, set_last_hit_pt_value__num, "set_last_hit_pt_value(num)");
            CREATE_SLF(taunt_exchange, set_normalized_weight__num, "set_normalized_weight(num)");

            slc = classes[30];
            CREATE_SLF(taunt_exchange_list, add__taunt_exchange, "add(taunt_exchange)");
            CREATE_SLF(taunt_exchange_list, clear, "clear()");
            CREATE_SLF(taunt_exchange_list, get_index__num, "get_index(num)");
            CREATE_SLF(taunt_exchange_list, size, "size()");

            slc = classes[31];
            CREATE_SLF(trigger, contains__entity, "contains(entity)");
            CREATE_SLF(trigger, get_abs_position, "get_abs_position()");
            CREATE_SLF(trigger, get_radius, "get_radius()");
            CREATE_SLF(trigger, get_triggered_ent, "get_triggered_ent()");
            CREATE_SLF(trigger, get_triggered_entity_list, "get_triggered_entity_list()");
            CREATE_SLF(trigger, is_point_radius_trigger, "is_point_radius_trigger()");
            CREATE_SLF(trigger, set_active__num, "set_active(num)");
            CREATE_SLF(trigger, set_multiple_entrance__num, "set_multiple_entrance(num)");
            CREATE_SLF(trigger, set_position__vector3d, "set_position(vector3d)");
            CREATE_SLF(trigger, set_radius__num, "set_radius(num)");
            CREATE_SLF(trigger, set_sees_dead_people__num, "set_sees_dead_people(num)");
            CREATE_SLF(trigger, set_use_any_char__num, "set_use_any_char(num)");

            slc = classes[32];
            CREATE_SLF(vector3d, length, "length()");
            CREATE_SLF(vector3d, length2, "length2()");
            CREATE_SLF(vector3d, operator_not_equals__vector3d, "operator!=(vector3d)");
            CREATE_SLF(vector3d, operator_multiply__num, "operator*(num)");
            CREATE_SLF(vector3d, operator_plus__vector3d, "operator+(vector3d)");
            CREATE_SLF(vector3d, operator_minus__vector3d, "operator-(vector3d)");
            CREATE_SLF(vector3d, operator_divide__num, "operator/(num)");
            CREATE_SLF(vector3d, operator_equals_equals__vector3d, "operator==(vector3d)");
            CREATE_SLF(vector3d, vector3d__num__num__num, "vector3d(num,num,num)");
            CREATE_SLF(vector3d, x, "x()");
            CREATE_SLF(vector3d, xy_norm, "xy_norm()");
            CREATE_SLF(vector3d, xz_norm, "xz_norm()");
            CREATE_SLF(vector3d, y, "y()");
            CREATE_SLF(vector3d, z, "z()");

            slc = classes[35];
            CREATE_SLF(vector3d_list, add__vector3d, "add(vector3d)");
            CREATE_SLF(vector3d_list, begin, "begin()");
            CREATE_SLF(vector3d_list, clear, "clear()");
            CREATE_SLF(vector3d_list, end, "end()");
            CREATE_SLF(vector3d_list, get_index__num, "get_index(num)");
            CREATE_SLF(vector3d_list, remove__vector3d, "remove(vector3d)");
            CREATE_SLF(vector3d_list, remove__vector3d_list_iterator, "remove(vector3d_list_iterator)");
            CREATE_SLF(vector3d_list, set_index__num__vector3d, "set_index(num,vector3d)");
            CREATE_SLF(vector3d_list, size, "size()");

            slc = classes[36];
            CREATE_SLF(vector3d_list_iterator, get_vector3d, "get_vector3d()");
            CREATE_SLF(vector3d_list_iterator, operator_not_equals__vector3d_list_iterator, "operator!=(vector3d_list_iterator)");
            CREATE_SLF(vector3d_list_iterator, operator_plus_plus, "operator++()");
            CREATE_SLF(vector3d_list_iterator, operator_minus_munus, "operator--()");
            CREATE_SLF(vector3d_list_iterator, operator_equals_equals__vector3d_list_iterator, "operator==(vector3d_list_iterator)");
        }

#undef CREATE_SLF

        slc_global()->add_functions_complete();

        for (auto &slc : classes) {
            slc->add_functions_complete();
        }
        
        if constexpr (0) {
            for (auto class_idx {0u}; class_idx < classes.size(); ++class_idx) {
                auto &slc = classes[class_idx];

                printf("funcs = [");
                for (auto func_idx {0u}; func_idx < slc->total_funcs; ++func_idx) {
                    auto &func = slc->funcs[func_idx];
                    printf("\"%s\", ", func->m_name);
                }

                printf("]\n");

                printf("slc = script_library_class(\"%s\", funcs)\nclasses.append(slc)\n", slc->name);
            }

            assert(0);
        }

    } else {
        CDECL_CALL(0x0064EB60);
    }
}

void slc_manager::init()
{
    TRACE("slc_manager::init");

    if constexpr (1) {
        if (slc_manager_classes == nullptr) {
            slc_manager_classes = new std::set<script_library_class *>{};
        }

        if (slc_manager_class_array() == nullptr) {
            using slc_manager_class_array_t = std::decay_t<decltype(*slc_manager_class_array())>;
            slc_manager_class_array() = new slc_manager_class_array_t {};
        }

        register_standard_script_libs();
        chuck_register_script_libs();
        construct_client_script_libs();
    }
    else
    {
        CDECL_CALL(0x005AD720);
    }
}

void slc_manager::add(script_library_class *slc)
{
    TRACE("slc_manager::add");

    assert(slc_manager_classes != nullptr);
    auto ret = slc_manager_classes->insert(slc);
    if ( !ret.second ) {
        auto name = slc->get_name();
        sp_log("slc already exists %s", name);
        assert(0);
    }

    assert(slc_manager_class_array() != nullptr);
    slc_manager_class_array()->push_back(slc);
}

void slc_manager::kill() {
    TRACE("slc_manager::kill");

    if constexpr (1) {
        destruct_client_script_libs();
        if ( slc_manager_class_array() != nullptr ) {
            for ( auto &slc : (*slc_manager_class_array()) ) {
                if ( slc != nullptr ) {
                    delete slc;
                }
            }

            if ( slc_manager_class_array() != nullptr ) {
                delete slc_manager_class_array();
            }

            slc_manager_class_array() = nullptr;
        }
    } else {
        CDECL_CALL(0x005A5200);
    }
}

script_library_class *slc_manager::get_class(int class_index)
{
    assert(slc_manager_class_array() != nullptr);

    assert(class_index >= 0);

    assert(class_index < slc_manager_class_array()->size());

    return slc_manager_class_array()->at(class_index);
}

script_library_class *slc_manager::get(const char *a1)
{
    assert(slc_manager_classes != nullptr);

    script_library_class v9{};
    v9.store_name(a1);
    auto it = slc_manager_classes->find(&v9);
    auto end = slc_manager_classes->end();
    if ( it != end ) {
        return (*it);
    }

    return nullptr;
}

void slc_manager::un_mash_all_funcs()
{
    TRACE("slc_manager::un_mash_all_funcs");

    assert(!script_manager::using_chuck_old_fashioned());
    assert(!g_is_the_packer());

    auto a1 = create_resource_key_from_path("all_slc_functions_mac", RESOURCE_KEY_TYPE_SLF_LIST);
    sp_log("%s", a1.get_platform_string(3).c_str());

    auto *image = bit_cast<char *>(resource_manager::get_resource(a1, nullptr, nullptr));
    assert(image != nullptr);

    assert(slc_manager_class_array() != nullptr);

    auto total_classes = bit_cast<int *>(image)[0];
    auto *buffer = image + 4;

    assert(total_classes == slc_manager_class_array()->size());
    
    for ( auto &slc : (*slc_manager_class_array()) ) {
        slc->total_funcs = bit_cast<int *>(buffer)[0];
        buffer += 4;
        assert(slc->funcs == nullptr);

        if ( slc->total_funcs > 0 ) {
            slc->funcs = CAST(slc->funcs, buffer);
            buffer += 4 * slc->total_funcs;
            slc->field_1C |= 1u;
        }
    }

    if constexpr (0) {
        uint32_t i {0};
        auto total_funcs = 0u;
        for (auto &slc : (*slc_manager_class_array())) {
            printf("slc: %u %s %u\n", i++, slc->get_name(), slc->total_funcs);
            total_funcs += slc->total_funcs;
        }

        printf("total_funcs: %d", total_funcs);

        assert(0);
    }
}

void slc_manager_patch()
{
    script_lib_entity_patch();

    {
        FUNC_ADDRESS(address, &slf__get_ini_flag__str__t::operator());
        set_vfunc(0x0089A950, address);
    }
    
    SET_JUMP(0x005AD720, slc_manager::init);

    SET_JUMP(0x005A5200, slc_manager::kill);

    SET_JUMP(0x005A5280, slc_manager::add);

    SET_JUMP(0x0059EC00, slc_manager::un_mash_all_funcs);

    REDIRECT(0x005AD778, chuck_register_script_libs);
}
