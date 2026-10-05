#include "spidey_combat_inode.h"

#include "aeps.h"
#include "ai_tentacle_info.h"
#include "base_ai_core.h"
#include "common.h"
#include "conglom.h"
#include "func_wrapper.h"
#include "memory.h"
#include "polytube.h"
#include "polytubecustommaterial.h"
#include "variables.h"
#include "vtbl.h"
#include "wds.h"

#include <algorithm>
#include <array>
#include <new>


struct spidey_combat_web {
    entity *source;
    entity_base *target_bone;
    ai_tentacle_info tentacle;
    int field_E0;
    bool retracting;
    bool finished;
    vhandle_type<actor> target;
    vector3d target_position;
    float timer;

    spidey_combat_web(entity *owner, ai::ai_core *core, vhandle_type<actor> target_handle, vector3d position)
        : source(owner), tentacle(core), retracting(false), finished(false), target(target_handle),
          target_position(position), timer(0.0f)
    {
        if (auto *actor = target.get_volatile_ptr())
            target_bone = static_cast<conglomerate *>(actor)->get_bone(string_hash{"BIP01 SPINE1"}, true);
        tentacle.field_A8 |= 1;
        tentacle.init_positions(true);
        tentacle.set_code_blend(1.0f, 0.0f);
        update_positions();
        tentacle.base_node = nullptr;
        tentacle.end_node = nullptr;
        tentacle.positions.m_data = new vector3d[5]{};
        tentacle.positions.m_size = 5;
        tentacle.tween_positions.m_data = new vector3d[5]{};
        tentacle.tween_positions.m_size = 5;
        tentacle.render_info->num_sides = 2;
        tentacle.render_info->radius = 0.1f;
        tentacle.render_info->texture_scale = 1.5f;
        tentacle.render_info->spline_flags = 2;
        tentacle.create_tentacle(nullptr);
        if (webline_texture != nullptr) {
            tentacle.tentacle->set_material(webline_texture);
            tentacle.tentacle->field_D0->m_blend_mode = static_cast<nglBlendModeType>(2);
        }
    }

    ~spidey_combat_web()
    {
        tentacle.kill_all_engines();
    }

    vector3d endpoint()
    {
        if (var<bool>(0x96BE78))
            target_position = var<vector3d>(0x96BFF8);
        else if (target.get_volatile_ptr() != nullptr)
            target_position = target_bone->get_abs_position();
        return target_position;
    }

    void update_positions()
    {
        const vector3d origin = source->get_abs_position();
        const vector3d destination = endpoint();
        float amount = timer / 0.13300000131130219f;
        if (amount >= 1.0f) {
            if (retracting)
                finished = true;
            amount = 0.99989998f;
        }
        tentacle.field_60 = retracting ? origin + (destination - origin) * amount : origin;
        if (tentacle.tentacle != nullptr && (tentacle.field_A8 & 0x100) == 0 &&
            tentacle.tentacle == reinterpret_cast<polytube *>(tentacle.my_ai != nullptr
                                                                  ? static_cast<entity_base *>(tentacle.my_ai->field_64)
                                                                  : tentacle.tentacle))
            tentacle.tentacle->set_abs_position(tentacle.field_60);
        tentacle.end_pos = retracting ? destination : origin + (destination - origin) * amount;
        tentacle.create_line(tentacle.end_pos, nullptr);
    }

    void frame_advance(Float delta)
    {
        timer += delta;
        update_positions();
        tentacle.frame_advance(delta);
        tentacle.update_spline();
    }

    void retract()
    {
        if (!retracting) {
            retracting = true;
            timer = 0.0f;
        }
    }
};

VALIDATE_SIZE(spidey_combat_web, 0xFC);
VALIDATE_OFFSET(spidey_combat_web, retracting, 0xE4);
VALIDATE_OFFSET(spidey_combat_web, timer, 0xF8);

namespace ai {
VALIDATE_SIZE(spidey_combat_inode, 0x350);

namespace {
void delete_web(spidey_combat_web *&web)
{
    if (web == nullptr)
        return;
    web->~spidey_combat_web();
    if (slab_allocator::get_max_object_size() >= sizeof(spidey_combat_web))
        slab_allocator::deallocate(web, nullptr);
    else
        ::operator delete(web);
    web = nullptr;
}
void __fastcall spidey_destroy(spidey_combat_inode *self, void *)
{
    self->_destruct_mashed_class();
}
void *__fastcall spidey_delete(spidey_combat_inode *self, void *, unsigned flags)
{
    self->~spidey_combat_inode();
    if (flags & 1)
        ::operator delete(self);
    return self;
}
unsigned __fastcall spidey_type(spidey_combat_inode *, void *)
{
    return 248;
}
bool __fastcall spidey_subclass(spidey_combat_inode *, void *, unsigned type)
{
    return type == 346 || type == 342 || type == 537 || type == 573;
}
void __fastcall spidey_frame(spidey_combat_inode *self, void *, Float dt)
{
    self->_frame_advance(dt);
}
void __fastcall spidey_activate(spidey_combat_inode *self, void *, ai_core *core)
{
    self->_activate(core);
}
void __fastcall spidey_deactivate(spidey_combat_inode *self, void *)
{
    self->_deactivate();
}
int __fastcall spidey_size(spidey_combat_inode *, void *)
{
    return sizeof(spidey_combat_inode);
}
int __fastcall spidey_sense(spidey_combat_inode *self, void *)
{
    return self->field_338;
}
void __fastcall spidey_activate_sense(spidey_combat_inode *self, void *)
{
    self->activate_sense();
}
void __fastcall spidey_web(spidey_combat_inode *self, void *, entity *source, float x, float y, float z)
{
    self->activate_web(source, x, y, z);
}
void __fastcall spidey_unweb(spidey_combat_inode *self, void *, entity *source)
{
    self->deactivate_web(source);
}
void __fastcall spidey_pending(spidey_combat_inode *self, void *, combat_inode::incoming_move move)
{
    self->update_pending_move(move);
}
}  // namespace

void *spidey_combat_inode::native_vtable()
{
    static auto table = [] {
        std::array<void *, 76> result;
        std::copy_n(static_cast<void **>(player_combat_inode::native_vtable()), result.size(), result.data());
        result[0] = reinterpret_cast<void *>(&spidey_destroy);
        result[2] = reinterpret_cast<void *>(&spidey_delete);
        result[3] = reinterpret_cast<void *>(&spidey_type);
        result[4] = reinterpret_cast<void *>(&spidey_subclass);
        result[7] = reinterpret_cast<void *>(&spidey_frame);
        result[8] = reinterpret_cast<void *>(&spidey_activate);
        result[9] = reinterpret_cast<void *>(&spidey_deactivate);
        result[11] = reinterpret_cast<void *>(&spidey_size);
        result[22] = reinterpret_cast<void *>(&spidey_sense);
        result[23] = reinterpret_cast<void *>(&spidey_activate_sense);
        result[24] = reinterpret_cast<void *>(&spidey_web);
        result[25] = reinterpret_cast<void *>(&spidey_unweb);
        result[54] = reinterpret_cast<void *>(&spidey_pending);
        return result;
    }();
    return table.data();
}

spidey_combat_inode::spidey_combat_inode() : player_combat_inode()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[248]);
    field_334 = 0;
    field_348 = field_34C = nullptr;
}
spidey_combat_inode::spidey_combat_inode(from_mash_in_place_constructor *tag) : player_combat_inode(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[248]);
    field_334 = 0;
    field_348 = field_34C = nullptr;
}
spidey_combat_inode::~spidey_combat_inode()
{
    finalize();
}

void spidey_combat_inode::finalize()
{
    delete_web(field_348);
    delete_web(field_34C);
}
void spidey_combat_inode::_destruct_mashed_class()
{
    finalize();
    combat_inode::_destruct_mashed_class();
}
void spidey_combat_inode::_activate(ai_core *core)
{
    player_combat_inode::_activate(core);
    field_330 = -1.0f;
    field_334 = field_338 = 0;
    field_348 = field_34C = nullptr;
}
void spidey_combat_inode::_frame_advance(Float delta)
{
    if (field_338 > 0)
        --field_338;
    player_combat_inode::_frame_advance(delta);
    using sense_fn = int(__fastcall *)(spidey_combat_inode *, void *);
    if (reinterpret_cast<sense_fn>(get_vfunc(m_vtbl, 0x58))(this, nullptr))
        aeps::DoSpideySenseEffect(field_C, 0.1f, 0);
    for (auto **web : {&field_348, &field_34C}) {
        if (*web == nullptr)
            continue;
        if ((*web)->finished)
            delete_web(*web);
        else {
            (*web)->frame_advance(delta);
            if (!has_cur_move())
                (*web)->retract();
        }
    }
}
void spidey_combat_inode::_deactivate()
{
    combat_inode::_deactivate();
    for (auto **web : {&field_348, &field_34C}) {
        if (*web != nullptr) {
            (*web)->retract();
            (*web)->frame_advance(1.0f);
            delete_web(*web);
        }
    }
}
void spidey_combat_inode::activate_web(entity *source, float x, float y, float z)
{
    if (field_348 != nullptr && field_348->source == source)
        delete_web(field_348);
    if (field_34C != nullptr && field_34C->source == source)
        delete_web(field_34C);
    const vhandle_type<actor> target{field_20};
    vector3d position;
    if (auto *actor = target.get_volatile_ptr())
        position = actor->get_abs_position();
    else
        position = (field_C->get_abs_po().get_z_facing() * 10.0f + field_C->get_abs_position()) * 0.25f +
                   vector3d{x, y, z} * 0.75f;
    auto **slot = field_348 == nullptr ? &field_348 : field_34C == nullptr ? &field_34C : nullptr;
    if (slot != nullptr) {
        void *memory = sizeof(spidey_combat_web) <= slab_allocator::get_max_object_size()
                           ? slab_allocator::allocate(sizeof(spidey_combat_web), nullptr)
                           : ::operator new(sizeof(spidey_combat_web));
        *slot = ::new (memory) spidey_combat_web(source, field_8, target, position);
    }
}
void spidey_combat_inode::deactivate_web(entity *source)
{
    if (field_348 != nullptr && field_348->source == source)
        field_348->retract();
    if (field_34C != nullptr && field_34C->source == source)
        field_34C->retract();
}
void spidey_combat_inode::update_pending_move(const combat_inode::incoming_move &move)
{
    combat_inode::update_pending_move(move);
    const float time = g_world_ptr->time_manager.field_8;
    if (!(field_330 <= -1.0f && field_330 >= -1.0f) && time >= field_330) {
        field_330 = -1.0f;
        field_334 = 0;
    }
    using sense_fn = int(__fastcall *)(spidey_combat_inode *, void *);
    if (reinterpret_cast<sense_fn>(get_vfunc(m_vtbl, 0x58))(this, nullptr)) {
        field_334 = move.field_4;
        field_330 = time + 0.1f;
    }
}
}  // namespace ai

void spidey_combat_inode_patch()
{
    FUNC_ADDRESS(address, &ai::spidey_combat_inode::_activate);
    set_vfunc(0x0087D720, address);
}
