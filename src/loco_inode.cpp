#include "loco_inode.h"

#include "base_ai_core.h"
#include "common.h"
#include "mash_info_struct.h"
#include "vtbl.h"
#include "native_info_node_table.h"
#include "als_inode.h"
#include "param_list.h"

namespace ai {

VALIDATE_SIZE(loco_inode, 0x64);
VALIDATE_SIZE(biped_layer_inode, 0x64);
VALIDATE_SIZE(nonpath_loco_inode, 0x64);

namespace {
const string_hash default_goto_speed_id{static_cast<int>(to_hash("default_goto_speed"))};
const string_hash default_goto_radius_id{static_cast<int>(to_hash("default_goto_radius"))};
const string_hash default_min_goto_time_id{static_cast<int>(to_hash("default_min_goto_time"))};
const string_hash always_update_als_id{static_cast<int>(to_hash("always_update_als"))};
const string_hash idle_walk_run_id{static_cast<int>(to_hash("Idle_Walk_Run"))};
const resource_key unset_graph{string_hash{0}, RESOURCE_KEY_TYPE_AI_STATE_GRAPH};
const resource_key biped_graph{string_hash{static_cast<int>(to_hash("biped_layer"))},
                             RESOURCE_KEY_TYPE_AI_STATE_GRAPH};
const resource_key nonpath_graph{string_hash{static_cast<int>(to_hash("nonpath_loco_layer"))},
                               RESOURCE_KEY_TYPE_AI_STATE_GRAPH};

void __fastcall native_loco_destruct(loco_inode *self, void *)
{
    self->als_category.destruct_mashed_class();
    self->_destruct_mashed_class();
}
void __fastcall native_loco_unmash(loco_inode *self, void *, mash_info_struct *info, void *base)
{
    self->_unmash(info, base);
}
void __fastcall native_loco_activate(loco_inode *self, void *, ai_core *core)
{
    self->_activate(core);
}
void __fastcall native_loco_initialize(loco_inode *, void *) {}
void __fastcall native_loco_defaults(loco_inode *self, void *) { self->_reset_loco_defaults(); }
template<class T>
const resource_key &__fastcall native_loco_graph(const T *self, void *) { return self->_get_graph(); }
template<class T>
void __fastcall native_layer_initialize(T *self, void *) { self->_initialize_loco_inode(); }

template<class T, unsigned Parent>
native_inode::table<T, T::virtual_type, Parent, 15> make_loco_table()
{
    native_inode::table<T, T::virtual_type, Parent, 15> result;
    result[0] = reinterpret_cast<void *>(&native_loco_destruct);
    result[1] = reinterpret_cast<void *>(&native_loco_unmash);
    result[8] = reinterpret_cast<void *>(&native_loco_activate);
    result[12] = reinterpret_cast<void *>(&native_loco_initialize);
    result[13] = reinterpret_cast<void *>(&native_loco_graph<T>);
    result[14] = reinterpret_cast<void *>(&native_loco_defaults);
    return result;
}
}

void *loco_inode::native_vtable()
{

    static auto table = make_loco_table<loco_inode, 537>();
    return table.data();
}

void *biped_layer_inode::native_vtable()
{
    static auto table = [] {
        auto result = make_loco_table<biped_layer_inode, loco_inode::virtual_type>();
        result[12] = reinterpret_cast<void *>(&native_layer_initialize<biped_layer_inode>);
        return result;
    }();
    return table.data();
}

void *nonpath_loco_inode::native_vtable()
{
    static auto table = [] {
        auto result = make_loco_table<nonpath_loco_inode, loco_inode::virtual_type>();
        result[12] = reinterpret_cast<void *>(&native_layer_initialize<nonpath_loco_inode>);
        return result;
    }();
    return table.data();
}

loco_inode::loco_inode()
    : info_node(), als_category{0}, goto_destination{ZEROVEC.x, ZEROVEC.y, ZEROVEC.z},
      field_54(false), field_55(false), field_56(false), needs_repathfind(false),
      field_58(false), field_59(false), allow_facing_change(false),
      explicit_goto_speed(false), explicit_goto_radius(false), explicit_min_goto_time(false),
      field_5E(false), field_5F(false), always_update_als(false)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[virtual_type]);
}

biped_layer_inode::biped_layer_inode() : loco_inode()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[virtual_type]);
}

nonpath_loco_inode::nonpath_loco_inode() : loco_inode()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[virtual_type]);
}

loco_inode::loco_inode(from_mash_in_place_constructor *tag) : info_node(tag), als_category(tag)
{

    _reset_loco_defaults();
    field_50 = -1.0f;
    allow_facing_change = false;
    field_55 = false;
    field_59 = false;
    always_update_als = false;
}

void loco_inode::initialize_loco_inode()
{
    using callback = void (__fastcall *)(loco_inode *, void *);
    reinterpret_cast<callback>(get_vfunc(m_vtbl, 0x30))(this, nullptr);
}

const resource_key &loco_inode::get_graph() const
{
    using callback = const resource_key &(__fastcall *)(const loco_inode *, void *);
    return reinterpret_cast<callback>(get_vfunc(m_vtbl, 0x34))(this, nullptr);
}

void loco_inode::reset_loco_defaults()
{
    using callback = void (__fastcall *)(loco_inode *, void *);
    reinterpret_cast<callback>(get_vfunc(m_vtbl, 0x38))(this, nullptr);
}

void loco_inode::_activate(ai_core *core)
{
    info_node::_activate(core);
    const int core_value = core->field_50.get_optional_pb_int(always_update_als_id, 0, nullptr);
    if (my_param_block.get_optional_pb_int(always_update_als_id, core_value, nullptr))
        always_update_als = true;
}

void loco_inode::_unmash(mash_info_struct *info, void *base)
{
    info_node::_unmash(info, base);
    info->unmash_class_in_place(als_category, this);
}

const resource_key &loco_inode::_get_graph() const
{
    return unset_graph;
}

void loco_inode::set_goto_speed(float speed)
{
    goto_speed = speed;
    explicit_goto_speed = true;
    if (goto_speed < 0.0f) {
        explicit_goto_speed = false;
        goto_speed = my_param_block.get_optional_pb_float(default_goto_speed_id, 4.0f, nullptr);
    }
}

void loco_inode::set_goto_radius(float radius)
{
    goto_radius = radius;
    explicit_goto_radius = true;
    if (goto_radius < 0.0f) {
        explicit_goto_radius = false;
        goto_radius = my_param_block.get_optional_pb_float(default_goto_radius_id, 1.5f, nullptr);
    }
    if (goto_radius < 0.25f)
        goto_radius = 0.25f;
}

void loco_inode::set_min_goto_time(float time)
{
    min_goto_time = time;
    explicit_min_goto_time = true;
    if (min_goto_time < 0.0f) {
        explicit_min_goto_time = false;

        min_goto_time = my_param_block.get_optional_pb_float(default_min_goto_time_id, 0.0f, nullptr);
    }
}

void loco_inode::_reset_loco_defaults()
{
    set_goto_speed(-1.0f);
    set_goto_radius(-1.0f);
    field_24 = -1.0f;
    field_20 = -1.0f;
    set_min_goto_time(-1.0f);
    field_40 = 0;
    field_56 = false;
    allow_facing_change = false;
    field_5F = false;
    field_54 = true;
}

biped_layer_inode::biped_layer_inode(from_mash_in_place_constructor *tag) : loco_inode(tag) {}

void biped_layer_inode::_initialize_loco_inode()
{
    reset_loco_defaults();
    field_58 = true;
    als_category = idle_walk_run_id;
}

const resource_key &biped_layer_inode::_get_graph() const
{
    return biped_graph;
}

nonpath_loco_inode::nonpath_loco_inode(from_mash_in_place_constructor *tag) : loco_inode(tag) {}

void nonpath_loco_inode::_initialize_loco_inode()
{
    reset_loco_defaults();
    field_58 = true;
    als_category = idle_walk_run_id;
}

const resource_key &nonpath_loco_inode::_get_graph() const
{
    return nonpath_graph;
}

void loco_inode::set_facing_dir(const vector3d &direction)
{

    if (!allow_facing_change)
        return;
    auto *animation = static_cast<als_inode *>(field_8->get_info_node(als_inode::default_id, true));
    als::param_list params;
    params.add_param(0x1B, direction);
    animation->set_desired_params(params, static_cast<als::layer_types>(0));
    params.clear();
}

}
