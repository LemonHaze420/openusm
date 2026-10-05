#include "base_full_target_inode.h"

#include "common.h"
#include "func_wrapper.h"
#include "base_ai_core.h"
#include "controller_inode.h"
#include "wds.h"
#include "vtbl.h"
#include "oldmath_po.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <new>

namespace ai {
VALIDATE_SIZE(base_full_target_inode, 0x84);
VALIDATE_SIZE(targetable_inode_285, 0x24);

namespace {
void *__fastcall target_delete(base_full_target_inode *self, void *, unsigned flags)
{
    self->~base_full_target_inode();
    if (flags & 1)
        mash_virtual_base::operator delete(self, sizeof(base_full_target_inode));
    return self;
}
unsigned __fastcall target_type(base_full_target_inode *, void *)
{
    return 349;
}
bool __fastcall target_subclass(base_full_target_inode *, void *, unsigned type)
{
    return type == 350 || type == 537 || type == 573;
}
bool __fastcall target_needs_advance(base_full_target_inode *, void *)
{
    return true;
}
void __fastcall target_advance(base_full_target_inode *self, void *, Float delta)
{
    self->_frame_advance(delta);
}
void __fastcall target_activate(base_full_target_inode *self, void *, ai_core *core)
{
    self->_activate(core);
}
void __fastcall target_deactivate(base_full_target_inode *self, void *)
{
    self->_deactivate();
}
int __fastcall target_size(base_full_target_inode *, void *)
{
    return sizeof(base_full_target_inode);
}
void __fastcall target_unregister(base_full_target_inode *self, void *)
{
    self->unregister_as_targetable();
}
void __fastcall target_register(base_full_target_inode *self, void *)
{
    self->register_as_targetable();
}
vhandle_type<actor> *__fastcall target_get(base_full_target_inode *self, void *, vhandle_type<actor> *out)
{
    *out = self->field_34;
    return out;
}
vector3d *__fastcall target_position(base_full_target_inode *self, void *, vector3d *out)
{
    *out = self->last_known_position;
    return out;
}
int __fastcall target_rank(base_full_target_inode *self, void *)
{
    return self->field_38;
}
bool __fastcall target_plausible(base_full_target_inode *self, void *, vhandle_type<actor> candidate)
{
    return candidate.field_0 != self->field_C->my_handle && candidate.get_volatile_ptr() != nullptr;
}
vector3d *__fastcall target_direction(base_full_target_inode *self, void *, vector3d *out)
{
    *out = self->get_controller_look_direction();
    return out;
}
void __fastcall target_update(base_full_target_inode *self, void *)
{
    self->update_targeting();
}
void __fastcall target_calculate(base_full_target_inode *self, void *, vhandle_type<actor> candidate)
{
    self->calc_and_update_target(candidate);
}
vhandle_type<actor> *__fastcall target_find(base_full_target_inode *self, void *, vhandle_type<actor> *out)
{
    *out = self->find_target();
    return out;
}
void __fastcall target_player_advance(base_full_target_inode *self, void *, Float delta)
{
    self->player_style_frame_advance(delta);
}
void __fastcall target_cache(base_full_target_inode *self, void *, bool force)
{
    self->update_cached_params(force);
}
void __fastcall target_set(base_full_target_inode *self, void *, vhandle_type<actor> candidate)
{
    self->field_34 = candidate;
    using calculate_fn = void(__fastcall *)(base_full_target_inode *, void *, vhandle_type<actor>);
    reinterpret_cast<calculate_fn>(get_vfunc(self->m_vtbl, 0x50))(self, nullptr, candidate);
}
bool __fastcall target_visible(base_full_target_inode *self, void *)
{
    return self->field_38 <= 3;
}
bool __fastcall target_viable(base_full_target_inode *self, void *)
{
    return self->field_38 <= 4;
}
bool __fastcall target_known(base_full_target_inode *self, void *)
{
    return self->field_38 <= 5;
}
bool __fastcall target_lost(base_full_target_inode *self, void *)
{
    return self->field_38 == 6;
}
void __fastcall target_clear(base_full_target_inode *self, void *)
{
    self->clear_target();
}
bool __fastcall target_perfect(base_full_target_inode *self, void *)
{
    return self->perfect_perception;
}
float __fastcall target_angle(base_full_target_inode *self, void *)
{
    return self->vision_angle_cos;
}
float __fastcall target_range(base_full_target_inode *self, void *)
{
    return self->vision_range;
}
float __fastcall target_viable_radius(base_full_target_inode *self, void *)
{
    return self->viable_radius;
}
float __fastcall target_aware_radius(base_full_target_inode *self, void *)
{
    return self->aware_radius;
}
bool __fastcall target_search_active(base_full_target_inode *self, void *)
{
    return self->field_54;
}
void __fastcall target_search_end(base_full_target_inode *self, void *)
{
    self->end_multi_frame_search();
}
void __fastcall target_search_start(base_full_target_inode *self, void *, int budget)
{
    self->start_multi_frame_search(budget);
}
}  // namespace

void *base_full_target_inode::native_vtable()
{
    auto **base = static_cast<void **>(info_node::native_vtable());
    static void *table[] = {
        base[0],
        base[1],
        reinterpret_cast<void *>(&target_delete),
        reinterpret_cast<void *>(&target_type),
        reinterpret_cast<void *>(&target_subclass),
        base[5],
        reinterpret_cast<void *>(&target_needs_advance),
        reinterpret_cast<void *>(&target_advance),
        reinterpret_cast<void *>(&target_activate),
        reinterpret_cast<void *>(&target_deactivate),
        base[10],
        reinterpret_cast<void *>(&target_size),
        reinterpret_cast<void *>(&target_unregister),
        reinterpret_cast<void *>(&target_register),
        reinterpret_cast<void *>(&target_get),
        reinterpret_cast<void *>(&target_position),
        reinterpret_cast<void *>(&target_rank),
        reinterpret_cast<void *>(&target_plausible),
        reinterpret_cast<void *>(&target_direction),
        reinterpret_cast<void *>(&target_update),
        reinterpret_cast<void *>(&target_calculate),
        reinterpret_cast<void *>(&target_find),
        reinterpret_cast<void *>(&target_player_advance),
        reinterpret_cast<void *>(&target_cache),
        reinterpret_cast<void *>(&target_set),
        reinterpret_cast<void *>(&target_get),
        reinterpret_cast<void *>(&target_visible),
        reinterpret_cast<void *>(&target_viable),
        reinterpret_cast<void *>(&target_known),
        reinterpret_cast<void *>(&target_lost),
        reinterpret_cast<void *>(&target_clear),
        reinterpret_cast<void *>(&target_perfect),
        reinterpret_cast<void *>(&target_angle),
        reinterpret_cast<void *>(&target_range),
        reinterpret_cast<void *>(&target_viable_radius),
        reinterpret_cast<void *>(&target_aware_radius),
        reinterpret_cast<void *>(&target_search_active),
        reinterpret_cast<void *>(&target_search_end),
        reinterpret_cast<void *>(&target_search_start),
    };
    return table;
}

namespace {
void __fastcall targetable_285_destruct(targetable_inode_285 *self, void *)
{
    self->_destruct_mashed_class();
}
void *__fastcall targetable_285_delete(targetable_inode_285 *self, void *, unsigned flags)
{
    self->~targetable_inode_285();
    if (flags & 1)
        mash_virtual_base::operator delete(self, sizeof(targetable_inode_285));
    return self;
}
unsigned __fastcall targetable_285_type(targetable_inode_285 *, void *)
{
    return 285;
}
bool __fastcall targetable_285_subclass(targetable_inode_285 *, void *, unsigned type)
{
    return type == 537 || type == 573;
}
void __fastcall targetable_285_activate(targetable_inode_285 *self, void *, ai_core *core)
{
    self->_activate(core);
}
int __fastcall targetable_285_size(targetable_inode_285 *, void *)
{
    return sizeof(targetable_inode_285);
}
}  // namespace

void *targetable_inode_285::native_vtable()
{
    auto **base = static_cast<void **>(info_node::native_vtable());
    static void *table[] = {
        reinterpret_cast<void *>(&targetable_285_destruct),
        base[1],
        reinterpret_cast<void *>(&targetable_285_delete),
        reinterpret_cast<void *>(&targetable_285_type),
        reinterpret_cast<void *>(&targetable_285_subclass),
        base[5],
        base[6],
        base[7],
        reinterpret_cast<void *>(&targetable_285_activate),
        base[9],
        base[10],
        reinterpret_cast<void *>(&targetable_285_size),
    };
    return table;
}

mVectorBasic<vhandle_type<actor>> &targetable_inode_285::target_list()
{
    if constexpr (STANDALONE_SYSTEM) {
        struct registry : mVectorBasic<vhandle_type<actor>> {
            ~registry()
            {
                if (!is_pointer_in_mash_image(m_data))
                    ::operator delete[](m_data);
            }
        };

        static registry list;
        return list;
    } else {
        return var<mVectorBasic<vhandle_type<actor>>>(0x009585FC);
    }
}

targetable_inode_285::targetable_inode_285() : field_1C(0), field_20(0)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[285]);
}

targetable_inode_285::targetable_inode_285(from_mash_in_place_constructor *tag)
    : info_node(tag), field_1C(0), field_20(0)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[285]);
}

targetable_inode_285::~targetable_inode_285()
{
    unregister_as_targetable();
}

void targetable_inode_285::_activate(ai_core *core)
{
    info_node::_activate(core);

    target_list().push_back(vhandle_type<actor>{field_8->field_64->my_handle});
}

void targetable_inode_285::unregister_as_targetable()
{
    auto &list = target_list();
    if (list.size() == 0)
        return;
    const auto handle = field_8->field_64->my_handle;
    for (int i = 0; i < list.size(); ++i) {
        if (list.m_data[i].field_0 == handle) {
            std::memmove(list.m_data + i, list.m_data + i + 1, (list.size() - i - 1) * sizeof(vhandle_type<actor>));
            --list.m_size;
            return;
        }
    }
}

void targetable_inode_285::_destruct_mashed_class()
{
    unregister_as_targetable();
    info_node::_destruct_mashed_class();
}

void base_full_target_inode::_deactivate()
{
    delete field_28;
    field_28 = nullptr;
    field_2C = 0;
}

vhandle_type<actor> base_full_target_inode::find_target()
{
    const int budget = field_4C;
    field_4C = field_1C->size();
    field_50 = 0;
    field_38 = 7;
    field_48 = std::numeric_limits<float>::max();
    using update_fn = void(__fastcall *)(base_full_target_inode *);
    reinterpret_cast<update_fn>(get_vfunc(m_vtbl, 0x4C))(this);
    field_4C = budget;
    field_55 = false;
    return field_20 < field_38 ? vhandle_type<actor>{0} : field_34;
}

base_full_target_inode::base_full_target_inode()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[349]);
    field_20 = 3;
    field_2C = field_30 = field_50 = 0;
    field_34 = {0};
    field_38 = 7;
    last_known_position = {0.0f, 0.0f, 0.0f};
    field_48 = std::numeric_limits<float>::max();
    field_4C = 1;
    field_54 = field_55 = field_5C = allow_target_maintenance = perfect_perception = false;
    field_28 = nullptr;
}

base_full_target_inode::base_full_target_inode(from_mash_in_place_constructor *a2) : info_node(a2)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[349]);
    field_20 = 3;
    field_2C = field_30 = field_50 = 0;
    field_34 = {0};
    field_38 = 7;
    last_known_position = {0.0f, 0.0f, 0.0f};
    field_48 = std::numeric_limits<float>::max();
    field_4C = 1;
    field_54 = false;
    field_28 = nullptr;
}

void base_full_target_inode::register_as_targetable()
{
    const auto handle = field_C->my_handle;
    for (int i = 0; i < field_1C->size(); ++i) {
        if (field_1C->m_data[i].field_0 == handle)
            return;
    }
    field_1C->push_back(vhandle_type<actor>{handle});
}

void base_full_target_inode::unregister_as_targetable()
{
    const auto handle = field_C->my_handle;
    for (int i = 0; i < field_1C->size(); ++i) {
        if (field_1C->m_data[i].field_0 == handle) {
            std::memmove(field_1C->m_data + i,
                         field_1C->m_data + i + 1,
                         (field_1C->size() - i - 1) * sizeof(vhandle_type<actor>));
            --field_1C->m_size;
            return;
        }
    }
}

void base_full_target_inode::update_cached_params(bool force)
{
    if (!force && my_param_block.field_0 < g_world_ptr->time_manager.field_C - 1)
        return;
    perfect_perception =
        my_param_block.get_optional_pb_int(string_hash{int(to_hash("perfect_perception"))}, 0, nullptr) != 0;
    const float angle = my_param_block.get_optional_pb_float(string_hash{int(to_hash("vision_ang"))}, 45.0f, nullptr);
    vision_angle_cos = angle < 360.0f ? static_cast<float>(std::cos(angle * (3.14159265358979323846 / 180.0))) : -1.0f;
    const float range = my_param_block.get_optional_pb_float(string_hash{int(to_hash("vision_range"))}, 20.0f, nullptr);
    vision_range = range < 75.0f ? range : 75.0f;
    const float close_angle =
        my_param_block.get_optional_pb_float(string_hash{int(to_hash("close_vision_ang"))}, 60.0f, nullptr);
    close_vision_angle_cos =
        close_angle < 360.0f ? static_cast<float>(std::cos(close_angle * (3.14159265358979323846 / 180.0))) : -1.0f;
    close_vision_range =
        my_param_block.get_optional_pb_float(string_hash{int(to_hash("close_vision_range"))}, 3.0f, nullptr);
    viable_radius = my_param_block.get_optional_pb_float(string_hash{int(to_hash("viable_radius"))}, 8.0f, nullptr);
    const float aware = my_param_block.get_optional_pb_float(string_hash{int(to_hash("aware_radius"))}, 20.0f, nullptr);
    aware_radius = aware < 75.0f ? aware : 75.0f;
    ellipse_length = my_param_block.get_optional_pb_float(string_hash{int(to_hash("ellipse_length"))}, 7.0f, nullptr);
    ellipse_width = my_param_block.get_optional_pb_float(string_hash{int(to_hash("ellipse_width"))}, 4.0f, nullptr);
    touch_range = my_param_block.get_optional_pb_float(string_hash{int(to_hash("touch_range"))}, 1.0f, nullptr);
}

void base_full_target_inode::reset_target_selection_delay()
{
    if (target_selection_delay >= 0.0f) {
        target_selection_delay =
            field_8->field_50.get_optional_pb_float(string_hash{int(to_hash("target_selection_time"))}, -1.0f, nullptr);

        target_selection_delay = static_cast<float>(
            (std::rand() * static_cast<double>(1.0f / 32767.0f) * 0.20000004768371582f + 0.89999997615814209f) *
            target_selection_delay);
    }
}

void base_full_target_inode::_activate(ai_core *core)
{
    info_node::_activate(core);
    using register_fn = void(__fastcall *)(base_full_target_inode *);
    reinterpret_cast<register_fn>(get_vfunc(m_vtbl, 0x34))(this);
    field_55 = false;
    field_24 = reinterpret_cast<std::intptr_t>(field_8->get_info_node(controller_inode::default_id, false));
    allow_target_maintenance =
        field_8->field_50.get_optional_pb_int(string_hash{int(to_hash("allow_target_maintance"))}, 1, nullptr) != 0;
    using cache_fn = void(__fastcall *)(base_full_target_inode *, void *, bool);
    reinterpret_cast<cache_fn>(get_vfunc(m_vtbl, 0x5C))(this, nullptr, true);
    if (field_8->field_50.get_optional_pb_float(string_hash{int(to_hash("target_selection_time"))}, -1.0f, nullptr) <
        0.0f) {
        target_selection_delay = -1.0f;
    } else {
        target_selection_delay = 0.0f;
        reset_target_selection_delay();
    }
    if (target_selection_delay >= 0.0f)
        target_selection_delay =
            static_cast<float>(std::rand() * static_cast<double>(1.0f / 32767.0f) * 0.5 * target_selection_delay);
    field_28 = nullptr;
    field_5C = false;
}

vector3d base_full_target_inode::get_controller_look_direction()
{
    if (field_24)
        return reinterpret_cast<controller_inode *>(field_24)->get_axis(
            static_cast<controller_inode::eControllerAxis>(2));
    return field_C->get_abs_po().get_z_facing();
}

void base_full_target_inode::start_multi_frame_search(int budget)
{
    reset_target_selection_delay();
    field_4C = budget;
    field_50 = 0;
    field_54 = true;
}

void base_full_target_inode::update_targeting()
{
    int index = 0;
    const vhandle_type<actor> previous{entity_base_vhandle{static_cast<uint32_t>(field_50)}};
    if (previous.get_volatile_ptr()) {
        while (index < field_1C->size() && field_1C->m_data[index].field_0 != previous.field_0)
            ++index;
        if (index < field_1C->size())
            ++index;
    }
    using plausible_fn = bool(__fastcall *)(base_full_target_inode *, void *, vhandle_type<actor>);
    using calculate_fn = void(__fastcall *)(base_full_target_inode *, void *, vhandle_type<actor>);
    int considered = 0;
    while (considered < field_4C && index < field_1C->size()) {
        const auto candidate = field_1C->m_data[index++];
        field_50 = candidate.field_0.field_0;
        if (reinterpret_cast<plausible_fn>(get_vfunc(m_vtbl, 0x44))(this, nullptr, candidate)) {
            ++considered;
            reinterpret_cast<calculate_fn>(get_vfunc(m_vtbl, 0x50))(this, nullptr, candidate);
        }
    }
    if (index == field_1C->size())
        field_54 = false;
}

void base_full_target_inode::_frame_advance(Float delta)
{
    using cache_fn = void(__fastcall *)(base_full_target_inode *, void *, bool);
    using active_fn = bool(__fastcall *)(base_full_target_inode *);
    using search_fn = void(__fastcall *)(base_full_target_inode *, void *, int);
    using plausible_fn = bool(__fastcall *)(base_full_target_inode *, void *, vhandle_type<actor>);
    using calculate_fn = void(__fastcall *)(base_full_target_inode *, void *, vhandle_type<actor>);
    using update_fn = void(__fastcall *)(base_full_target_inode *);
    reinterpret_cast<cache_fn>(get_vfunc(m_vtbl, 0x5C))(this, nullptr, false);
    if (target_selection_delay >= 0.0f && !reinterpret_cast<active_fn>(get_vfunc(m_vtbl, 0x90))(this)) {
        target_selection_delay -= delta;
        if (target_selection_delay <= 0.0f) {
            target_selection_delay = 0.0f;
            reinterpret_cast<search_fn>(get_vfunc(m_vtbl, 0x98))(this, nullptr, 1);
        }
    }
    const auto maintain = [this] {
        field_48 = std::numeric_limits<float>::max();
        field_38 = 6;
        if (reinterpret_cast<plausible_fn>(get_vfunc(m_vtbl, 0x44))(this, nullptr, field_34))
            reinterpret_cast<calculate_fn>(get_vfunc(m_vtbl, 0x50))(this, nullptr, field_34);
    };
    if (allow_target_maintenance)
        maintain();
    if (field_54) {
        if (!allow_target_maintenance)
            maintain();
        reinterpret_cast<update_fn>(get_vfunc(m_vtbl, 0x4C))(this);
    }
}

void base_full_target_inode::player_style_frame_advance(Float delta)
{
    using active_fn = bool(__fastcall *)(base_full_target_inode *);
    using search_fn = void(__fastcall *)(base_full_target_inode *, void *, int);
    using clear_fn = void(__fastcall *)(base_full_target_inode *);
    if (!reinterpret_cast<active_fn>(get_vfunc(m_vtbl, 0x90))(this))
        reinterpret_cast<search_fn>(get_vfunc(m_vtbl, 0x98))(this, nullptr, 1);
    base_full_target_inode::_frame_advance(delta);
    if (field_38 >= 6)
        reinterpret_cast<clear_fn>(get_vfunc(m_vtbl, 0x78))(this);
}

vhandle_type<actor> base_full_target_inode::quick_targeting()
{
    return this->field_34;
}

bool base_full_target_inode::is_target_known()
{
    return this->field_38 <= 5;
}

}  // namespace ai
