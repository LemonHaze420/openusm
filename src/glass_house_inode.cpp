#include "glass_house_inode.h"

#include "common.h"
#include "actor.h"
#include "base_ai_core.h"
#include "fe_mission_text.h"
#include "femanager.h"
#include "glass_house_manager.h"
#include "igofrontend.h"
#include "func_wrapper.h"
#include "native_info_node_table.h"
#include "game.h"
#include "localized_string_table.h"
#include "sound_manager.h"
#include "sound_instance_id.h"

namespace ai {

VALIDATE_SIZE(glass_house_inode, 0x34);

namespace {
void __fastcall native_activate(glass_house_inode *self, void *, ai_core *core)
{
    self->activate(core);
}
void __fastcall native_deactivate(glass_house_inode *self, void *)
{
    self->deactivate();
}
void __fastcall native_advance(glass_house_inode *self, void *, Float dt)
{
    self->frame_advance(dt);
}
}  // namespace

void *glass_house_inode::native_vtable()
{
    static auto table = [] {
        native_inode::table<glass_house_inode, 383> result;
        result[6] = reinterpret_cast<void *>(&native_inode::always_advance);
        result[7] = reinterpret_cast<void *>(&native_advance);
        result[8] = reinterpret_cast<void *>(&native_activate);
        result[9] = reinterpret_cast<void *>(&native_deactivate);
        return result;
    }();
    return table.data();
}

glass_house_inode::glass_house_inode() : info_node(), field_20(false), field_21(false)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[383]);
}

glass_house_inode::glass_house_inode(from_mash_in_place_constructor *tag) : info_node(tag), field_24(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[383]);
}

void glass_house_inode::unmash(mash_info_struct *info, void *data)
{
    info_node::_unmash(info, data);
}

void glass_house_inode::activate(ai_core *core)
{
    info_node::_activate(core);
    field_30 = 0.0f;
    field_1C = core->field_64;
}

void glass_house_inode::frame_advance(Float dt)
{
    const vector3d position = field_1C->get_abs_position();
    field_21 = glass_house_manager::is_point_in_glass_house(position);
    field_20 = !field_21;
    if (field_21) {
        field_24 = position;
    }
    if (field_30 > 0.0f) {
        field_30 -= dt;
        if (field_30 < 0.0f) {
            field_30 = 0.0f;
            g_femanager.IGO->m_fe_mission_text->SetShown(false);
        }
    }
}

void glass_house_inode::deactivate()
{
    if (field_30 > 0.0f) {
        g_femanager.IGO->m_fe_mission_text->SetShown(false);
    }
}

void glass_house_inode::show_glass_house_message()
{
    field_30 = 2.0f;
    mString text{g_game_ptr->field_7C->lookup_localized_string(static_cast<global_text_enum>(102))};
    auto *mission_text = g_femanager.IGO->m_fe_mission_text;
    mission_text->set_text(*reinterpret_cast<fe_mission_text::string *>(&text));
    mission_text->SetShown(true);
    static const string_hash sound_hash{"FE_Glass_Wall_Fail_1"};
    auto sound = sound_manager::create_sound_instance(0, sound_hash);
    if (auto *instance = sound.get_sound_instance_ptr()) {
        instance->play();
    }
}

}  // namespace ai
