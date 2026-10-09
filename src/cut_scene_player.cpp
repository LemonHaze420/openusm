#include "cut_scene_player.h"

#include "ambient_audio_manager.h"
#include "app.h"
#include "comic_panels.h"
#include "cut_scene.h"
#include "cut_scene_segment.h"
#include "entity.h"
#include "entity_class_entry.h"
#include "event.h"
#include "event_manager.h"
#include "fe_health_widget.h"
#include "fe_mini_map_widget.h"
#include "femanager.h"
#include "game.h"
#include "glass_house_manager.h"
#include "igofrontend.h"
#include "igozoomoutmap.h"
#include "oldmath_po.h"
#include "os_developer_options.h"
#include "spiderman_camera.h"
#include "traffic.h"
#include "wds.h"
#include "common.h"
#include "func_wrapper.h"
#include "input_mgr.h"
#include "nal_system.h"
#include "resource_pack_standalone.h"
#include "stream_scene_anim.h"
#include "trace.h"
#include "utility.h"
#include "actor.h"
#include "ai_player_controller.h"
#include "comic_page_camera.h"
#include "mission_manager.h"
#include "physical_interface.h"
#include "region.h"
#include "resource_manager.h"
#include "resource_partition.h"
#include "resource_pack_slot.h"
#include "sound_manager.h"
#include "sound_source.h"
#include "terrain.h"
#include <cstring>
#include "ped_spawner.h"
#include "resource_location.h"
#include "ngl/shaders/us_lighting.h"
#include "tracking_panel.h"
#include "base_ai_core.h"
#include "ai_std_combat_target.h"
#include "ai_voice_box_inode.h"
#include "conglom.h"
#include "entity_viseme_entry.h"
#include "camera_setup_entry.h"
#include "game_camera.h"
#include "line_info.h"
#include "vehicle.h"
#include "script_memtrack.h"
#include "scene_anim.h"
#include "nal_generic.h"
#include "ngl.h"
#include "ngl_morph.h"
#include "variant_interface.h"
#include "glam_camera.h"


#include <cstdlib>
#include <algorithm>
#include <cmath>

VALIDATE_SIZE(cut_scene_player, 360u);
VALIDATE_OFFSET(cut_scene_player, panels, 0x8);
VALIDATE_OFFSET(cut_scene_player, animated_entities, 0x18);
VALIDATE_OFFSET(cut_scene_player, streams, 0x48);
VALIDATE_OFFSET(cut_scene_player, hidden_regions, 0x58);
VALIDATE_OFFSET(cut_scene_player, tracked_entities, 0x68);
VALIDATE_OFFSET(cut_scene_player, sound_inst, 0xB8);
VALIDATE_OFFSET(cut_scene_player, owned_camera, 0xCC);
VALIDATE_OFFSET(cut_scene_player, minimap_was_shown, 0xD4);
VALIDATE_OFFSET(cut_scene_player, peds_and_traffic_overridden, 0xD7);
VALIDATE_OFFSET(cut_scene_player, field_E4, 0xE4);
VALIDATE_SIZE(cut_scene_panel_state, 0xC);
VALIDATE_SIZE(cut_scene_panel_node, 0x14);
VALIDATE_SIZE(cut_scene_panel_tree, 0xC);

namespace {
Var<bool> fade_legos{0x009391EC};
Var<bool> saved_physics_enabled{0x009391ED};
Var<bool> saved_glass_house_enabled{0x009391EE};
Var<bool> cut_scene_camera_active{0x0096F7C0};
Var<bool> cut_scene_camera_transition{0x0096F7C1};
Var<float> frame_time_limit{0x00936414};
Var<float> cut_scene_time_inc{0x009682D0};
Var<bool> should_render_scene_anims{0x009682F0};

#if STANDALONE_SYSTEM
[[maybe_unused]] const bool retail_globals_initialized = [] {
    fade_legos() = true;
    saved_physics_enabled() = true;
    saved_glass_house_enabled() = true;
    frame_time_limit() = 0.06673340499401093f;
    return true;
}();
#endif

po restored_pose(const entity_class_entry &entry)
{
    po result{identity_matrix};
    constexpr float radians_per_degree = 0.017453292f;
    const vector3d angles = entry.field_34 * radians_per_degree;
    for (int axis = 0; axis != 3; ++axis) {
        if (std::fabs(angles[axis]) <= EPSILON)
            continue;
        po rotation{identity_matrix};
        if (axis == 0)
            rotation.set_rotate_x(Float{angles.x});
        else if (axis == 1)
            rotation.set_rotate_y(Float{angles.y});
        else
            rotation.set_rotate_z(Float{angles.z});
        ptr_to_po product{&result.m, &rotation.m};
        result.set_from_ptr_to_po_world(product);
    }
    result.set_position(entry.field_28);
    return result;
}

template <typename Function>
void visit_entity_entries(mAvlTree<entity_class_entry> &tree, Function function)
{
    auto *node = tree.root();
    while (node && node->m_left)
        node = node->m_left;
    for (mAvlTree<entity_class_entry>::iterator it{node}; it.field_0; ++it)
        function(*(*it).m_key);
}

actor *lip_sync_actor(entity_class_entry &entry)
{
    auto *entity = g_world_ptr->ent_mgr.get_entity(entry.field_0);
    if (!entity || !entity->is_an_actor())
        return nullptr;
    if ((entity->field_4 & 4) && !entry.field_54.empty()) {
        auto *member = static_cast<conglomerate *>(entity)->get_member(string_hash{entry.field_54.c_str()}, 1);
        if (member && member->is_an_actor())
            entity = static_cast<actor *>(member);
    }
    return static_cast<actor *>(entity);
}

ai_lip_sync *get_lip_sync(actor *actor)
{
    auto *core = actor->get_ai_core();
    auto *voice = core ? static_cast<ai::voice_box_inode *>(core->get_info_node(ai::voice_box_inode::default_id, false))
                       : nullptr;
    return voice ? voice->lip_sync : nullptr;
}

template <typename Function>
void visit_panel_states(cut_scene_panel_node *node, Function function)
{
    if (!node)
        return;
    visit_panel_states(node->left, function);
    function(*node->state);
    visit_panel_states(node->right, function);
}

void destroy_panel_nodes(cut_scene_panel_node *node, bool owns_panels)
{
    if (!node)
        return;
    destroy_panel_nodes(node->left, owns_panels);
    destroy_panel_nodes(node->right, owns_panels);
    if (owns_panels) {
        if (node->state->animation)
            node->state->animation->destroy();
        if (node->state->panel)
            comic_panels::release_panel(node->state->panel);
        delete node->state;
    }
    delete node;
}
struct morph_slider_anim : nalClientSceneAnim {
    tlFixedString entity_name;
    tlFixedString morph_name;
    tlFixedString member_name;
    tlFixedString suffix;
    nalGeneric::nalGenericPose *pose = nullptr;
    nalGeneric::nalGenericComponentHandle<nalGeneric::MorphSliderPoseTemplate<6>> handle;

    morph_slider_anim(const tlFixedString &entity, const tlFixedString &morph, const tlFixedString &member,
                      const tlFixedString &ending)
        : entity_name(entity), morph_name(morph), member_name(member), suffix(ending)
    {
        static nalClientSceneAnim::vtable table{reinterpret_cast<decltype(table.CreateInstance)>(create),
                                                reinterpret_cast<decltype(table.Advance)>(advance),
                                                reinterpret_cast<decltype(table.Render)>(render),
                                                reinterpret_cast<decltype(table.Release)>(release)};
        m_vtbl = &table;
    }
    static nalAnimClass<nalAnyPose>::nalInstanceClass *__fastcall create(morph_slider_anim *self, void *,
                                                                         nalAnimClass<nalAnyPose> *anim)
    {
        auto *generic = reinterpret_cast<nalGeneric::nalGenericAnim *>(anim);
        if (!self->pose) {
            self->pose = new (tlMemAlloc(sizeof(nalGeneric::nalGenericPose), 8, 0))
                nalGeneric::nalGenericPose{generic->field_30};
            tlFixedString name{"MorphedMesh"};
            tlFixedString component{"USMMorph"};
            generic->field_30->GetComponentHandle(self->handle, name, component);
        }
        return reinterpret_cast<nalAnimClass<nalAnyPose>::nalInstanceClass *>(generic->CreateInstance(nullptr));
    }
    static void __fastcall advance(morph_slider_anim *self, void *,
                                   nalAnimClass<nalAnyPose>::nalInstanceClass *instance, Float time, Float previous,
                                   Float, Float)
    {
        auto *generic = reinterpret_cast<nalGeneric::nalGenericInstance *>(instance);
        generic->GetPose(time, previous, *self->pose, generic->field_C->field_CC);
    }
    static void __fastcall render(morph_slider_anim *self, void *, nalAnimClass<nalAnyPose>::nalInstanceClass *, Float)
    {
        auto *value = g_world_ptr->ent_mgr.get_entity(string_hash{self->entity_name.to_string()});
        if (!value || !value->is_an_actor())
            return;
        auto *actor = static_cast<struct actor *>(value);
        if ((value->field_4 & 4) && *self->member_name.to_string()) {
            auto *member =
                static_cast<conglomerate *>(actor)->get_member(string_hash{self->member_name.to_string()}, 1);
            if (member && member->is_an_actor())
                actor = static_cast<struct actor *>(member);
        }
        auto *mesh = actor->get_mesh();
        const auto *weights = (*self->pose)[self->handle];
        mString morph{self->morph_name.to_string()};
        if (*self->suffix.to_string())
            morph.append(self->suffix.to_string());
        auto *set = actor->get_morph(tlFixedString{morph.c_str()}, false);
        if (!set)
            return;
        nglMorphFrame frames[6];
        nglMorphEntry entries[6];
        unsigned count = 0;
        for (; count < 6 && weights->frames[count] != 255; ++count) {
            if (weights->frames[count] >= set->NFrames)
                return;
            frames[count] = nglMorphFrame{&set->Frames[weights->frames[count]]};
            entries[count] = {weights->weights[count] * (1.0f / 255.0f), &frames[count]};
        }
        if (count && actor->field_90.field_5 > 1)
            nglBlendMorphs(mesh, count, entries);
    }
    static void __fastcall release(morph_slider_anim *self, void *)
    {
        if (self->pose) {
            self->pose->~nalGenericPose();
            tlMemFree(self->pose);
        }
        delete self;
    }
};
}  // namespace

cut_scene_player::cut_scene_player()
    : sound_inst(0), peds_and_traffic_overridden(false), field_E1(false), field_E4(static_cast<game_control_t>(96)),
      field_118(static_cast<game_control_t>(114)), field_14C(-1), field_154(-1.0f), field_158(-1.0f), field_164(0.0f)
{}

cut_scene_player::~cut_scene_player() = default;

cut_scene_panel_tree::~cut_scene_panel_tree()
{
    destroy_panel_nodes(root, owns_panels);
}

void cut_scene_player::restore_game_play_panel()
{
    auto *panel = comic_panels::game_play_panel();
    if (panel == nullptr)
        return;
    panel->field_67 = false;
    panel->field_4[3][0] = 320.0f;
    panel->field_4[3][1] = 240.0f;
    panel->field_4[3][2] = 0.0f;
    panel->m_size = vector2d{640.0f, 480.0f};
    panel->reset_gameplay_components();
    panel->field_50 = 1.0f;
}


void cut_scene_player::advance_lip_syncing(Float a2)
{
    visit_entity_entries((*current_segment)->field_84, [a2](entity_class_entry &entry) {
        auto *entity = g_world_ptr->ent_mgr.get_entity(entry.field_0);
        if (entity && entity->is_an_actor()) {
            if (auto *lip = get_lip_sync(static_cast<actor *>(entity)))
                lip->frame_advance(a2);
        }
    });
}

bool cut_scene_player::advance_panel_anims(Float a2)
{
    bool finished = true;
    visit_panel_states(panels.root, [&](cut_scene_panel_state &state) {
        if (state.animation && state.animation->is_playing()) {
            finished = false;
            state.animation->advance(a2);
            state.panel->set_loc(state.animation->get_loc());
            state.panel->set_size(state.animation->get_size());
            state.panel->set_gutter_rect(state.animation->get_gutter_rect());
        }
    });
    return finished;
}

void cut_scene_player::clean_up_finished_segment()
{
    if (!field_E1)
        return;
    field_E0 = false;
    fade_legos() = true;
    field_15C = field_160 = 0.0f;
    for (auto *instance : streams)
        instance->Destroy();
    streams.clear();
    auto next = current_segment;
    ++next;
    const bool last = next == current_cut_scene->segments.end();
    auto *segment = *current_segment;
    if (last || !segment->field_1)
        clear_panels();
    if (page_camera) {
        if (page_camera == comic_panels::cur_page_camera()) {
            page_camera->finalize(true);
            comic_panels::cur_page_camera() = nullptr;
        }
        page_camera = nullptr;
    }
    while (!acquired_entities.empty()) {
        auto *entity = acquired_entities.back();
        auto *entry = get_entity_entry(entity->field_10);
        if (entry->field_24) {
            if (entity->is_an_actor()) {
                const auto pose = entity->get_abs_po();
                entity->set_parent(nullptr);
                entity_set_abs_po(entity, pose);
                static_cast<actor *>(entity)->unbind_from_scene_anim(entry->field_18, entry->field_1C);
                if (entry->field_25)
                    entity_set_abs_po(entity, restored_pose(*entry));
            }
            restored_entities.push_back(entity_base_vhandle{entity->my_handle});
        } else {
            g_world_ptr->ent_mgr.release_entity(entity);
        }
        acquired_entities.pop_back();
    }
    while (!animated_entities.empty()) {
        auto *entity = static_cast<struct entity *>(animated_entities.back().get_volatile_ptr());
        if (entity && entity->is_an_actor()) {
            auto *entry = get_entity_entry(entity->field_10);
            const auto pose = entity->get_abs_po();
            entity->set_parent(nullptr);
            entity_set_abs_po(entity, pose);
            static_cast<actor *>(entity)->unbind_from_scene_anim(entry ? entry->field_18 : string_hash{},
                                                                 entry ? entry->field_1C : string_hash{});
            if (entry && entry->field_25) {
                entity->physical_ifc()->set_velocity(ZEROVEC, false);
                entity_set_abs_po(entity, restored_pose(*entry));
            }
        }
        animated_entities.pop_back();
    }
    if (last || (*next)->field_94 != string_hash{}) {
        if (auto *sound = sound_inst.get_sound_instance_ptr())
            sound->stop();
        ambient_audio_manager::set_max_playing_tracks(2);
    }
    stop_lip_syncing();
    if (segment->field_6) {
        if (auto *hero = static_cast<actor *>(g_world_ptr->get_hero_ptr(0))) {
            hero->unsuspend(true);
            hero->physical_ifc()->suspend(false);
            hero->physical_ifc()->enable(true);
            hero->get_player_controller()->unlock_controls(false);
            hero->get_player_controller()->clear_controls();
        }
    }
    var<void *>(0x0096F7C8) = nullptr;
    var<int>(0x0096F7CC) = 0;
    if (segment->field_AC >= 0) {
        g_world_ptr->the_terrain->unlock_district_pack_slot(segment->field_AC);
        segment->field_AC = -1;
    }
    for (auto *name : segment->field_34)
        g_world_ptr->the_terrain->show_region(g_world_ptr->the_terrain->find_region(string_hash{name->c_str()}));
    for (auto *region : hidden_regions)
        region->flags |= 1u;
    hidden_regions.clear();
    visit_entity_entries(segment->field_84, [](entity_class_entry &entry) {
        auto *entity = g_world_ptr->ent_mgr.get_entity(entry.field_0);
        if (entity && entry.field_25) {
            static_cast<actor *>(entity)->set_allow_tunnelling_into_next_frame(true);
            static_cast<actor *>(entity)->invalidate_frame_delta();
            entity->physical_ifc()->cancel_all_velocity();
            entity_set_abs_po(entity, restored_pose(entry));
        }
    });
}


void cut_scene_player::clean_up()
{
    stop(nullptr);
    clear_panels();
}

void cut_scene_player::clear_panels()
{
    destroy_panel_nodes(panels.root, panels.owns_panels);
    panels.root = nullptr;
    panels.count = 0;
}

cut_scene_panel_state *cut_scene_player::get_panel_entry(string_hash id, bool create)
{
    auto **link = &panels.root;
    cut_scene_panel_node *parent = nullptr;
    while (*link) {
        auto *node = *link;
        const auto key = node->state->id.source_hash_code;
        if (key == id.source_hash_code)
            return node->state;
        parent = node;
        link = id.source_hash_code < key ? &node->left : &node->right;
    }
    if (!create)
        return nullptr;
    auto *panel = comic_panels::acquire_panel(nullptr);
    panel->field_67 = true;
    auto *state = new cut_scene_panel_state{id, panel, nullptr};
    *link = new cut_scene_panel_node{nullptr, nullptr, parent, state, 1};
    ++panels.count;
    auto height = [](cut_scene_panel_node *node) {
        return node ? node->height : 0;
    };
    auto rotate = [&](cut_scene_panel_node *node, bool left) {
        auto *pivot = left ? node->right : node->left;
        auto *middle = left ? pivot->left : pivot->right;
        auto *up = node->parent;
        if (left) {
            node->right = middle;
            pivot->left = node;
        } else {
            node->left = middle;
            pivot->right = node;
        }
        if (middle)
            middle->parent = node;
        node->parent = pivot;
        pivot->parent = up;
        if (!up)
            panels.root = pivot;
        else if (up->left == node)
            up->left = pivot;
        else
            up->right = pivot;
        node->height = 1 + std::max(height(node->left), height(node->right));
        pivot->height = 1 + std::max(height(pivot->left), height(pivot->right));
        return pivot;
    };
    while (parent) {
        parent->height = 1 + std::max(height(parent->left), height(parent->right));
        const int balance = height(parent->left) - height(parent->right);
        if (balance > 1) {
            if (height(parent->left->left) < height(parent->left->right))
                rotate(parent->left, true);
            parent = rotate(parent, false);
        } else if (balance < -1) {
            if (height(parent->right->right) < height(parent->right->left))
                rotate(parent->right, false);
            parent = rotate(parent, true);
        }
        parent = parent->parent;
    }
    return state;
}

nalClientSceneAnim *cut_scene_player::find_scene_anim_entries(const tlFixedString &name)
{
    const char *text = name.to_string();
    if (!std::strncmp(text, "slider", 6)) {
        const tlFixedString entity_name{text + 6};
        tlFixedString morph_name{""}, member_name{""}, suffix{""};
        if (auto *entry = get_entity_entry(string_hash{text + 6})) {
            const char *path = entry->field_44.c_str();
            const char *base = std::strrchr(path, '\\');
            morph_name = tlFixedString{base ? base + 1 : path};
            member_name = tlFixedString{entry->field_54.c_str()};
            suffix = tlFixedString{entry->field_64.c_str()};
        }
        return new morph_slider_anim{entity_name, morph_name, member_name, suffix};
    }
    if (!std::strncmp(text, "panel", 5)) {
        if (field_14C >= 0 && field_150++ != field_14C)
            return nullptr;
        if (!std::strcmp(text, "panel_game")) {
            comic_panels::game_play_panel()->field_67 = false;
            return reinterpret_cast<nalClientSceneAnim *>(comic_panels::game_play_panel());
        }
        return reinterpret_cast<nalClientSceneAnim *>(get_panel_entry(string_hash{text}, true)->panel);
    }
    if (!std::strncmp(text, "pagecam", 7)) {
        page_camera = comic_panels::create_page_camera();
        page_camera->field_ED = true;
        return reinterpret_cast<nalClientSceneAnim *>(page_camera);
    }
    auto *entity = static_cast<struct entity *>(g_world_ptr->ent_mgr.get_entity(resolve_entity_name(text)));
    if (entity && entity->is_an_actor())
        animated_entities.push_back(entity->my_handle);
    else {
        entity = create_entity(create_resource_key_from_path(text, static_cast<resource_key_type>(0)));
        if (!entity)
            return nullptr;
    }
    if (auto *entry = get_entity_entry(entity->get_id()))
        if (entry->field_20 != string_hash{} && (entity->field_4 & 4) && entity->has_variant_ifc())
            entity->variant_ifc()->apply_variant(entry->field_20);
    if (entity->has_physical_ifc())
        entity->physical_ifc()->set_control_parent(nullptr);
    if (owned_camera)
        entity->set_parent(owned_camera);
    if (entity->is_renderable()) {
        entity->set_visible(false, false);
        entity->set_visible(true, false);
    }
    auto bind =
        reinterpret_cast<nalClientSceneAnim *(__fastcall *)(struct entity *, void *)>(get_vfunc(entity->m_vtbl, 0x218));
    return bind(entity, nullptr);
}

void cut_scene_player::setup_tracking_panels()
{
    auto *hero = g_world_ptr->get_hero_ptr(0);
    for (auto *definition : (*current_segment)->field_70) {
        auto *state = get_panel_entry(definition->field_18, false);
        auto *animation = new continuous_tracking_panel_anim{definition};
        auto *camera = comic_panels::glamour_cams[definition->camera_index];
        if (!camera) {
            delete animation;
            return;
        }
        if (state) {
            animation->initial_location = state->panel->get_loc();
            animation->initial_size = state->panel->m_size;
            animation->gutter_rect = state->panel->get_gutter_rect();
            state->panel->add_color_component({0.1f, 0.0f, 0.5f, 1.0f}, true);
        } else {
            state = get_panel_entry(definition->field_18, true);
            state->panel->add_gutter_component(true, true, true);
            auto *texture = nglLoadTexture(tlFixedString{"grad_burst03"});
            if (!texture || texture == nglDefaultTex)
                state->panel->add_color_component({0.65f, 0.55f, 0.0f, 1.0f}, true);
            else
                state->panel->add_texture_component(texture);
        }
        camera->elevation = definition->camera_elevation;
        camera->azimuth = definition->camera_azimuth;
        camera->distance = definition->camera_distance;
        camera->fov = definition->camera_fov;
        const char *names[]{"GLAMCAM0", "GLAMCAM1", "GLAMCAM2", "GLAMCAM3"};
        state->panel->add_camera_component(names[definition->camera_index], true, 255);
        state->panel->reset_base_opacity();
        state->panel->field_50 = 1.0f;
        state->animation = animation;
        state->panel->set_loc(animation->get_loc());
        state->panel->set_size(animation->get_size());
        auto *entity = static_cast<struct entity *>(
            g_world_ptr->ent_mgr.get_entity(resolve_entity_name(definition->field_0.c_str())));
        if (!entity)
            entity = create_entity(
                create_resource_key_from_path(definition->field_0.c_str(), static_cast<resource_key_type>(0)));
        if (!entity)
            continue;
        if (entity != hero)
            tracked_entities[4].push_back(entity->my_handle);
        camera->target = entity->my_handle;
        camera->target_bone = definition->field_10.m_hash;
        entity->get_primary_region();
        const auto position = entity->get_abs_position();
        if (auto *core = entity->get_ai_core()) {
            auto *target =
                static_cast<ai::combat_target_inode *>(core->get_info_node(ai::combat_target_inode::default_id, true));
            if (target) {
                vhandle_type<actor> scripted{target->field_30};
                if (scripted.get_volatile_ptr())
                    tracked_entities[2].push_back(scripted.field_0);
                else {
                    const auto quick = target->quick_targeting();
                    if (quick.get_volatile_ptr() && target->is_target_known())
                        tracked_entities[2].push_back(quick.field_0);
                }
            }
        }
        for (auto *spawner : ped_spawner::ped_spawner_list) {
            if (!spawner)
                continue;
            auto *actor = spawner->get_my_actor();
            if (actor && (actor->get_abs_position() - position).length2() < 100.0f)
                tracked_entities[3].push_back(actor->my_handle);
        }
    }
}

void cut_scene_player::activate_current_segment()
{
    resource_manager::push_resource_context(current_cut_scene->field_50);
    if (!current_cut_scene->gameplay_panel_hidden)
        comic_panels::game_play_panel()->field_67 = true;
    for (auto &group : tracked_entities)
        group.clear();
    setup_tracking_panels();
    auto *hero = g_world_ptr->get_hero_ptr(0);
    if (hero)
        tracked_entities[0].push_back(hero->my_handle);
    auto *segment = *current_segment;
    auto add = [&](struct entity *entity, unsigned group) {
        if (!entity)
            return;
        entity->compute_sector(g_world_ptr->the_terrain, false, nullptr);
        if (entity != hero)
            tracked_entities[group].push_back(entity->my_handle);
    };
    for (unsigned group = 1; group < 5; ++group) {
        for (auto *name : *segment->field_48.at(group - 1))
            add(static_cast<struct entity *>(g_world_ptr->ent_mgr.get_entity(resolve_entity_name(name->c_str()))),
                group);
        if (group == 1 && tracked_entities[group].empty()) {
            for (const auto handle : animated_entities)
                add(static_cast<struct entity *>(handle.get_volatile_ptr()), group);
            for (auto *entity : acquired_entities)
                add(entity, group);
        }
    }
    visit_panel_states(panels.root, [this](cut_scene_panel_state &state) { state.panel->field_4C = this; });
    for (auto *setup : segment->field_5C) {
        auto *entity = g_world_ptr->ent_mgr.get_entity(string_hash{setup->field_0.c_str()});
        if (!entity || !entity->is_a_camera())
            continue;
        auto *target = g_world_ptr->ent_mgr.get_entity(resolve_entity_name(setup->field_10.c_str()));
        if (!target)
            target = hero;
        auto is_glam = reinterpret_cast<bool(__fastcall *)(entity_base *, void *)>(get_vfunc(entity->m_vtbl, 0x78));
        if (is_glam(entity, nullptr)) {
            auto *camera = static_cast<glam_camera *>(entity);
            std::memcpy(&camera->elevation, setup->field_24, 16);
            camera->target = target->my_handle;
        } else {
            entity->set_parent(target);
            using target_bone = void(__fastcall *)(entity_base *, void *, string_hash);
            string_hash bone;
            std::memcpy(&bone, setup->field_24 + 16, sizeof(bone));
            reinterpret_cast<target_bone>(get_vfunc(entity->m_vtbl, 0x2AC))(entity, nullptr, bone);
            line_info line{entity->get_abs_position(), target->get_abs_position()};
            line.check_collision(*local_collision::entfilter_line_segment_camera_collision,
                                 *local_collision::obbfilter_lineseg_test,
                                 nullptr);
            if (setup->field_20 != string_hash{})
                entity->is_an_actor();
        }
    }
    if (segment->field_98.size() > 0)
        current_camera =
            static_cast<entity *>(g_world_ptr->ent_mgr.get_entity(resolve_entity_name(segment->field_98.c_str())));
    if (hero && !segment->field_6 &&
        std::find(animated_entities.begin(), animated_entities.end(), hero->my_handle) == animated_entities.end()) {
        hero->unsuspend(true);
        hero->field_4 &= ~0x4000u;
        hero->physical_ifc()->suspend(false);
        hero->physical_ifc()->enable(true);
        glass_house_manager::enabled = saved_glass_house_enabled();
    }
    resource_manager::pop_resource_context();
}

entity *cut_scene_player::create_entity(resource_key key)
{
    auto *entry = get_entity_entry(key.m_hash);
    if (!entry || entry->field_4 == string_hash{})
        return nullptr;
    script_memtrack::begin_entity_creation(mString{entry->field_4.to_string()});
    auto *entity = g_world_ptr->ent_mgr.acquire_entity(entry->field_4, string_hash{entry->field_8.c_str()}, 129);
    script_memtrack::end_entity_creation(entity->my_handle);
    acquired_entities.push_back(entity);
    if (!(entry->field_40 & 1))
        return entity;
    if (!(entry->field_40 & 2)) {
        vehicle::pick_body_and_color(static_cast<actor *>(entity));
        return entity;
    }
    auto *conglom = static_cast<conglomerate *>(entity);
    conglom->get_member(string_hash{"POLICE_GEAR"}, 1)->field_4 &= ~0x80000000u;
    static_cast<actor *>(entity)->ifl_lock(1);
    auto visible = [conglom](const char *name, bool enabled) {
        auto *member = static_cast<struct entity *>(conglom->get_member(string_hash{name}, 1));
        if (enabled)
            member->field_4 &= ~0x80000000u;
        else
            member->field_4 |= 0x80000000u;
        member->set_visible(enabled, false);
        member->set_collisions_active(enabled, true);
        if (enabled)
            member->set_render_color(color32{0xFFFFFFFF});
    };
    for (const char *name :
         {"N1", "N1_D", "N1_P", "N1_D_WINDOW_NOTINT", "N1_P_WINDOW_NOTINT", "N1_XTRAS_NOTINT", "T2", "T2_XTRAS_NOTINT"})
        visible(name, true);
    for (const char *name : {"N2",
                             "N2_D",
                             "N2_P",
                             "N2_D_WINDOW_NOTINT",
                             "N2_P_WINDOW_NOTINT",
                             "N2_XTRAS_NOTINT",
                             "N5",
                             "N5_D",
                             "N5_P",
                             "T1",
                             "T1_XTRAS_NOTINT",
                             "T3",
                             "T3_XTRAS_NOTINT"})
        visible(name, false);
    static_cast<actor *>(conglom->get_member(string_hash{"N1_XTRAS_NOTINT"}, 1))->ifl_lock(0);
    static_cast<actor *>(conglom->get_member(string_hash{"T2_XTRAS_NOTINT"}, 1))->ifl_lock(0);
    return entity;
}

entity_class_entry *cut_scene_player::get_entity_entry(string_hash id)
{
    auto find = [id](mAvlTree<entity_class_entry> &tree) -> entity_class_entry * {
        auto *node = tree.root();
        while (node) {
            const auto key = node->m_key->field_0.source_hash_code;
            if (key == id.source_hash_code)
                return node->m_key;
            node = id.source_hash_code < key ? node->m_left : node->m_right;
        }
        return nullptr;
    };
    if (!current_cut_scene)
        return nullptr;
    if (*current_segment) {
        if (auto *entry = find((*current_segment)->field_84))
            return entry;
    }
    return find(current_cut_scene->field_0);
}

string_hash cut_scene_player::resolve_entity_name(const char *name)
{
    const auto *entry = get_entity_entry(string_hash{name});
    if (entry)
        name = entry->field_8.c_str();
    if (std::strncmp(name, "LIST_", 5) == 0) {
        const unsigned index = std::atoi(name + 5);
        if (index < field_BC.size()) {
            if (auto *entity = field_BC[index].get_volatile_ptr())
                return entity->field_10;
        }
    }
    return string_hash{name};
}

unsigned char *cut_scene_player::get_heap_slot_datum()
{
    auto *segment = *current_segment;
    if (!segment->field_A8)
        return nullptr;
    auto &slots = resource_manager::get_partition_pointer(RESOURCE_PARTITION_DISTRICT)->get_pack_slots();
    return slots[segment->field_AC]->get_header_mem_addr();
}

int cut_scene_player::get_heap_slot_size()
{
    auto *segment = *current_segment;
    if (!segment->field_A8)
        return 0;
    auto &slots = resource_manager::get_partition_pointer(RESOURCE_PARTITION_DISTRICT)->get_pack_slots();
    return slots[segment->field_AC]->get_slot_size();
}

bool cut_scene_player::wait_for_regions(float dt)
{
    for (auto *name : (*current_segment)->field_34) {
        const auto *region = g_world_ptr->the_terrain->find_region(string_hash{name->c_str()});
        if ((region->flags & 0x10) == 0) {
            field_15C += dt;
            return false;
        }
    }
    return true;
}

bool cut_scene_player::wait_for_heap_slot(float dt)
{
    auto *segment = *current_segment;
    if (!segment->field_A8)
        return true;
    auto *terrain = g_world_ptr->the_terrain;
    if (segment->field_AC == -1) {
        int districts[8];
        int count = 0;
        for (auto *name : segment->field_34)
            districts[count++] = terrain->find_region(string_hash{name->c_str()})->district_id;
        segment->field_AC = terrain->lock_district_pack_slot(districts, count);
    }
    field_160 += dt;
    if (segment->field_AC == -1 || !terrain->is_district_pack_slot_locked(segment->field_AC))
        return false;
    return resource_manager::get_partition_pointer(RESOURCE_PARTITION_DISTRICT)
        ->get_pack_slots()[segment->field_AC]
        ->is_empty();
}

void cut_scene_player::start_lip_syncing()
{
    visit_entity_entries((*current_segment)->field_84, [](entity_class_entry &entry) {
        auto *actor = lip_sync_actor(entry);
        if (!actor)
            return;
        actor->field_90.start_buffering(2);
        if (auto *lip = get_lip_sync(actor)) {
            lip->set_morph_name(entry.field_44);
            for (auto *viseme : entry.field_74)
                lip->queue(viseme->field_0, viseme->field_4 * (1.0f / 30.0f));
        }
    });
}

void cut_scene_player::stop_lip_syncing()
{
    visit_entity_entries((*current_segment)->field_84, [](entity_class_entry &entry) {
        auto *actor = lip_sync_actor(entry);
        if (!actor)
            return;
        actor->field_90.end_buffering();
        if (auto *lip = get_lip_sync(actor))
            lip->stop_all();
    });
}

nalClientSceneAnim *cut_scene_player::scene_anim_callback(const tlFixedString &name, void *context)
{
    return static_cast<cut_scene_player *>(context)->find_scene_anim_entries(name);
}

void cut_scene_player::play(cut_scene *scene)
{
    if (!scene || !scene->field_4C || scene->segments.empty())
        return;
    stop(nullptr);
    saved_glass_house_enabled() = glass_house_manager::enabled;
    current_cut_scene = scene;
    field_DC = scene->recording ? 1 : scene->field_4C;
    if (auto *panel = comic_panels::game_play_panel())
        panel->field_65 = !scene->field_25;
    current_segment = scene->segments.begin();
    if (scene->physics_overridden) {
        saved_physics_enabled() = !g_game_ptr->flag.physics_enabled;
        g_game_ptr->enable_physics(false);
    }
    if (scene->ui_hidden) {
        minimap_was_shown = g_femanager.IGO->m_fe_mini_map_widget->field_3A8;
        hero_health_was_shown = g_femanager.IGO->m_hero_health->field_54;
        boss_health_was_shown = g_femanager.IGO->m_boss_health->field_54;
        if (minimap_was_shown)
            g_femanager.IGO->m_fe_mini_map_widget->SetShown(false);
        if (hero_health_was_shown)
            g_femanager.IGO->m_hero_health->SetShown(false);
        if (boss_health_was_shown)
            g_femanager.IGO->m_boss_health->SetShown(false);
        g_femanager.IGO->m_igo_zoom_out_map->field_5C6 = true;
        g_game_ptr->zoomInactive = true;
    }
    if (scene->tokens_hidden) {
        event_manager::raise_event(event::HIDE_FINGERS_OF_GOD, entity_base_vhandle{0});
        g_world_ptr->field_188.mark_invisible_by_id(false);
    }
    owned_camera = nullptr;
    if (scene->field_3C.size() > 0) {
        auto *ent = g_world_ptr->ent_mgr.get_entity(resolve_entity_name(scene->field_3C.c_str()));
        if (ent) {
            owned_camera = g_world_ptr->ent_mgr.acquire_entity(string_hash{"nullo"}, 0x81);
            if (owned_camera) {
                owned_camera->set_visible(false, false);
                owned_camera->set_abs_po(ent->get_abs_po());
            }
        }
    }
    m_peds_enabled = os_developer_options::instance->get_flag(static_cast<os_developer_options::flags_t>(136));
    m_traffic_enabled = traffic::traffic_enabled;
    peds_and_traffic_overridden = true;
    play_current_segment();
    field_E1 = true;
}

void cut_scene_player::play_current_segment()
{
    resource_manager::push_resource_context(current_cut_scene->field_50);
    fade_legos() = false;
    field_E0 = false;
    auto *segment = *current_segment;
    if (segment->field_0 || segment->field_6) {
        cut_scene_camera_active() = true;
        glass_house_manager::enabled = false;
    } else {
        cut_scene_camera_transition() = true;
    }
    if (!segment->field_2)
        clear_panels();
    for (auto *name : segment->field_34) {
        auto *reg = g_world_ptr->the_terrain->find_region(string_hash{name->c_str()});
        if (reg && (reg->flags & 1)) {
            hidden_regions.push_back(reg);
            reg->flags &= ~1u;
        }
        g_world_ptr->the_terrain->hide_region(reg);
    }
    if (current_cut_scene->stream_anims) {
        cut_scene::init_stream_scene_anims();
        for (auto *key : segment->field_20) {
            resource_location location;
            cut_scene::stream_anim_pack.get_unloaded_resource_location(*key, &location);
            auto *instance = create_stream_instance(cut_scene::stream_anim_pack.get_nfl_file_handle().field_0,
                                                    location.m_offset,
                                                    location.m_size,
                                                    0,
                                                    scene_anim_callback,
                                                    this);
            streams.push_back(instance);
            while (!instance->IsReady()) {
                nflUpdate();
                instance->Advance(Float{0.0f});
            }
            instance->Advance(Float{0.0f});
            instance->Render();
        }
    } else {
        for (int i = 0; i < segment->field_10.size(); ++i) {
            auto *instance = segment->field_10.at(i)->CreateInstance(scene_anim_callback, this);
            streams.push_back(instance);
            instance->Advance(Float{0.0f});
            instance->Render();
        }
    }
    g_world_ptr->the_terrain->force_streamer_refresh();
    if (segment->field_6) {
        if (auto *hero = static_cast<actor *>(g_world_ptr->get_hero_ptr(0))) {
            hero->get_player_controller()->lock_controls(false);
            hero->suspend(true);
            hero->physical_ifc()->suspend(true);
            hero->physical_ifc()->enable(false);
        }
    }
    if (!segment->field_4)
        os_developer_options::instance->set_flag(static_cast<os_developer_options::flags_t>(136), false);
    if (!segment->field_5)
        traffic::enable_traffic(false, false);
    if (segment->field_94 != string_hash{}) {
        auto source = sound_manager::get_sound_source(segment->field_94);
        if (source.get_channel_count() <= 1) {
            sound_inst = sound_manager::create_sound_instance(15, segment->field_94);
        } else if (source.get_sample_rate() <= 24000) {
            ambient_audio_manager::set_max_playing_tracks(1);
            sound_inst = sound_manager::create_lofi_stereo_sound_instance(15, segment->field_94);
        } else {
            sound_inst = sound_manager::create_hifi_stereo_sound_instance(15, segment->field_94, false);
        }
        if (auto *sound = sound_inst.get_sound_instance_ptr())
            sound->queue();
    }
    field_15C = 0.0f;
    field_160 = 0.0f;
    resource_manager::pop_resource_context();
}

void cut_scene_player::frame_advance(Float dt)
{
    if (!frame_advance_lite(dt))
        return;
    float time = std::min(current_cut_scene && current_cut_scene->recording ? float(dt) : cut_scene_time_inc(), 0.1f);
    field_E4.update(Float{time});
    field_118.update(Float{time});
    const bool triggered = ((!field_118.is_flagged(0x20) && field_118.is_flagged(2)) ||
                            (!field_E4.is_flagged(0x20) && field_E4.is_flagged(2))) &&
                           field_158 <= 0.0f;
    const bool skip = (triggered || field_154 > 0.0f) && current_cut_scene->field_24;
    field_E2 = skip;
    if (skip && triggered)
        field_154 = 3.0f;
    float rate = 1.0f;
    if (os_developer_options::instance->get_flag(static_cast<os_developer_options::flags_t>(132))) {
        rate = input_mgr::instance->get_control_state(static_cast<game_control_t>(16), static_cast<device_id_t>(-1)) +
               1.0f;
        if (rate > 1.0f)
            rate = (rate - 1.0f) * 5.0f + 1.0f;
    }
    if (current_cut_scene) {
        time *= rate * current_cut_scene->field_2C;
        for (auto it = streams.begin(); it != streams.end();) {
            auto *instance = *it;
            if (skip)
                time = *reinterpret_cast<float *>(&instance->field_4->field_3C);
            if (instance->IsFinished()) {
                instance->Destroy();
                it = streams.erase(it);
            } else {
                instance->Advance(Float{time});
                if (should_render_scene_anims())
                    instance->Render();
                ++it;
            }
        }
        advance_lip_syncing(Float{time});
        if ((advance_panel_anims(Float{time}) && streams.empty()) || skip) {
            clean_up_finished_segment();
            ++current_segment;
            if (current_segment == current_cut_scene->segments.end() || skip) {
                if (field_DC-- == 1) {
                    stop(nullptr);
                } else {
                    current_segment = current_cut_scene->segments.begin();
                    play_current_segment();
                }
            } else {
                play_current_segment();
            }
            if (skip)
                mission_manager::s_inst->sub_5BACA0(Float{0.0f});
        }
    }
}

bool cut_scene_player::frame_advance_lite(Float dt)
{
    const float time =
        std::min(current_cut_scene && current_cut_scene->recording ? float(dt) : cut_scene_time_inc(), 0.1f);
    field_E2 = false;
    if (field_154 > 0.0f)
        field_154 -= time;
    if (!field_E1) {
        if (field_158 < 1.0f)
            field_158 += time;
        return false;
    }
    if (field_158 >= 0.0f)
        field_158 -= time;
    if (current_camera) {
        const auto &pose = current_camera->get_abs_po();
        sound_manager::set_listener_position(pose.get_position());
        sound_manager::set_listener_orientation(vector3d{pose.m[2][0], pose.m[2][1], pose.m[2][2]},
                                                vector3d{pose.m[1][0], pose.m[1][1], pose.m[1][2]});
        sound_manager::set_listener_velocity(current_camera->get_velocity());
    }
    if (field_E0) {
        field_164 += time;
        fade_legos() = true;
        return true;
    }
    if (!wait_for_regions(time) || !wait_for_heap_slot(time))
        return false;
    if (auto *sound = sound_inst.get_sound_instance_ptr(); sound && sound->state == 1)
        return false;
    var<void *>(0x0096F7C8) = get_heap_slot_datum();
    var<int>(0x0096F7CC) = get_heap_slot_size();
    mission_manager::s_inst->blackscreen_off(Float{0.0f});
    auto hide_panel = [](auto &&visit, cut_scene_panel_node *node) -> void {
        if (!node)
            return;
        visit(visit, node->left);
        node->state->panel->field_67 = false;
        visit(visit, node->right);
    };
    hide_panel(hide_panel, panels.root);
    field_E0 = true;
    cut_scene_camera_active() = false;
    cut_scene_camera_transition() = false;
    field_164 = 0.0f;
    activate_current_segment();
    start_lip_syncing();
    if ((*current_segment)->field_8 >= 0)
        us_lighting_switch_time_of_day((*current_segment)->field_8);
    if (current_cut_scene->recording) {
        app::instance->field_4.begin_screen_recording(mString{"cut_scene_capture"}, 30);
        current_cut_scene->field_4C = 1;
        frame_time_limit() = 10000000.0f;
    }
    if (auto *sound = sound_inst.get_sound_instance_ptr())
        sound->play();
    return true;
}

bool cut_scene_player::is_playing()
{
    return this->field_E1 || this->field_E2;
}

cut_scene_player *g_cut_scene_player()
{
#if STANDALONE_SYSTEM
    static cut_scene_player player;
    return &player;
#else
    static Var<cut_scene_player> player{0x0096FEB8};
    return &player();
#endif
}

void cut_scene_player::stop(cut_scene *a2)
{
    fade_legos() = true;
    cut_scene_camera_active() = false;
    glass_house_manager::enabled = saved_glass_house_enabled();
    cut_scene_camera_transition() = false;
    if (a2 != nullptr && a2 != current_cut_scene)
        return;

    if (field_E1) {
        if (*current_segment && current_segment != current_cut_scene->segments.end())
            clean_up_finished_segment();
        auto *node = current_cut_scene->field_0.root();
        while (node && node->m_left)
            node = node->m_left;
        for (mAvlTree<entity_class_entry>::iterator it{node}; it.field_0; ++it) {
            auto *entry = (*it).m_key;
            auto *ent = g_world_ptr->ent_mgr.get_entity(entry->field_0);
            if (ent && entry->field_25)
                entity_set_abs_po(ent, restored_pose(*entry));
        }
        if (current_cut_scene->sync_camera.m_hash != string_hash{}) {
            auto *ent = g_world_ptr->ent_mgr.get_entity(current_cut_scene->sync_camera.m_hash);
            if (ent && ent->is_a_camera())
                g_spiderman_camera_ptr()->sync(*static_cast<camera *>(ent));
        }
        if (peds_and_traffic_overridden) {
            os_developer_options::instance->set_flag(static_cast<os_developer_options::flags_t>(136), m_peds_enabled);
            traffic::enable_traffic(m_traffic_enabled, false);
            peds_and_traffic_overridden = false;
        }
        if (current_cut_scene->recording)
            app::instance->field_4.end_screen_recording();
        if (auto *sound = sound_inst.get_sound_instance_ptr()) {
            sound->stop();
            ambient_audio_manager::set_max_playing_tracks(2);
        }
    }
    restore_game_play_panel();
    field_E1 = false;
    if (current_cut_scene) {
        if (current_cut_scene->physics_overridden) {
            g_game_ptr->enable_physics(saved_physics_enabled());
            saved_physics_enabled() = true;
        }
        if (current_cut_scene->ui_hidden) {
            if (hero_health_was_shown)
                g_femanager.IGO->m_hero_health->SetShown(hero_health_was_shown);
            if (minimap_was_shown)
                g_femanager.IGO->m_fe_mini_map_widget->SetShown(minimap_was_shown);
            if (boss_health_was_shown)
                g_femanager.IGO->m_boss_health->SetShown(boss_health_was_shown);
            g_femanager.IGO->m_igo_zoom_out_map->field_5C6 = false;
            g_game_ptr->zoomInactive = false;
        }
        if (current_cut_scene->tokens_hidden) {
            event_manager::raise_event(event::SHOW_FINGERS_OF_GOD, entity_base_vhandle{0});
            g_world_ptr->field_188.mark_invisible_by_id(true);
        }
    }
    if (owned_camera) {
        g_world_ptr->ent_mgr.release_entity(owned_camera);
        owned_camera = nullptr;
    }
    current_camera = nullptr;
    current_cut_scene = nullptr;
    frame_time_limit() = std::min(frame_time_limit(), 1.0f / 15.0f);
}

void cut_scene_player_patch()
{
    {
        FUNC_ADDRESS(address, &cut_scene_player::play_current_segment);
        REDIRECT(0x00742419, address);
    }

    {
        FUNC_ADDRESS(address, &cut_scene_player::frame_advance);
        REDIRECT(0x0054FC7A, address);
        REDIRECT(0x0054FC8D, address);
    }

    //cut_scene_player::play_current_segment
    {
        {
            FUNC_ADDRESS(address, &resource_pack_standalone::get_nfl_file_handle);
            REDIRECT(0x007416A8, address);
        }

        REDIRECT(0x007416AE, create_stream_instance);
    }
}
