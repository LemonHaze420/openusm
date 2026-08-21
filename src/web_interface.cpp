#include "web_interface.h"

#include "actor.h"
#include "ai_std_combat_target.h"
#include "common.h"
#include "conglom.h"
#include "func_wrapper.h"
#include "memory.h"
#include "oldmath_po.h"
#include "resource_manager.h"
#include "trace.h"
#include "utility.h"
#include "wds.h"
#include "wds_entity_manager.h"

#include <cassert>

VALIDATE_SIZE(web_info_nugget, 0x20u);
VALIDATE_SIZE(web_interface, 0x20u);

web_info_nugget::web_info_nugget(mString a2, Float fade_in_time, Float a4)
{
    assert(fade_in_time >= 0.0f);

    this->field_C = a4;
    this->field_8 = fade_in_time;

    this->field_4 = 0;
    this->field_10 = 0;
    this->field_18 = string_hash{a2.c_str()};
    this->field_1C = false;
}

web_info_nugget::web_info_nugget(from_mash_in_place_constructor *a2) : field_18(a2)
{
    this->initialize(mash::FROM_MASH);
}

void web_info_nugget::initialize(mash::allocation_scope)
{
    this->field_1C = false;
}


void web_info_nugget::create_web_entity()
{
    assert(this->my_actor != nullptr);

    auto __old_context = resource_manager::push_resource_context(this->my_actor->get_resource_context());
    mString a3{};
    auto &v5 = po_identity_matrix;
    auto v4 = make_unique_entity_id();
    auto v3 = this->field_18;
    this->m_web_entity = g_world_ptr->ent_mgr.create_and_add_entity_or_subclass(v3, v4, v5, a3, 1u, nullptr);

    resource_manager::pop_resource_context();

    assert(resource_manager::get_resource_context() == __old_context);

    if (this->m_web_entity != nullptr) {
        this->m_web_entity->set_visible(false, false);
        this->m_web_entity->clear_parent(true);
        this->m_web_entity->remove_from_regions();
    } else {
        this->m_web_entity = nullptr;
    }
}

void web_info_nugget::destroy_web_entity()
{
    if (this->m_web_entity != nullptr) {
        m_web_entity->set_visible(false, false);
        this->m_web_entity->set_parent(nullptr);
        this->m_web_entity->remove_from_regions();
        g_world_ptr->ent_mgr.destroy_entity(this->m_web_entity);
        this->m_web_entity = nullptr;
    }
}

void recursive_web_position_helper(entity_base *a1, conglomerate *a2)
{
    auto *member = a2->get_member(a1->field_10, true);
    auto *v3 = member;
    if (member != nullptr) {
        entity_set_abs_po(a1, v3->get_abs_po());
    } else {
        entity_set_abs_position(a1, a2->get_abs_position());
    }

    for (auto *i = a1->get_first_child(); i != nullptr; i = i->field_28) {
        recursive_web_position_helper(i, a2);
    }
}

void web_info_nugget::frame_advance(Float a2)
{
    assert(this->m_web_entity != nullptr);

    auto *v3 = this->m_web_entity;
    if (v3->is_a_conglomerate() && this->my_actor->is_a_conglomerate()) {
        for (auto *i = v3->get_first_child(); i != nullptr; i = i->field_28) {
            recursive_web_position_helper(i, bit_cast<conglomerate *>(this->my_actor));
        }
    } else {
        v3->set_abs_po(po_identity_matrix);
    }

    this->m_web_entity->set_parent(this->my_actor);
    this->m_web_entity->set_visible(true, false);
    this->m_web_entity->compute_sector(g_world_ptr->the_terrain, false, nullptr);
    if (!this->field_1C && this->my_actor->field_88->field_18 == 1) {
        this->field_4 += a2;
        if (this->field_4 >= this->field_C) {
            this->field_10 = (this->field_4 - this->field_C) / this->field_8;
            auto col = this->m_web_entity->get_render_color();

            col.set_alpha(this->field_10 * 255.0);

            this->m_web_entity->set_render_color(col);
            this->field_1C = (this->field_10 >= 1.0f);
        }
    }
}

web_interface::web_interface(actor *a2) : field_0()
{
    this->my_actor = a2;
    this->field_1C = 0;
    this->field_18 = 0;
}

void *web_interface::operator new(size_t size)
{
    return mem_alloc(size);
}

void web_interface::operator delete(void *ptr, size_t size)
{
    mem_dealloc(ptr, size);
}

void web_interface::frame_advance(Float a2)
{
    if (this->field_18 != 0) {
        for (auto &nugget : this->field_0) {
            nugget->frame_advance(a2);
        }
    }
}

void web_interface::set_my_actor(actor *a2)
{
    this->my_actor = a2;
    assert(this->my_actor != nullptr);

    if (this->field_0.empty()) {
        assert(
            0 &&
            "No actor will be set because you don't have any web nuggets. Call this function AFTER adding web nuggets");
    }

    for (auto &nugget : this->field_0) {
        nugget->my_actor = a2;
    }
}

void web_interface::add_web_nugget(web_info_nugget *nugget)
{
    assert(nugget != nullptr);
    this->field_0.push_back(nugget);
}

void web_interface::insert_in_web_targets_list()
{
    web_interface::m_all_web_interfaces.push_back(this);
    if (this->is_flagged(static_cast<eWebInterfaceFlags>(4u))) {
        vhandle_type<actor> my_vhandle{this->my_actor->get_my_vhandle()};
        ai::player_web_target_inode::add_to_web_targets_list(my_vhandle);

        auto func = [](web_interface *self, uint16_t a2, bool a3) -> void {
            if (a3) {
                self->field_1C |= a2;
            } else {
                self->field_1C &= ~a2;
            }
        };

        func(this, 2u, true);
    }
}

void web_interface::frame_advance_all_web_interfaces(Float a1)
{
    TRACE("web_interface::frame_advance_all_web_interfaces");

    if constexpr (0) {
        for (auto &web_curr : m_all_web_interfaces) {
            assert(web_curr != nullptr);
            web_curr->frame_advance(a1);
        }
    } else {
        CDECL_CALL(0x004F2AC0, a1);
    }
}

void web_interface_patch()
{
    REDIRECT(0x0055850C, web_interface::frame_advance_all_web_interfaces);
}
