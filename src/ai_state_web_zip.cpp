#include "ai_state_web_zip.h"

#include "actor.h"
#include "ai_player_controller.h"
#include "ai_state_jump.h"
#include "ai_state_run.h"
#include "ai_state_swing.h"
#include "ai_std_hero.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "base_state.h"
#include "common.h"
#include "conglom.h"
#include "controller_inode.h"
#include "event.h"
#include "from_mash_in_place_constructor.h"
#include "func_wrapper.h"
#include "glass_house_manager.h"
#include "hit_react_state.h"
#include "oldmath_po.h"
#include <functional>
#include "physics_inode.h"
#include "polytube.h"
#include "state_machine.h"
#include "tentacle_interface.h"
#include "utility.h"
#include "vector2d.h"
#include "web_sounds.h"
#include "dangler.h"
#include "ngl.h"
#include "polytubecustommaterial.h"
#include "resource_manager.h"
#include "slab_allocator.h"
#include "variables.h"
#include "wds.h"
#include "game.h"
#include "game_settings.h"
#include "info_node_desc_list.h"
#include "physical_interface.h"
#include <algorithm>
#include <array>
#include <cmath>

#include <cassert>

namespace ai {

VALIDATE_SIZE(web_zip_state, 0x40);

VALIDATE_SIZE(web_zip_inode, 0xE0);

namespace {
void __fastcall zip_state_destroy(web_zip_state *self, void *) { self->finalize(mash::ALLOCATED); }
void *__fastcall zip_state_delete(web_zip_state *self, void *, unsigned flags)
{
    self->~web_zip_state();
    if (flags & 1)
        mash_virtual_base::operator delete(self, sizeof(web_zip_state));
    return self;
}
unsigned __fastcall zip_state_type(web_zip_state *, void *) { return 327; }
bool __fastcall zip_state_subclass(web_zip_state *, void *, mash::virtual_types_enum type)
{
    return type == static_cast<mash::virtual_types_enum>(535) ||
        type == static_cast<mash::virtual_types_enum>(567) ||
        type == static_cast<mash::virtual_types_enum>(573);
}
void __fastcall zip_state_activate(web_zip_state *self, void *, ai_state_machine *machine,
    const mashed_state *state, const mashed_state *previous, const param_block *params,
    base_state::activate_flag_e flags)
{
    self->activate(machine, state, previous, params, flags);
}
void __fastcall zip_state_deactivate(web_zip_state *self, void *, const mashed_state *state)
{
    self->deactivate(state);
}
state_trans_messages __fastcall zip_state_frame(web_zip_state *self, void *, Float dt)
{
    return self->frame_advance(dt);
}
void __fastcall zip_state_list(web_zip_state *self, void *, info_node_desc_list &list)
{
    self->get_info_node_list(list);
}
int __fastcall zip_state_size(web_zip_state *, void *) { return sizeof(web_zip_state); }
}

void *web_zip_state::native_vtable()
{

    static auto table = [] {
        std::array<void *, 16> result;
        std::copy_n(static_cast<void **>(enhanced_state::native_vtable()), result.size(), result.data());
        result[0] = reinterpret_cast<void *>(&zip_state_destroy);
        result[2] = reinterpret_cast<void *>(&zip_state_delete);
        result[3] = reinterpret_cast<void *>(&zip_state_type);
        result[4] = reinterpret_cast<void *>(&zip_state_subclass);
        result[6] = reinterpret_cast<void *>(&zip_state_activate);
        result[7] = reinterpret_cast<void *>(&zip_state_deactivate);
        result[8] = reinterpret_cast<void *>(&zip_state_frame);
        result[9] = reinterpret_cast<void *>(&zip_state_list);
        result[13] = reinterpret_cast<void *>(&zip_state_size);
        return result;
    }();
    return table.data();
}

web_zip_state::web_zip_state() : enhanced_state()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

web_zip_state::web_zip_state(from_mash_in_place_constructor *constructor) : enhanced_state(constructor)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

web_zip_state::~web_zip_state()
{
    finalize(mash::ALLOCATED);
}

void web_zip_state::get_info_node_list(info_node_desc_list &list)
{
    list.add_entry({web_zip_inode::default_id, static_cast<mash::virtual_types_enum>(326)});
}

void web_zip_state::finalize(mash::allocation_scope)
{
    if (!field_C)
        return;
    auto *zip = field_3C->field_3C;
    auto *owner = get_actor();
    if (owner && owner->has_physical_ifc()) {
        if (zip->m_zip_type == 2) {
            auto *physics = field_3C->field_28;
            auto velocity = physics->get_velocity();
            velocity.x = 0.0f;
            velocity.y = 0.0f;
            physics->set_velocity(velocity, false);
        }
        owner->physical_ifc()->field_C &= ~0x200u;
    }
    zip->release_zip_web();
}

state_trans_messages web_zip_state::frame_advance(Float dt)
{
    get_actor()->m_player_controller->set_spidey_loco_mode(eHeroLocoMode::WEB_ZIP);
    field_3C->field_3C->process_zip(dt);
    return TRANS_TOTAL_MSGS;
}

void web_zip_state::activate(ai_state_machine *machine, const mashed_state *state,
    const mashed_state *previous, const param_block *params, activate_flag_e flags)
{
    enhanced_state::activate(machine, state, previous, params, flags);
    field_3C = static_cast<hero_inode *>(get_core()->get_info_node(hero_inode::default_id, true));
    auto *animation = field_3C->field_20;
    auto *zip = field_3C->field_3C;
    animation->request_category_transition(string_hash{"Web_Zip"}, static_cast<als::layer_types>(0),
                                          true, false, true);
    if (!g_world_ptr->field_1B0.field_8.is_set()) {
        auto hero = static_cast<actor *>(g_world_ptr->get_hero_ptr(0))->get_player_controller()->m_hero_type;
        if (hero == SPIDEY || hero == PARKER)
            g_world_ptr->activate_web_splats();
    }
    auto *owner = get_actor();
    if (owner->is_a_conglomerate() && static_cast<conglomerate *>(owner)->has_tentacle_ifc())
        static_cast<conglomerate *>(owner)->tentacle_ifc()->begin_zip(zip->field_1C.hit_pos);
    owner->m_player_controller->set_spidey_loco_mode(eHeroLocoMode::WEB_ZIP);
    zip->field_7C = 0;
    if (zip->m_zip_type == 1) {
        field_3C->field_28->setup_for_crawl_zip();
        als::param_list desired;
        desired.add_param(0x27u, zip->field_1C.hit_pos);
        desired.add_param(0x18u, zip->field_1C.hit_norm);
        desired.add_param(als::param{0, get_core()->get_param_block()->get_pb_float(string_hash{"web_zip_speed"})});
        animation->set_desired_params(desired, static_cast<als::layer_types>(0));
    }
    field_30 = owner->get_abs_position();
    zip->field_80 = false;
    if (owner->has_sound_and_pfx_ifc())
        web_sounds_manager::add_web_sound(owner, zip->field_1C.hit_pos, string_hash{"ZIP"});
}

void web_zip_state::deactivate(const mashed_state *state)
{
    base_state::_deactivate(state);
    auto *owner = get_actor();
    if (owner && owner->has_physical_ifc() &&
        !get_core()->get_param_block()->get_pb_int(string_hash{"has_tentacle_zip"})) {

        g_game_ptr->gamefile->update_miles_web_zipping((owner->get_abs_position() - field_30).length());
    }
    if (static_cast<conglomerate *>(owner)->has_tentacle_ifc())
        static_cast<conglomerate *>(owner)->tentacle_ifc()->cancel_zip();
}

namespace {
void __fastcall web_zip_destruct(web_zip_inode *self, void *)
{
    self->field_1C.remove_to_collision_check_queue();
    for (auto &swingback : self->field_A4)
        swingback.~SpidermanLocoSwingBack();
    self->_destruct_mashed_class();
}

void __fastcall web_zip_unmash(web_zip_inode *self, void *,
                              mash_info_struct *info, void *context)
{
    self->unmash(info, context);
}

void *__fastcall web_zip_delete(web_zip_inode *self, void *, unsigned flags)
{
    self->~web_zip_inode();
    if (flags & 1)
        mash_virtual_base::operator delete(self, sizeof(web_zip_inode));
    return self;
}

unsigned __fastcall web_zip_type(web_zip_inode *, void *) { return 326; }
bool __fastcall web_zip_subclass(web_zip_inode *, void *, unsigned type)
{
    return type == 537 || type == 573;
}
bool __fastcall web_zip_needs_advance(web_zip_inode *, void *) { return true; }
void __fastcall web_zip_advance(web_zip_inode *self, void *, Float dt)
{
    self->frame_advance(dt);
}
void __fastcall web_zip_activate(web_zip_inode *self, void *, ai_core *core)
{
    self->activate(core);
}
void __fastcall web_zip_deactivate(web_zip_inode *self, void *)
{
    self->deactivate();
}
int __fastcall web_zip_size(web_zip_inode *, void *) { return sizeof(web_zip_inode); }
}

void *web_zip_inode::native_vtable()
{

    auto **base = static_cast<void **>(info_node::native_vtable());
    static void *table[] = {
        reinterpret_cast<void *>(&web_zip_destruct),
        reinterpret_cast<void *>(&web_zip_unmash),
        reinterpret_cast<void *>(&web_zip_delete),
        reinterpret_cast<void *>(&web_zip_type),
        reinterpret_cast<void *>(&web_zip_subclass), base[5],
        reinterpret_cast<void *>(&web_zip_needs_advance),
        reinterpret_cast<void *>(&web_zip_advance),
        reinterpret_cast<void *>(&web_zip_activate),
        reinterpret_cast<void *>(&web_zip_deactivate),
        base[10],
        reinterpret_cast<void *>(&web_zip_size),
    };
    return table;
}

web_zip_inode::web_zip_inode() : field_80(false)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[326]);
}

web_zip_inode::web_zip_inode(from_mash_in_place_constructor *a2)
    : info_node(a2), field_1C(a2)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[326]);
}

uint32_t web_zip_inode::get_virtual_type_enum()
{
    return 326;
}

void web_zip_inode::activate(ai_core *core)
{
    info_node::_activate(core);
    field_DC = static_cast<hero_inode *>(core->get_info_node(hero_inode::default_id, true));
    resource_manager::push_resource_context(field_C->get_resource_context());
    webline_texture = new PolytubeCustomMaterial{
        nglLoadTexture(tlFixedString{"spideywebstring"}), static_cast<nglBlendModeType>(2), 72};
    resource_manager::pop_resource_context();
    field_D4 = 0;
    for (auto &swingback : field_A4) {
        swingback.field_0 = field_C;
        void *memory = sizeof(polytube) <= slab_allocator::get_max_object_size()
            ? slab_allocator::allocate(sizeof(polytube), nullptr) : ::operator new(sizeof(polytube));
        auto *tube = new (memory) polytube{make_unique_entity_id(), 0};
        swingback.field_8 = tube;
        g_world_ptr->ent_mgr.add_dynamic_instanced_entity(tube);
        tube->set_render_color(color32{0xFFFFFFFF});
        if (std::not_equal_to<float>{}(tube->tube_radius, 0.1f)) {
            tube->tube_radius = 0.1f;
            tube->field_78 = false;
        }
        tube->tiles_per_meter = 1.5f;
        if (tube->num_sides != 2) {
            tube->num_sides = 2;
            tube->field_78 = false;
        }
        tube->max_length = 30.0f;
        tube->set_material(webline_texture);
        tube->field_D0->m_blend_mode = static_cast<nglBlendModeType>(2);
        tube->set_force_start(true);
        tube->reserve_control_pts(3);
        for (int i = 0; i < 3; ++i) {
            tube->add_control_pt(ZEROVEC);
        }
        tube->build(10, static_cast<spline::eSplineType>(3));
        tube->set_visible(false, false);
        swingback.field_C = false;
    }
    void *memory = sizeof(polytube) <= slab_allocator::get_max_object_size()
        ? slab_allocator::allocate(sizeof(polytube), nullptr) : ::operator new(sizeof(polytube));
    field_D8 = new (memory) polytube{make_unique_entity_id(), 0};
    g_world_ptr->ent_mgr.add_dynamic_instanced_entity(field_D8);
    field_D8->set_render_color(color32{0xFFFFFEFF});
    if (std::not_equal_to<float>{}(field_D8->tube_radius, 0.1f)) {
        field_D8->tube_radius = 0.1f;
        field_D8->field_78 = false;
    }
    field_D8->tiles_per_meter = 1.5f;
    if (field_D8->num_sides != 2) {
        field_D8->num_sides = 2;
        field_D8->field_78 = false;
    }
    field_D8->set_material(webline_texture);
    field_D8->force_regions(field_C);
    field_D8->set_force_start(true);
    field_D8->reserve_control_pts(2);
    for (int i = 0; i < 2; ++i) {
        field_D8->add_control_pt(ZEROVEC);
    }
    field_D8->build(10, static_cast<spline::eSplineType>(3));
    field_D8->set_visible(false, false);
}

void web_zip_inode::frame_advance(Float dt)
{
    for (auto &swingback : field_A4) {
        auto *tube = swingback.field_8;
        if (!tube->get_occluded_last_frame()) {
            if (swingback.field_C) {
                swingback.web_dangler->frame_advance(dt);
                swingback.web_dangler->build_polytube(tube);
            }
            if (std::not_equal_to<float>{}(tube->tube_radius, 0.1f)) {
                tube->tube_radius = 0.1f;
                tube->field_78 = false;
            }
        }
    }
}

bool web_zip_inode::can_go_to(string_hash state)
{
    if (state == hit_react_state::default_id)
        return false;
    const bool crawl = hero_inode::is_a_crawl_state(state, true);
    if (m_zip_type == 0 || m_zip_type == 2) {
        if (crawl || state == run_state::default_id)
            return false;
        return field_DC->field_20->get_als_layer(static_cast<als::layer_types>(0))
            ->get_time_to_signal(event::ANIM_ACTION) < 0.0f;
    }
    if (m_zip_type == 1) {
        const auto target = field_1C.hit_pos + field_1C.hit_norm * field_C->get_floor_offset();
        const float distance = (field_DC->field_28->get_abs_position() - target).length();
        const bool ground = field_1C.hit_norm.y > 0.5f;
        if (crawl)
            return !ground && distance < 1.3f;
        return state == run_state::default_id && ground &&
            field_C->physical_ifc()->get_floor_offset() + 0.1f > distance;
    }
    return false;
}

bool web_zip_inode::is_eligible(string_hash state)
{
    if (!field_8->get_param_block()->get_pb_int(string_hash{"loco_allow_web_zip"}))
        return false;
    const auto button = field_DC->field_24->get_button(static_cast<controller_inode::eControllerButton>(13));
    if (!button.is_triggered())
        return false;
    if (state == run_state::default_id) {
        m_zip_type = 0;
        return find_zip_anchor_and_transition_to_zip_jump({0});
    }
    if (state == jump_state::default_id) {
        m_zip_type = 2;
        return find_zip_anchor_and_transition_to_zip_jump({0});
    }
    if (hero_inode::is_a_crawl_state(state, true)) {
        m_zip_type = 1;
        return find_zip_anchor_from_crawl();
    }

    return true;
}

bool web_zip_inode::find_zip_anchor_from_crawl()
{
    auto *physics = field_DC->field_28;
    const float length = field_8->get_param_block()->get_pb_float(string_hash{"web_zip_from_crawl_max_length"});
    auto &hit = field_1C;
    auto check = [&] {
        hit.sub_48B410(100.0f);
        return hit.check_collision(*local_collision::entfilter_entity_no_capsules,
                                   *local_collision::obbfilter_lineseg_test, nullptr);
    };
    auto crawlable = [&] {
        return !is_noncrawlable_surface(hit) || hit.hit_norm.y > 0.732421875f;
    };
    bool accepted = false;
    bool first_collision = false;
    hit.clear();
    hit.field_0 = get_actor()->get_abs_position();
    hit.field_C = hit.field_0 + physics->get_z_facing() * length;
    if (check()) {
        accepted = crawlable() && correct_zip_target_pos(&hit);
        first_collision = true;
    }
    if (hit.collision && !accepted) {
        const auto start = hit.hit_pos + hit.hit_norm;
        hit.clear();
        hit.field_0 = start;
        hit.field_C = start - physics->get_y_facing() * 2.0f;
        accepted = check() && (physics->get_abs_position() - hit.hit_pos).length2() > 4.0f &&
            crawlable() && correct_zip_target_pos(&hit);
    }
    if (!accepted && !first_collision) {
        hit.clear();
        hit.field_0 = physics->get_abs_position() + physics->get_z_facing() * length;
        hit.field_C = hit.field_0 - physics->get_y_facing() * 2.0f;
        hit.field_0 += physics->get_y_facing();
        accepted = check() && !is_noncrawlable_surface(hit) && correct_zip_target_pos(&hit);
        if (!accepted) {
            hit.clear();
            hit.field_C = get_actor()->get_abs_position() - physics->get_y_facing();
            hit.field_0 = hit.field_C + physics->get_z_facing() * length;
            bool standing_clearance = false;
            if (check() && !is_noncrawlable_surface(hit)) {
                standing_clearance = hit.hit_norm.y > 0.732421875f &&
                    std::abs(dot(physics->get_y_facing(), hit.hit_norm)) < 0.35f;

                line_info clearance;
                clearance.field_C = hit.hit_pos + hit.hit_norm * 0.1f;
                clearance.field_0 = clearance.field_C + physics->get_y_facing() * 2.0f;
                hit.field_C = hit.hit_pos - hit.hit_norm * 0.5f;
                hit.field_0 = hit.field_C + physics->get_y_facing() * 2.0f;
                if (check()) {
                    if ((get_actor()->get_abs_position() - hit.hit_pos).length2() < 4.0f) {
                        hit.clear();
                        standing_clearance = false;
                    }
                } else {
                    standing_clearance = false;
                }
                if (standing_clearance) {
                    clearance.clear();
                    clearance.field_0 = hit.hit_pos + hit.hit_norm * 0.1f;
                    clearance.field_C = clearance.field_0 + YVEC * (get_actor()->get_render_scale().y * 1.9f);
                    clearance.sub_48B410(100.0f);
                    standing_clearance = !clearance.check_collision(*local_collision::entfilter_entity_no_capsules,
                        *local_collision::obbfilter_lineseg_test, nullptr);
                }
            }
            accepted = hit.collision && (standing_clearance || !is_noncrawlable_surface(hit)) &&
                correct_zip_target_pos(&hit);
        }
    }
    if (accepted) {
        auto forward = physics->get_z_facing();
        if (forward.y > 0.0f)
            forward = -forward;
        const auto position = get_actor()->get_abs_position();
        vector3d point, normal;
        find_intersection(position, position + forward * 6.0f, *local_collision::entfilter_entity_collision,
            *local_collision::obbfilter_lineseg_test, &point, &normal, nullptr, nullptr, nullptr, false);
        return true;
    }
    hit.clear();
    hit.field_0 = get_actor()->get_abs_position();
    hit.field_C = hit.field_0 - physics->get_y_facing() * 2.0f;
    check();
    return hit.collision && !is_noncrawlable_surface(hit);
}

bool web_zip_inode::find_zip_anchor_and_transition_to_zip_jump(eZipReattachMode)
{
    auto *physics = field_DC->field_28;
    auto *params = field_8->get_param_block();
    const float angle1 = params->get_pb_float(string_hash{"web_zip_angle1"}) * 0.017453292f;
    const float angle2 = params->get_pb_float(string_hash{"web_zip_angle2"}) * 0.017453292f;
    const float length = params->get_pb_float(string_hash{"web_zip_max_length"});
    const auto forward = physics->get_abs_po().get_z_facing();
    auto &hit = field_1C;
    auto check = [&] {
        hit.sub_48B410(100.0f);
        return hit.check_collision(*local_collision::entfilter_entity_no_capsules,
                                   *local_collision::obbfilter_lineseg_test, nullptr);
    };
    auto acceptable = [&] {
        const bool surface = m_zip_type == 1 ? !is_noncrawlable_surface(hit) :
            !g_world_ptr->is_point_under_water(hit.hit_pos);
        return (surface || hit.hit_norm.y > 0.732421875f) && correct_zip_target_pos(&hit);
    };
    hit.field_0 = get_actor()->get_abs_position();
    hit.field_C = hit.field_0 + forward * length;
    if (check())
        return acceptable();
    auto right = vector3d::cross(forward, YVEC);
    right.normalize();
    auto vertical = vector3d::cross(forward, right);
    vertical.normalize();
    const auto end = hit.field_C;
    for (int sample = 0; sample < 6; ++sample) {
        vector3d offset = ZEROVEC;
        switch (sample) {
        case 0: offset = vertical; break;
        case 1: offset = -vertical; break;
        case 2: offset = right; break;
        case 3: offset = -right; break;
        default:
            if (field_C->physical_ifc()->is_effectively_standing()) {
                if (sample == 4) {
                    hit.field_0 = get_actor()->get_abs_position() + YVEC;
                    offset = vertical * 2.0f;
                } else {
                    offset = vertical * 3.0f;
                }
            } else {
                const float angle = sample == 4 ? angle1 : angle2;
                offset = (forward * std::cos(angle) + vertical * std::sin(angle)) * length;
            }
            break;
        }
        hit.field_C = end + offset;
        if (check() && acceptable())
            return true;
    }
    return false;
}

bool web_zip_inode::correct_zip_target_pos(line_info *hit)
{
    if (!hit || !hit->collision || !glass_house_manager::is_point_in_glass_house(hit->hit_pos) ||
        hit->hit_pos.y < -10000.0f)
        return false;
    auto up = YVEC;
    if (is_colinear(up, hit->hit_norm, 0.01f))
        up = XVEC;
    po basis;
    basis.set_po(up, hit->hit_norm, hit->hit_pos);
    const auto point = hit->hit_pos + hit->hit_norm * 0.025f;
    vector3d correction_x = ZEROVEC, correction_z = ZEROVEC;
    if (!get_axis_correction_delta(point, basis.get_x_facing(), 0.5f, &correction_x) ||
        !get_axis_correction_delta(point, basis.get_z_facing(), 0.5f, &correction_z))
        return false;
    const auto correction = correction_x + correction_z;
    if (correction.length2() <= 0.0f)
        return true;
    const auto corrected = point + correction;
    vector3d intersection, normal;
    if (!find_intersection(corrected, corrected - hit->hit_norm * 0.1f,
            *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test,
            &intersection, &normal, nullptr, nullptr, nullptr, false))
        return false;
    hit->hit_pos += correction;
    if (auto *entity = hit->hit_entity.get_volatile_ptr())
        hit->field_30 = entity->get_abs_po().inverse_xform(hit->hit_pos);
    return true;
}

void web_zip_inode::deactivate()
{
    delete webline_texture;
    webline_texture = nullptr;
    for (auto &swingback : field_A4) {
        g_world_ptr->ent_mgr.destroy_entity(swingback.field_8);
        swingback.field_8 = nullptr;
    }
    if (field_C->is_a_conglomerate() &&
        static_cast<conglomerate *>(field_C)->has_tentacle_ifc()) {
        static_cast<conglomerate *>(field_C)->tentacle_ifc()->cancel_zip();
    }
    g_world_ptr->ent_mgr.destroy_entity(field_D8);
    field_D8 = nullptr;
}

void web_zip_inode::unmash(mash_info_struct *a2, void *a3)
{
    info_node::_unmash(a2, a3);
}

void web_zip_inode::process_zip(Float)
{
    auto *physics = field_DC->field_28;
    if (!field_8->get_param_block()->get_pb_int(string_hash{"has_tentacle_zip"})) {
        if (field_7C == 0 && field_C->event_raised_last_frame(event::ANIM_ACTION)) {
            entity_set_abs_po(field_D8, field_C->get_abs_po());
            const auto hero = static_cast<actor *>(g_world_ptr->get_hero_ptr(0))
                ->get_player_controller()->m_hero_type;
            if (hero == SPIDEY || hero == PARKER)
                swing_inode::do_web_splat(field_1C.hit_pos, field_1C.hit_norm,
                                         *local_collision::entfilter_entity_no_capsules);
            field_7C = 1;
        }
        if (field_7C == 1) {
            auto *bone = static_cast<conglomerate *>(field_C)->get_bone(string_hash{"BIP01 R HAND"}, true);
            auto hand = field_C->get_abs_po().inverse_xform(bone->get_abs_position());
            const auto scale = field_C->get_render_scale();
            hand.x *= scale.x;
            hand.y *= scale.y;
            hand.z *= scale.z;
            hand = field_C->get_abs_po().slow_xform(hand);
            field_D8->set_abs_control_pt(0, hand);
            field_D8->set_abs_control_pt(field_D8->get_num_control_pts() - 1, field_1C.hit_pos);
            if (field_D8->the_spline.need_rebuild)
                field_D8->the_spline.rebuild_helper();
            field_D8->unforce_regions();
            field_D8->force_regions(field_C);
            field_D8->set_visible(true, false);
        }
    }
    auto velocity = physics->get_velocity();
    const float speed = std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
    if (speed > 30.0f) {
        const float factor = 30.0f / speed;
        velocity.x *= factor;
        velocity.z *= factor;
        physics->set_velocity(velocity, false);
    }
}

void web_zip_inode::add_swingback(polytube *&a2, entity_base *a3, actor *a4)
{
    this->field_A4[this->field_D4].init(a2, a4, a3);
    ++this->field_D4;
    if (this->field_D4 >= 3) {
        this->field_D4 = 0;
    }
}

void web_zip_inode::release_zip_web()
{
    field_7C = 2;
    add_swingback(field_D8, field_1C.hit_entity.get_volatile_ptr(), field_C);
    field_D8->set_visible(false, false);
}

}  // namespace ai

void web_zip_state_patch()
{
#if !STANDALONE_SYSTEM
    FUNC_ADDRESS(address, &ai::web_zip_inode::is_eligible);
    REDIRECT(0x0048899E, address);
    REDIRECT(0x00488CBF, address);
    REDIRECT(0x00488F54, address);
#endif
}
