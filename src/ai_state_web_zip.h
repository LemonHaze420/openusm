#pragma once

#include "info_node.h"

#include "enhanced_state.h"

#include "float.hpp"
#include "line_info.h"
#include "spidermanlocoswingback.h"
#include "vector3d.h"

struct from_mash_in_place_constructor;
struct polytube;
struct mash_info_struct;

namespace ai {

struct mashed_state;
struct hero_inode;

struct web_zip_state : enhanced_state {
    vector3d field_30;
    hero_inode *field_3C;

    web_zip_state();
    ~web_zip_state();
    static void *native_vtable();
    void finalize(mash::allocation_scope scope);
    void get_info_node_list(info_node_desc_list &list);
    int get_mash_sizeof() const
    {
        return sizeof(web_zip_state);
    }
    uint32_t get_virtual_type_enum() const
    {
        return 327;
    }

    //0x0044C560
    web_zip_state(from_mash_in_place_constructor *a2);

    //0x0047DEF0
    //virtual
    state_trans_messages frame_advance(Float a2);

    //0x0045D340
    //virtual
    void activate(ai_state_machine *a2, const mashed_state *a3, const mashed_state *a4, const param_block *a5,
                  base_state::activate_flag_e a6);

    //0x0044C6E0
    //virtual
    void deactivate(const mashed_state *a1);

    static const inline string_hash default_id{static_cast<int>(to_hash("WEB_ZIP"))};
};

struct web_zip_inode : info_node {
    struct eZipReattachMode {
        int field_0;
    };

    line_info field_1C;
    int m_zip_type;
    int field_7C;
    bool field_80;
    int field_84[8];
    SpidermanLocoSwingBack field_A4[3];
    int field_D4;
    polytube *field_D8;
    hero_inode *field_DC;

    web_zip_inode();
    static void *native_vtable();

    //0x004815D0
    web_zip_inode(from_mash_in_place_constructor *a2);
    int get_mash_sizeof() const
    {
        return 0xE0;
    }
    void frame_advance(Float dt);

    //0x0044C930
    bool can_go_to(string_hash a2);

    //0x0046C280
    bool is_eligible(string_hash a2);

    //0x0045D600
    bool find_zip_anchor_from_crawl();

    //0x0044CBD0
    bool correct_zip_target_pos(line_info *si);

    //0x0045DFD0
    bool find_zip_anchor_and_transition_to_zip_jump(web_zip_inode::eZipReattachMode a2);

    //0x00478A80
    void process_zip(Float a2);

    //0x00481A00
    void add_swingback(polytube *&a2, entity_base *a3, actor *a4);
    void release_zip_web();

    //0x00474010
    //virtual
    void unmash(mash_info_struct *a2, void *a3);

    //0x0044C5A0
    /* virtual */ uint32_t get_virtual_type_enum();

    //0x00481A50
    //virtual
    void activate(ai_core *a2);

    //0x0044C880
    //virtual
    void deactivate();

    static inline string_hash default_id{to_hash("WEBZIP")};
};

}  // namespace ai

extern void web_zip_state_patch();
