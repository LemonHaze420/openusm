#include "tentacle_interface.h"

#include "als_inode.h"
#include "base_ai_core.h"
#include "animation_controller.h"
#include "fixedstring.h"
#include "ngl.h"
#include "nal_anim_controller.h"
#include "polytube.h"
#include "polytubecustommaterial.h"
#include "slab_allocator.h"
#include "state_machine.h"
#include "variable.h"
#include <new>
#include "common.h"
#include "conglom.h"
#include "entity.h"
#include "event.h"
#include "event_manager.h"
#include "event_recipient_entry.h"
#include "wds.h"
#include <functional>

VALIDATE_SIZE(tentacle_interface, 0x38);

void tentacle_interface::initialize_polytubes()
{
    for (int index = 0; index < field_1C.m_size; ++index) {
        auto &info = field_1C[index];
        void *storage = sizeof(polytube) > slab_allocator::get_max_object_size()
                            ? ::operator new(sizeof(polytube))
                            : slab_allocator::allocate(sizeof(polytube), nullptr);
        auto *tube = new (storage) polytube(make_unique_entity_id(), 0);
        const tlFixedString sphere_name{"us_char_sphrmap_ink_6"};
        const tlFixedString texture_name{info.texture_name};
        auto *sphere = nglLoadTexture(sphere_name);
        auto *texture = nglLoadTexture(texture_name);
        Tentacle_ShaderMaterial material(texture, sphere, 1);
        tube->set_material(&material);
        tube->field_E8 = info.tentacle_id;
        tube->field_EC = info.field_8;
        if (std::not_equal_to<float>{}(tube->tube_radius, info.radius)) {
            tube->tube_radius = info.radius;
            tube->field_78 = false;
        }
        tube->set_render_color(info.color);
        tube->tiles_per_meter = 5.0f;
        if (tube->num_sides != 5) {
            tube->num_sides = 5;
            tube->field_78 = false;
        }
        tube->field_7B = false;
        tube->field_7C = nullptr;
        tube->set_visible(true, false);
        tube->the_spline.reserve_control_pts(info.field_3C.m_size);
        for (int point = 0; point < info.field_3C.m_size; ++point) {
            auto *source = my_conglomerate->get_member(info.field_3C[point].id, true);
            info.field_44[point] = source;
            tube->add_control_pt(tube->get_abs_po().inverse_xform(source->get_abs_position()));
        }
        tube->build(info.field_3C.m_size, static_cast<spline::eSplineType>(2));
        g_world_ptr->ent_mgr.add_dynamic_instanced_entity(tube);
        field_24[index] = tube;
        if (info.zip_entity_id.source_hash_code != 0)
            info.zip_entity = my_conglomerate->get_bone(info.zip_entity_id, true);
    }
}

void zip_event_callback(event *the_event, entity_base_vhandle a2, void *params)
{
    assert(the_event != nullptr && params != nullptr);

    if (a2.get_volatile_ptr() != nullptr) {
        static_cast<tentacle_interface *>(params)->tentacle_zip_event_fired();
    }
}

void tentacle_interface::begin_zip(const vector3d &a2)
{
    this->field_10 = a2;
    auto v3 = this->field_34;
    this->field_28 = 0;
    if (v3 == 0) {
        auto v4 = this->my_conglomerate->my_handle;
        this->field_34 = event_manager::add_callback(event::ANIM_ACTION, v4, zip_event_callback, this, false);
    }
}

void tentacle_interface::tentacle_zip_event_fired()
{
    if (field_28 != 0) {
        field_28 = 2;
        if (auto *core = my_conglomerate->get_ai_core()) {
            auto *node = static_cast<ai::als_inode *>(core->get_info_node(ai::als_inode::default_id, false));
            if (node != nullptr) {
                const auto &handle = node->get_als_layer(static_cast<als::layer_types>(0))->get_anim_handle();
                field_2C = handle.get_anim_duration() - handle.get_anim_time_in_sec();
                field_30 = field_2C;
            }
        }
    } else {
        field_28 = 1;
        field_2C = field_30 = 0.0f;
    }
}

void tentacle_interface::cancel_zip()
{
    auto v2 = this->field_34;
    if (v2 != 0) {
        event_manager::remove_callback(v2, event::ANIM_ACTION, this->my_conglomerate->get_my_vhandle());
    }

    this->field_34 = 0;
    this->field_28 = 3;
}

void tentacle_interface::release_ifc()
{
    for (int i = 0; i < this->field_1C.size(); ++i) {
        if (this->field_24[i] != nullptr) {
            g_world_ptr->ent_mgr.destroy_entity(this->field_24[i]);
            this->field_24[i] = nullptr;
        }
    }

    this->cancel_zip();
}

namespace {
Var<int> last_tentacle_tick{0x0095BFCC};
const bool initialize_tentacle_tick = [] {
    last_tentacle_tick() = -1;
    return true;
}();

void apply_tentacle_animation(conglomerate *owner, tentacle_info &info, polytube *tube)
{
    if (owner->anim_ctrl != nullptr) {
        const auto key = info.field_3C[0].id;
        tube->set_tentacle_width(Float{owner->anim_ctrl->get_tentacle_width(key)});
        tube->set_tentacle_pull_factor(Float{owner->anim_ctrl->get_tentacle_pull_factor(key)});
    }
}
}  // namespace

void tentacle_interface::standard_tentacle_update(int index, tentacle_info &info)
{
    auto *tube = static_cast<polytube *>(field_24[index]);
    for (int point = 0; point < info.field_3C.m_size; ++point) {
        const auto position = tube->get_abs_po().inverse_xform(info.field_44[point]->get_abs_position());
        if (point < tube->get_num_control_pts())
            tube->set_control_pt(point, position);
    }
    apply_tentacle_animation(my_conglomerate, info, tube);
}

void tentacle_interface::update_tentacle_zip_aiming(Float elapsed, int index)
{
    auto &info = field_1C[index];
    if (last_tentacle_tick() != g_world_ptr->time_manager.field_C) {
        last_tentacle_tick() = g_world_ptr->time_manager.field_C;
        field_2C -= elapsed.value;
    }
    if (field_2C < 0.0f)
        field_2C = 0.0f;
    if (field_28 == 1 && std::equal_to<float>{}(field_30, 0.0f)) {
        if (auto *core = my_conglomerate->get_ai_core()) {
            auto *node = static_cast<ai::als_inode *>(core->get_info_node(ai::als_inode::default_id, false));
            if (node != nullptr) {
                field_30 = node->get_eta_of_combat_signal(static_cast<als::layer_types>(0));
                if (field_30 <= elapsed.value)
                    field_30 = 0.0f;
                field_2C = field_30;
            }
        }
    }
    float blend;
    if (field_28 == 1 && field_30 > 0.0f) {
        blend = 1.0f - field_2C / field_30;
    } else if (field_28 == 2) {
        blend = field_2C / field_30;
        if (field_2C <= 0.0f)
            cancel_zip();
    } else {
        standard_tentacle_update(index, info);
        return;
    }
    if (blend <= 0.0f) {
        standard_tentacle_update(index, info);
        return;
    }
    const auto origin = info.field_44[0]->get_abs_position();
    const float original_length = (info.field_44[info.field_3C.m_size - 1]->get_abs_position() - origin).length();
    auto direction = field_10 - origin;
    const float target_length = direction.length();
    if (original_length > 0.0f && target_length >= 0.0f) {
        direction = direction / target_length;
        const float inverse_length = 1.0f / original_length;
        auto *tube = static_cast<polytube *>(field_24[index]);
        for (int point = 0; point < info.field_3C.m_size; ++point) {
            auto current = info.field_44[point]->get_abs_position();
            const auto offset = current - origin;
            const float fraction = offset.length() * inverse_length;
            if (original_length > target_length)
                current = origin + offset * (inverse_length * target_length);
            const auto aimed = origin + direction * (target_length * fraction);
            const float amount = fraction * blend * blend;
            tube->set_abs_control_pt(point, current * (1.0f - amount) + aimed * amount);
        }
        apply_tentacle_animation(my_conglomerate, info, tube);
    }
}

void tentacle_interface::render(Float fade)
{
    for (int index = 0; index < field_1C.m_size; ++index) {
        auto &info = field_1C[index];
        if (field_28 == 3 || info.zip_entity == nullptr)
            standard_tentacle_update(index, info);
        else
            update_tentacle_zip_aiming(Float{g_world_ptr->time_manager.field_18}, index);
        auto *tube = static_cast<polytube *>(field_24[index]);
        tube->rebuild_helper();
        tube->field_4 |= 0x200;
        tube->_render(fade);
        tube->field_4 &= ~0x200u;
    }
}
