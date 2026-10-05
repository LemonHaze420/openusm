#include "prop_physics_inode.h"

#include "actor.h"
#include "common.h"
#include "mash_config.h"
#include "mash_info_struct.h"
#include "physical_interface.h"
#include "wds.h"
#include "conglom.h"
#include "memory.h"
#include "prop_system.h"
#include "physics/include/nuge.h"
#include "physics/include/phys_vector3d.h"
#include "physics/include/rigid_body.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>

#include <new>

namespace ai {

VALIDATE_SIZE(prop_physics_inode, 0x30);
VALIDATE_SIZE(prop_physics_inode::prop_record, 0x18);

#if STANDALONE_SYSTEM
namespace {
void __fastcall prop_mashed_destruct(prop_physics_inode *node)
{
    node->destruct_mashed_class();
}
void __fastcall prop_unmash(prop_physics_inode *node, void *, mash_info_struct *info, void *owner)
{
    node->_unmash(info, owner);
}
prop_physics_inode *__fastcall prop_delete(prop_physics_inode *node, void *, unsigned char flags)
{
    node->~prop_physics_inode();
    if ((flags & 1) != 0)
        mem_dealloc(node, sizeof(*node));
    return node;
}
int __fastcall prop_type(const prop_physics_inode *)
{
    return 403;
}
bool __fastcall prop_subclass(const prop_physics_inode *, void *, int type)
{
    return type == 537 || type == 573;
}
bool __fastcall prop_needs_advance(const prop_physics_inode *)
{
    return true;
}
void __fastcall prop_advance(prop_physics_inode *node, void *, Float elapsed)
{
    node->_frame_advance(elapsed);
}
void __fastcall prop_deactivate(prop_physics_inode *node)
{
    node->_deactivate();
}
int __fastcall prop_size(const prop_physics_inode *)
{
    return sizeof(prop_physics_inode);
}
}  // namespace

void *prop_physics_inode::native_vtable()
{
    static std::array<void *, 12> table = [] {
        std::array<void *, 12> result;
        std::copy_n(static_cast<void **>(info_node::native_vtable()), result.size(), result.begin());
        result[0x00 / 4] = reinterpret_cast<void *>(&prop_mashed_destruct);
        result[0x04 / 4] = reinterpret_cast<void *>(&prop_unmash);
        result[0x08 / 4] = reinterpret_cast<void *>(&prop_delete);
        result[0x0C / 4] = reinterpret_cast<void *>(&prop_type);
        result[0x18 / 4] = reinterpret_cast<void *>(&prop_needs_advance);
        result[0x10 / 4] = reinterpret_cast<void *>(&prop_subclass);
        result[0x1C / 4] = reinterpret_cast<void *>(&prop_advance);
        result[0x24 / 4] = reinterpret_cast<void *>(&prop_deactivate);
        result[0x2C / 4] = reinterpret_cast<void *>(&prop_size);
        return result;
    }();
    return table.data();
}
#else
void *prop_physics_inode::native_vtable()
{
    return reinterpret_cast<void *>(0x0087DCA4);
}
#endif


prop_physics_inode::prop_physics_inode()
    : info_node(), records(), records_data(nullptr), records_capacity(0), field_2C(false)
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[403]);
#else
    m_vtbl = 0x0087DCA4;
#endif
}

prop_physics_inode::prop_physics_inode(from_mash_in_place_constructor *constructor)
    : info_node(constructor), records(constructor)
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[403]);
#else
    m_vtbl = 0x0087DCA4;
#endif
}

prop_physics_inode::~prop_physics_inode()
{
    clear_records();
}

void prop_physics_inode::_unmash(mash_info_struct *info, void *owner)
{
    info_node::_unmash(info, owner);
#if OPENUSM_XBOX_MASH_FORMAT && !defined(OPENUSM_XBPACK_V10)
    records.m_size = *reinterpret_cast<int *>(info->read_from_buffer(mash::SHARED_BUFFER, 4, 4));
#endif
    if (records_data)
        records_data = reinterpret_cast<prop_record *>(info->read_from_buffer(sizeof(prop_record) * records.m_size, 4));
    records.field_0 = static_cast<int>(info->mash_image_ptr[0] + info->buffer_size_used[0] -
                                       reinterpret_cast<unsigned char *>(&records));
}

namespace {

void apply_prop_ballistic_target(rigid_body *body, const float *target)
{
    math::VecClass<3, 1> position;
    math::VecClass<3, 0> momentum;
    float mass;
    nuge::get_ballistic_info(&body, 1, &position, &momentum, &mass);
    const float velocity_x = momentum[0] / mass;
    const float velocity_y = momentum[1] / mass;
    const float velocity_z = momentum[2] / mass;
    const float delta_x = target[0] - position[0];
    const float delta_y = target[1] - position[1];
    const float delta_z = target[2] - position[2];
    const float distance = std::sqrt(delta_x * delta_x + delta_z * delta_z);
    if (distance < 0.001f)
        return;
    static Var<float> half_gravity{0x008BFE70};
    const float gravity_distance = half_gravity() * distance;
    const float slope = delta_y / distance;
    const float direction_x = delta_x / distance;
    const float direction_z = delta_z / distance;
    const float horizontal_velocity = direction_x * velocity_x + direction_z * velocity_z;
    float speed = 1.0f;
    for (int iteration = 0; iteration <= 10; ++iteration) {
        const float x = speed - horizontal_velocity;
        const float y = slope * speed - gravity_distance / speed - velocity_y;
        const float derivative = gravity_distance / (speed * speed) + slope;
        const float next = speed - (y * derivative + x) / (y * (-2.0f * gravity_distance / (speed * speed * speed)) +
                                                           derivative * derivative + 1.0f);
        const bool converged = std::fabs(next - speed) < 0.001f;
        speed = next;
        if (converged)
            break;
    }
    if (std::fabs(speed) < 1.0f && std::fabs(speed) > 0.001f)
        speed = speed < 0.0f ? -1.0f : 1.0f;
    phys_vector3d impulse;
    impulse[0] = (direction_x * speed - velocity_x) * mass;
    impulse[1] = (slope * speed - gravity_distance / speed - velocity_y) * mass;
    impulse[2] = (direction_z * speed - velocity_z) * mass;
    body->sub_5B2D50(impulse);
}


bool prop_physics_at_rest(actor *prop)
{
    if (!prop || !prop->has_physical_ifc())
        return true;
    auto *body = prop->physical_ifc()->field_174;
    if (!body)
        return true;
    bool resting = (body->body->field_144 & 4) != 0;
    if ((prop->field_4 & 4) != 0) {
        for (auto *member : static_cast<conglomerate *>(prop)->members) {
            if (!resting)
                break;
            if (member->is_an_actor() && member->has_physical_ifc()) {
                auto *member_body = member->physical_ifc()->field_174;
                if (member_body)
                    resting = (member_body->body->field_144 & 4) != 0;
            }
        }
    }
    return resting;
}
}  // namespace

void prop_physics_inode::_frame_advance(Float elapsed_seconds)
{
    int index = 0;
    while (index != records.m_size) {
        auto &record = records_data[index];
        auto *prop = static_cast<actor *>(record.actor_handle.get_volatile_ptr());
        bool running = false;
        if (prop && prop->physical_ifc()->is_prop_physics_running()) {
            if (record.target_pending) {
                if (record.target_delay < 0.0f) {
                    apply_prop_ballistic_target(prop->physical_ifc()->field_174->body, record.ballistic_target);
                    record.target_pending = false;
                } else {
                    record.target_delay -= elapsed_seconds;
                }
            }
            if (prop_physics_at_rest(prop))
                prop->physical_ifc()->stop_prop_physics(false);
            else
                running = true;
        }
        if (running) {
            ++index;
        } else {
            if (prop && prop->physical_ifc()->is_prop_physics_running())
                prop->physical_ifc()->stop_prop_physics(false);
            std::memmove(
                records_data + index, records_data + index + 1, sizeof(prop_record) * (records.m_size - index - 1));
            --records.m_size;
        }
    }
}

void prop_physics_inode::clear_records()
{
    if (!records.is_pointer_in_mash_image(records_data))
        ::operator delete[](records_data);
    records_data = nullptr;
    records_capacity = 0;
    records.clear();
}

void prop_physics_inode::_deactivate()
{
    for (int i = 0; i != records.m_size; ++i) {
        auto *prop = static_cast<actor *>(records_data[i].actor_handle.get_volatile_ptr());
        if (prop) {
            auto *physics = prop->physical_ifc();
            if (physics->is_prop_physics_running())
                physics->stop_prop_physics(false);
            g_world_ptr->ent_mgr.destroy_entity(prop);
        }
    }
    clear_records();
}

void prop_physics_inode::destruct_mashed_class()
{
    clear_records();
    records.destruct_mashed_class();
    info_node::_destruct_mashed_class();
}

}  // namespace ai
