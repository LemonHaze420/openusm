#include "debug_menu.h"

#include "common.h"
#include "ngl.h"
#include "ngl_font.h"
#include "game.h"
#include "input_mgr.h"
#include "memory.h"
#include "os_developer_options.h"
#include "spider_monkey.h"
#include "string_hash.h"
#include "script_memtrack.h"
#include "script_object.h"
#include "mission_manager.h"
#include "mission_table_container.h"
#include "resource_manager.h"
#include "region.h"
#include "vm_executable.h"
#include "debug_menu_extra.h"

#include <cassert>
#include <vector>
#include <tuple>

VALIDATE_SIZE(debug_menu, 0x20);

VALIDATE_SIZE(debug_menu_entry, 0x50);

debug_menu *debug_menu::active_menu = nullptr;

debug_menu *debug_menu::root_menu = nullptr;

debug_menu_entry *g_debug_camera_entry = nullptr;

fixed_pool debug_menu::pool = fixed_pool{28, 16, 4, 1, 0, nullptr};
fixed_pool debug_menu_entry::pool = fixed_pool{76, 32, 4, 1, 0, nullptr};

debug_menu::debug_menu(const char *a1, debug_menu::sort_mode_t sort_mode) {
    this->field_C = mString{a1};
    this->m_sort_mode = sort_mode;
    this->first = nullptr;
    this->last = nullptr;
    this->highlighted = nullptr;
}

debug_menu::debug_menu(const mString &a1, debug_menu::sort_mode_t sort_mode)
{
    this->field_C = a1;
    this->m_sort_mode = sort_mode;
    this->first = nullptr;
    this->last = nullptr;
    this->highlighted = nullptr;
}

debug_menu::~debug_menu()
{
    while ( this->first != nullptr)
    {
        this->remove_entry(this->first);
    }
}

void * debug_menu::operator new(size_t )
{
    return debug_menu::pool.allocate_new_block();
}

void debug_menu::operator delete(void *) {}

void debug_menu::render_active()
{
    if ( debug_menu::active_menu != nullptr )
    {
        constexpr float menux = 25.0f;
        constexpr float menuy = 45.0f;
        active_menu->render(menux, menuy);
    }

}

void debug_menu::frame_advance(Float a1)
{
    had_menu_this_frame = false;
    gather_input(a1);
    if ( active_menu == nullptr || has_focus )
    {
        if ( virtual_input_state[8] )
        {
            if ( active_menu != nullptr )
            {
                hide();
            }
            else if ( spider_monkey::is_running() ||
                    !os_developer_options::instance->get_flag("ENABLE_ZOOM_MAP") ||
                    g_game_ptr->is_physics_enabled() )
            {
                show();
            }
        }

        if ( active_menu != nullptr )
        {
            had_menu_this_frame = true;
            active_menu->do_frame_advance(a1);
        }
    }
    else if ( debug_menu::virtual_input_state[9] )
    {
        had_menu_this_frame = true;
        grab_focus();
    }
}

void debug_menu::gather_input(Float a1)
{
    auto *v3 = input_mgr::instance;
    memcpy(previous_input_state, current_input_state, 10);
    memset(current_input_state, 0, sizeof(current_input_state));
    memset(virtual_input_state, 0, sizeof(virtual_input_state));

    if ( v3->get_control_state(69, INVALID_DEVICE_ID) > 0.5 )
    {
        current_input_state[1] = 1;
    }

    if ( v3->get_control_state(68, INVALID_DEVICE_ID) > 0.5 )
    {
        current_input_state[0] = 1;
    }

    if ( v3->get_control_state(76, INVALID_DEVICE_ID) > 0.5 ) {
        current_input_state[6] = 1;
    }
    
    if ( v3->get_control_state(77, INVALID_DEVICE_ID) > 0.5 ) {
        current_input_state[7] = 1;
    }

    if ( v3->get_control_state(74, INVALID_DEVICE_ID) > 0.5 ) {
        current_input_state[2] = 1;
    }

    if ( v3->get_control_state(75, INVALID_DEVICE_ID) > 0.5 ) {
        current_input_state[3] = 1;
    }

    if ( v3->get_control_state(78, INVALID_DEVICE_ID) > 0.5 ) {
        current_input_state[4] = 1;
    }

    if ( v3->get_control_state(79, INVALID_DEVICE_ID) > 0.5 ) {
        current_input_state[5] = 1;
    }

    if ( v3->get_control_state(73, INVALID_DEVICE_ID) > 0.5 ) {
        current_input_state[8] = 1;
    }

    if ( v3->get_control_state(72, INVALID_DEVICE_ID) > 0.5 ) {
        current_input_state[9] = 1;
    }

    if ( v3->get_control_state(70, INVALID_DEVICE_ID) > 0.5 ) {
        current_input_state[3] = 1;
    }

    if ( v3->get_control_state(70, INVALID_DEVICE_ID) < -0.5 ) {
        current_input_state[2] = 1;
    }

    if ( v3->get_control_state(71, INVALID_DEVICE_ID) > 0.5 ) {
        current_input_state[7] = 1;
    }

    if ( v3->get_control_state(71, INVALID_DEVICE_ID) < -0.5 ) {
        current_input_state[6] = 1;
    }

    for (auto i = 0; i < 10; ++i )
    {
        if ( current_input_state[i] == previous_input_state[i] )
        {
            input_state_timer[i] = input_state_timer[i] + a1;
            auto v1 = 0.25f;
            if ( virtual_input_repeating[i] )
            {
                v1 = 0.050000001f;
            }

            if ( input_state_timer[i] > v1 )
            {
                input_state_timer[i] = input_state_timer[i] - v1;
                virtual_input_state[i] = current_input_state[i];
                virtual_input_repeating[i] = true;
            }
        }
        else
        {
            input_state_timer[i] = 0.0;
            virtual_input_repeating[i] = false;
            virtual_input_state[i] = current_input_state[i];
        }
    }
}

debug_menu *debug_menu::find_submenu_parent(debug_menu *a2)
{
    for ( auto *i = this->first; i != nullptr; i = i->next)
    {
        if ( i->value_type == ValueType::POINTER_MENU )
        {
            if ( i->m_value.p_menu == a2 )
            {
                return this;
            }

            if ( i->m_value.p_menu != nullptr )
            {
                auto *v3 = i->m_value.p_menu->find_submenu_parent(a2);
                if ( v3 != nullptr )
                {
                    return v3;
                }
            }
        }
    }

    return nullptr;
}

void debug_menu::activate_parent()
{
    auto *v1 = root_menu->find_submenu_parent(this);
    if ( v1 != nullptr )
    {
        active_menu = v1;
    }
}

void debug_menu::do_frame_advance([[maybe_unused]] Float a2)
{
    if ( virtual_input_state[1] )
    {
        this->activate_parent();
    }

    if ( this->first != nullptr )
    {
        if ( this->highlighted == nullptr )
        {
            this->highlighted = this->first;
        }

        if ( virtual_input_state[3] )
        {
            this->highlighted = this->highlighted->next;
            if ( this->highlighted == nullptr )
            {
                this->highlighted = this->first;
            }
        }

        if ( virtual_input_state[2] )
        {
            this->highlighted = this->highlighted->prev;
            if ( this->highlighted == nullptr )
            {
                this->highlighted = this->last;
            }
        }

        if ( virtual_input_state[4] )
        {
            for ( auto i = 0; i < 20 && this->highlighted != nullptr; ++i )
            {
                this->highlighted = this->highlighted->prev;
            }
            
            if ( this->highlighted == nullptr )
            {
                this->highlighted = this->first;
            }
        }
        
        if ( virtual_input_state[5] )
        {
            for ( auto j = 0; j < 20 && this->highlighted != nullptr; ++j )
            {
                this->highlighted = this->highlighted->next;
            }

            if ( this->highlighted != nullptr )
            {
                this->highlighted = this->last;
            }
        }

        if ( virtual_input_state[7] )
        {
            this->highlighted->on_change(1.0, virtual_input_state[0]);
        }

        if ( virtual_input_state[6] )
        {
            this->highlighted->on_change(-1.0, virtual_input_state[0]);
        }

        if ( virtual_input_state[0] )
        {
            this->highlighted->on_select(1.0);
        }

        if ( debug_menu::virtual_input_state[9] )
        {
            release_focus();
        }

        assert(highlighted->frame_advance_callback != nullptr);
        this->highlighted->frame_advance_callback(this->highlighted);
    }
}

void debug_menu::render(int x, int y)
{
    nglQuad a1{};
    nglInitQuad(&a1);
    auto a5 = (float)(unsigned int)(y + menu_height + 10);
    auto a4 = (float)(unsigned int)(x + menu_width + 10);
    nglSetQuadRect(&a1, (float)(x - 5), (float)(y - 5), a4, a5);
    nglSetQuadColor(&a1, has_focus ? 0xC8141414 : 0x64141414);
    nglSetQuadZ(&a1, 0.5);
    nglListAddQuad(&a1);
    menu_width = 0;
    menu_height = 0;
    auto *v3 = this->field_C.c_str();

    uint32_t width, height;
    nglGetStringDimensions(nglSysFont(), &width, &height, v3);
    menu_width = width;
    menu_height += height;

    nglListAddString(nglSysFont(), (float)x, (float)y, 0.0, 0xFFFFFF00, v3);
    auto ya = height + y;
    int v18 = 0;
    int v17 = 0;
    for ( auto *i = this->first; i != nullptr; i = i->next)
    {
        if ( i == this->highlighted)
        {
            v17 = v18;
        }

        ++v18;
    }

    int v15 = 0;
    bool v14 = false;
    bool v13 = false;
    if ( v18 > 20 )
    {
        if ( v17 >= 10 )
        {
            if ( v17 < v18 - 10 )
            {
                v15 = v17 - 10;
                v14 = true;
                v13 = true;
            }
            else
            {
                v15 = v18 - 20;
                v14 = true;
            }
        }
        else
        {
            v15 = 0;
            v13 = true;
        }
    }

    int v12 = 0;
    int v10 = 0;
    for ( auto *entry = this->first; entry != nullptr; entry = entry->next)
    {
        if ( v12 >= v15 && v10 < 20 )
        {
            uint32_t arg4{};
            if ( v10 != 0 || !v14 )
            {
                if ( v10 == 19 && v13 )
                {
                    nglListAddString(nglSysFont(), (float)x, (float)ya, 0.0, 0xFFFF00FF, " v v v ");
                    nglGetStringDimensions(nglSysFont(), &arg4, nullptr, " v v v ");
                }
                else if ( entry == this->highlighted && has_focus )
                {
                    arg4 = entry->render(x, ya, true);
                }
                else
                {
                    arg4 = entry->render(x, ya, false);
                }
            }
            else
            {
                nglListAddString(nglSysFont(), (float)x, (float)ya, 0.0, 0xFFFF00FF, " ^ ^ ^ ");
                nglGetStringDimensions(nglSysFont(), &arg4, nullptr, " ^ ^ ^ ");
            }

            ya += height;
            debug_menu::menu_height += height;
            if ( static_cast<int>(arg4) > debug_menu::menu_width )
            {
                debug_menu::menu_width = arg4;
            }
            
            ++v10;
        }
        
        ++v12;
    }
}

mString entry_render_callback_default(debug_menu_entry *a2) {
    mString out;
    if ( a2 != nullptr )
    {
        switch ( a2->get_value_type() )
        {
        case ValueType::FLOAT:
        case ValueType::POINTER_FLOAT:
        {
            auto v2 = a2->get_fval();
            char a1[64];
            snprintf(a1, 64, "%.2f", v2);
            out = {a1};
            break;
        }
        case ValueType::BOOL:
        case ValueType::POINTER_BOOL:
        {
            out = (a2->get_bval() ? mString{"True"} : mString{"False"});
            break;
        }
        case ValueType::INT:
        case ValueType::POINTER_INT:
        {
            auto v4 = a2->get_ival();
            out = mString {v4};
            break;
        }
        case ValueType::POINTER_MENU:
        {
            out = mString {">"};
            break;
        }
        default:
            out = mString {};
            break;
        }
    }
    else
    {
        out = mString{};
    }

    return out; 
}

void entry_frame_advance_callback_default(debug_menu_entry *) {
    ;
}

debug_menu_entry::debug_menu_entry(const mString &a1) : field_20{0, 1.0, 0.1, 10.0}
{
    this->m_name = a1;
    this->value_type = ValueType::UNDEFINED;
    this->m_value.ival = 0;
    this->m_game_flags_handler = nullptr;
    this->render_callback = entry_render_callback_default;
    this->frame_advance_callback = entry_frame_advance_callback_default;
    this->field_C = nullptr;
    this->field_14 = nullptr;
    this->next = nullptr;
    this->m_data = nullptr;
    this->m_value_initialized = false;
}

debug_menu_entry::debug_menu_entry(debug_menu *submenu) :
                        field_20{0.0f, 1.0f, 0.1f, 10.0f}
{
    assert(submenu != nullptr);
    auto &v2 = submenu->sub_B7E660();
    this->m_name = v2;
    this->value_type = ValueType::POINTER_MENU;
    this->m_value.p_menu = submenu;
    this->m_game_flags_handler = nullptr; 
    this->render_callback = entry_render_callback_default;
    this->frame_advance_callback = entry_frame_advance_callback_default;
    this->field_C = nullptr;
    this->field_14 = nullptr;
    this->next = nullptr;
    this->m_data = nullptr;
    this->m_value_initialized = false;
}

debug_menu_entry::~debug_menu_entry()
{
    if ( this->value_type == ValueType::POINTER_MENU && this->m_value.p_menu != nullptr )
    {
        this->m_value.p_menu->~debug_menu();
        this->m_value.p_menu = nullptr;
    }
}

int debug_menu_entry::render(int a1, int a2, bool a3)
{
    auto a5 = a3 ? 0xFF00DC00 : 0xFFDCDCDC;

    assert(render_callback != nullptr);

    mString v10 = this->render_callback(this);

    char a2a[256]{};
    auto v14 = v10;
    if ( v14.length() )
    {
        auto *v7 = v14.c_str();
        auto *v4 = this->m_name.c_str();
        snprintf(a2a, 255u, "%s: %s", v4, v7);
    }
    else
    {
        auto *v5 = this->m_name.c_str();
        snprintf(a2a, 255u, "%s", v5);
    }

    uint32_t arg4a, a3a;
    nglGetStringDimensions(nglSysFont(), &arg4a, &a3a, a2a);
    nglListAddString(nglSysFont(), (float)a1, (float)a2, 0.0, a5, a2a);
    auto v11 = arg4a;
    return v11;
}

void script_handler_helper(debug_menu_entry *a2)
{
    if ( a2->field_18 >= 0 && a2->field_14 != nullptr )
    {
        auto *v2 = a2->field_14->get_parent();
        auto *exe = v2->get_func(a2->field_18);

        assert(exe != nullptr);

        if ( exe->get_parms_stacksize() == 4 ) {
            a2->field_14->add_thread(exe, (const char *)&a2);
        } else {
            a2->field_14->add_thread(exe);
        }

        debug_menu::hide();
    }
}

struct mission_t {
    mString field_0;
    const char *field_C;
    int m_district_id;
    int field_14;
};

std::vector<mission_t> menu_missions;

void mission_unload_handler(debug_menu_entry *)
{
    auto *v1 = mission_manager::s_inst;
    v1->prepare_unload_script();
    debug_menu::hide();
}

void mission_select_handler(debug_menu_entry *entry)
{
    auto v1 = entry->m_id;
    const auto &v7 = menu_missions[v1];
    auto v6 = v7.field_C;
    auto v5 = v7.field_14;
    auto *v4 = v7.field_0.c_str();
    auto v3 = v7.m_district_id;
    auto *v2 = mission_manager::s_inst;
    v2->force_mission(v3, v4, v5, v6);
    debug_menu::hide();
}

auto create_menu(const mString &str, debug_menu::sort_mode_t sort_mode) -> debug_menu * {
    auto *entry = new debug_menu{str, sort_mode};
    return entry;
}

auto create_menu(const char *str, debug_menu::sort_mode_t sort_mode) -> debug_menu *
{
    auto *v25 = new debug_menu{str, sort_mode};
    return v25;
}

auto create_menu_entry(const mString &str) -> debug_menu_entry *
{
    auto *entry = new debug_menu_entry{str};
    return entry;
}

auto create_menu_entry(debug_menu *menu) -> debug_menu_entry *
{
    auto *entry = new debug_menu_entry{menu};
    return entry;
}

void populate_missions_menu(debug_menu_entry *entry)
{
    menu_missions = {};
    if ( resource_manager::can_reload_amalgapak() )
    {
        resource_manager::reload_amalgapak();
    }

    auto *head_menu = create_menu(entry->get_name(), debug_menu::sort_mode_t::ascending);
    entry->set_submenu(head_menu);

    auto *mission_unload_entry = create_menu_entry(mString{"UNLOAD CURRENT MISSION"});

    mission_unload_entry->set_game_flags_handler(mission_unload_handler);
    head_menu->add_entry(mission_unload_entry);

    auto *v2 = mission_manager::s_inst;
    auto v58 = v2->get_district_table_count();
    for ( auto i = -1; i < v58; ++i )
    {
        fixedstring<8> v53 {};
        int district_id;
        mission_table_container *table = nullptr;
        if ( i == -1 )
        {
            table = v2->get_global_table();
            v53 = fixedstring<8> {"global"};
            district_id = 0;
        }
        else
        {
            table = v2->get_district_table(i);
            auto *reg = table->get_region();
            v53 = reg->get_name();

            district_id = reg->get_district_id();

            auto *v25 = create_menu(v53.to_string(), debug_menu::sort_mode_t::ascending);

            auto *v26 = create_menu_entry(v25);

            head_menu->add_entry(v26);
        }

        _std::vector<mission_table_container::script_info> script_infos;

        if ( table != nullptr )
        {
            auto res = table->append_script_info(&script_infos);
            assert(res);
        }

        for ( auto &info : script_infos)
        {
            auto v50 = menu_missions.size();
            const auto v19 = mString {"pk_"} + info.field_0;
            auto *v11 = v19.c_str();
            auto key = create_resource_key_from_path(v11, RESOURCE_KEY_TYPE_PACK);
            if ( resource_manager::get_pack_file_stats(key, nullptr, nullptr, nullptr) )
            {
                mission_t mission {};
                mission.field_0 = info.field_0;
                mission.m_district_id = district_id;
                mission.field_14 = info.field_8;

                mission.field_C = info.field_4->get_script_data_name();
                menu_missions.push_back(mission);

                mString v47{};
                if ( mission.field_C != nullptr )
                {
                    auto *v17 = mission.field_C;
                    auto *v14 = mission.field_0.c_str();
                    v47 = mString {0, "%s (%s)", v14, v17};
                }
                else
                {
                    auto v18 = mission.field_14;
                    auto *v15 = mission.field_0.c_str();
                    v47 = mString {0, "%s (%d)", v15, v18};
                }

                auto *v46 = create_menu_entry(v47);
                v46->set_id(v50);
                v46->set_game_flags_handler(mission_select_handler);
                head_menu->add_entry(v46);
            }
        }
    }
}

void _populate_missions()
{
    auto handle_table = [](mission_table_container *table, int district_id) -> void
    {

        _std::vector<mission_table_container::script_info> script_infos{};

        if ( table != nullptr )
        {
            table->append_script_info(&script_infos);
        }

        for ( auto &info : script_infos )
        {
            mString a2{"pk_"};
            auto v19 = a2 + info.field_0;
            auto *v11 = v19.c_str();
            auto key = create_resource_key_from_path(v11, RESOURCE_KEY_TYPE_PACK);
            if ( resource_manager::get_pack_file_stats(key, nullptr, nullptr, nullptr) )
            {
                mission_t mission{};
                mission.field_0 = info.field_0;
                mission.m_district_id = district_id;
                mission.field_14 = info.field_8;

                mission.field_C = info.field_4->get_script_data_name();
                menu_missions.push_back(mission);

                auto v47 = [](mission_t &mission) -> mString {
                    if ( mission.field_C != nullptr )
                    {
                        auto *v17 = mission.field_C;
                        auto *v14 = mission.field_0.c_str();
                        mString str{0, "%s (%s)", v14, v17};
                        return str;
                    }

                    auto v18 = mission.field_14;
                    auto *v15 = mission.field_0.c_str();
                    mString str{0, "%s (%d)", v15, v18};
                    return str;

                }(mission);

                sp_log(v47.c_str());
            }
        }
    };

    auto *v2 = mission_manager::s_inst;
    auto count = v2->get_district_table_count();
    sp_log("%s %d", "table_count = ", count);

    {
        auto *v3 = mission_manager::s_inst;
        auto *table = v3->get_global_table();

        handle_table(table, 0);
    }


    std::for_each(&v2->m_district_table_containers[0], &v2->m_district_table_containers[0] + count, [&handle_table](auto *table) {

        auto *reg = table->get_region();
        auto &v6 = reg->get_name();
        fixedstring<8> v53{v6.to_string()};

        auto district_id = reg->get_district_id();

        //sp_log("%d %s", i, v53.to_string());

        handle_table(table, district_id);
    });

    assert(0);
}

void * debug_menu_entry::operator new(size_t )
{
    return debug_menu_entry::pool.allocate_new_block();
}

void debug_menu_entry::set_submenu(debug_menu *submenu)
{
    if ( submenu != nullptr )
    {
        this->m_name = submenu->field_C;
    }

    if ( this->value_type == ValueType::POINTER_MENU && this->m_value.p_menu != nullptr )
    {
        delete this->m_value.p_menu;
    }

    this->value_type = ValueType::POINTER_MENU;
    this->m_value.p_menu = submenu;
}

bool debug_menu_entry::set_script_handler(script_instance *inst, const mString &a3)
{
    assert(inst != nullptr);

    auto *v3 = a3.c_str();
    string_hash v8{v3};

    auto *so= inst->get_parent();
    int func = so->find_func(v8);

    bool result;
    if ( func >= 0 )
    {
        this->field_14 = inst;
        this->field_18 = func;
        this->m_game_flags_handler = script_handler_helper;
        result = true;
    }
    else
    {
        auto *v6 = a3.c_str();
        sp_log("Could not find handler: %s", v6);
        result = false;
    }

    return result;
}

void debug_menu_entry::set_render_cb(mString (*a2)(debug_menu_entry *) ) {
    this->render_callback = a2;
}

void debug_menu_entry::set_id(short a2) {
    this->m_id = a2;
}

void debug_menu_entry::set_game_flags_handler(void (*a2)(debug_menu_entry *)) {
    this->m_game_flags_handler = a2;
}

bool debug_menu_entry::is_value_initialized() const
{
    return this->m_value_initialized;
}

int debug_menu_entry::set_ival(int a2, bool a3)
{
    if ( !this->is_value_initialized() )
    {
        if ( a2 > this->field_20.m_max_value ) {
            a2 = this->field_20.m_max_value;
        }

        if ( this->field_20.m_min_value > a2 ) {
            a2 = this->field_20.m_min_value;
        }

        auto v4 = this->value_type;
        if ( v4 == ValueType::INT )
        {
            this->m_value.ival = a2;
        }
        else if ( v4 == ValueType::POINTER_INT )
        {
            *this->m_value.p_ival = a2;
        }
        else
        {
            assert(0);
        }

        if ( this->m_game_flags_handler != nullptr && a3 )
        {
            this->m_game_flags_handler(this);
        }
    }

    return this->get_ival();
}

void debug_menu_entry::set_ival(int a2)
{
    this->value_type = ValueType::INT;
    this->m_value.ival = a2;
}

void debug_menu_entry::set_p_ival(int *a2)
{
    this->value_type = ValueType::POINTER_INT;
    this->m_value.p_ival = a2;
}

void debug_menu_entry::set_fl_values(const float *a2)
{
    auto &v2 = this->field_20;
    v2.m_min_value = a2[0];
    v2.m_max_value = a2[1];
    v2.m_step_size = a2[2];
    v2.m_step_scale = a2[3];
}

void debug_menu_entry::set_pt_bval(bool *a2)
{
    this->value_type = ValueType::POINTER_BOOL;
    this->m_value.p_bval = a2;
}

void debug_menu_entry::set_pt_fval(float *a2)
{
    this->value_type = ValueType::POINTER_FLOAT;
    this->m_value.p_fval = a2;
}

void debug_menu_entry::set_min_value(float a2)
{
    this->field_20.m_min_value = a2;
}

void debug_menu_entry::set_max_value(float a2)
{
    this->field_20.m_max_value = a2;
}


bool debug_menu_entry::set_bval(bool a2, bool a3)
{
    if ( !this->is_value_initialized() )
    {
        auto v4 = this->value_type;
        if ( v4 == ValueType::BOOL )
        {
            this->m_value.bval = a2;
        }
        else if ( v4 == ValueType::POINTER_BOOL )
        {
            *this->m_value.p_bval = a2;
        }
        else
        {
            assert(0);
        }

        if ( this->m_game_flags_handler != nullptr && a3 )
        {
            this->m_game_flags_handler(this);
        }
    }

    return this->get_bval();
}

void debug_menu_entry::set_bval(bool a2)
{
    this->value_type = ValueType::BOOL;
    this->m_value.bval = a2;
}

void debug_menu_entry::set_val(Float a1, bool a2)
{
    if ( !this->is_value_initialized() )
    {
        if ( this->value_type != ValueType::BOOL && this->value_type != ValueType::POINTER_BOOL )
        {
            if ( a1 > this->field_20.m_max_value ) {
                a1 = this->field_20.m_max_value;
            }

            if ( this->field_20.m_min_value > a1 ) {
                a1 = this->field_20.m_min_value;
            }
        }

        switch ( this->value_type )
        {
        case ValueType::FLOAT:
            this->m_value.fval= a1;
            break;
        case ValueType::POINTER_FLOAT:
            *this->m_value.p_fval = a1;
            break;
        case ValueType::BOOL:
            this->m_value.bval = not_equal<float>(a1, 0.0f);
            break;
        case ValueType::POINTER_BOOL:
            *this->m_value.p_bval = not_equal<float>(a1, 0.0f);
            break;
        case ValueType::INT:
            this->m_value.ival = (int)a1;
            break;
        case ValueType::POINTER_INT:
            *this->m_value.p_ival = (int)a1;
            break;
        default:
            assert(0);            
            break;
        }

        if ( this->m_game_flags_handler != nullptr && a2 )
        {
          this->m_game_flags_handler(this);
        }
    }

    this->get_val();
}

void debug_menu_entry::set_fval(float a2, bool a3)
{
    if ( !this->is_value_initialized() )
    {
        if ( a2 > this->field_20.m_max_value ) {
            a2 = this->field_20.m_max_value;
        }

        if ( this->field_20.m_min_value > a2 ) {
            a2 = this->field_20.m_min_value;
        }

        auto v3 = this->value_type;
        if ( v3 == ValueType::FLOAT )
        {
            this->m_value.fval = a2;
        }
        else if ( v3 == ValueType::POINTER_FLOAT )
        {
            *this->m_value.p_fval = a2;
        }
        else
        {
            assert(0);
        }

        if ( this->m_game_flags_handler != nullptr && a3 )
        {
            this->m_game_flags_handler(this);
        }
    }

    this->get_fval();
}

double debug_menu_entry::get_val()
{
    double result;

    switch ( this->value_type )
    {
    case ValueType::FLOAT:
        result = this->m_value.fval;
        break;
    case ValueType::POINTER_FLOAT:
        result = *this->m_value.p_fval;
        break;
    case ValueType::BOOL:
        result = (double) this->m_value.bval;
        break;
    case ValueType::POINTER_BOOL:
        result = (double) *this->m_value.p_bval;
        break;
    case ValueType::INT:
        result = (double) this->m_value.ival;
        break;
    case ValueType::POINTER_INT:
        result = (double) *this->m_value.p_ival;
        break;
    default:
        assert(0);
        result = 0.0;
        break;
    }

    return result;
}

double debug_menu_entry::get_fval() {
    auto v2 = this->value_type;
    if (v2 == ValueType::FLOAT) {
        return this->m_value.fval;
    }

    if (v2 == ValueType::POINTER_FLOAT) {
        return *this->m_value.p_fval;
    }

    assert(0);
    return 0.0;
}

bool debug_menu_entry::get_bval() {
    auto v2 = this->value_type;
    if (v2 == ValueType::BOOL) {
        return this->m_value.bval;
    }

    if (v2 == ValueType::POINTER_BOOL) {
        return *this->m_value.p_bval;
    }

    assert(0);
    return 0;
}

int debug_menu_entry::get_ival() {
    auto v2 = this->value_type;
    if (v2 == ValueType::INT) {
        return this->m_value.ival;
    }

    if (v2 == ValueType::POINTER_INT) {
        return *this->m_value.p_ival;
    }

    assert(0);
    return 0;
}

const mString &debug_menu_entry::get_name() const {
    return this->m_name;
}

void debug_menu_entry::on_select(float a2)
{
    switch ( this->value_type )
    {
    case ValueType::UNDEFINED:
        if ( this->m_game_flags_handler != nullptr )
        {
            this->m_game_flags_handler(this);
        }

        break;
    case ValueType::BOOL:
    case ValueType::POINTER_BOOL:
        this->on_change(a2, false);
        break;
    case ValueType::POINTER_MENU:
        this->remove_menu();
        if ( this->m_value.p_menu != nullptr )
        {
            debug_menu::active_menu = this->m_value.p_menu;
        }

        break;
    default:
        return;
    }
}

debug_menu *debug_menu_entry::remove_menu()
{
    if ( this->m_game_flags_handler != nullptr )
    {
        if ( this->m_value.p_menu != nullptr )
        {
            this->m_value.p_menu->~debug_menu();
        }

        this->m_value.p_menu = nullptr;
        this->m_game_flags_handler(this);
    }

    return this->m_value.p_menu;
}

void debug_menu_entry::on_change(float a3, bool a4)
{
    switch ( this->value_type )
    {
    case ValueType::FLOAT:
    case ValueType::POINTER_FLOAT:
    {
        float v6 = (a4
                    ? (this->field_20.m_step_size * this->field_20.m_step_scale)
                    : this->field_20.m_step_size
                    );

        auto v5 = this->get_fval() + a3 * v6;
        this->set_fval(v5, true);
        break;
    }
    case ValueType::BOOL:
    case ValueType::POINTER_BOOL:
    {
        auto v3 = this->get_bval();
        this->set_bval(!v3, true);
        break;
    }
    case ValueType::INT:
    case ValueType::POINTER_INT:
    {
        float v7 = (a4
                    ? (this->field_20.m_step_size * this->field_20.m_step_scale)
                    : this->field_20.m_step_size
                    );

        auto v8 = std::abs(v7);
        if ( v8 < 1.0 )
        {
            v8 = 1.0;
        }

        auto v4 = this->get_ival();
        if ( a3 >= 0.0 )
            this->set_ival((int)((float)v4 + v8), true);
        else
            this->set_ival((int)((float)v4 - v8), true);

        break;
    }
    default:
      return;
    }
}

bool sub_67A388(const mString &a1, const mString &a2) {
    auto *v2 = a2.c_str();
    return a1.compare(v2) == 1;
}

bool sub_672408(const mString &a1, const mString &a2) {
    auto *v2 = a2.c_str();
    return a1.compare(v2) == -1;
}

void debug_menu::add_entry(debug_menu *a2)
{
    auto *v2 = create_menu_entry(a2);
    this->add_entry(v2);
}

void debug_menu::add_entry(debug_menu_entry *e) {
    assert(e != nullptr);

    if (this->first != nullptr) {
        auto *v12 = this->first;
        debug_menu_entry *v11 = nullptr;
        for (; v12 != nullptr && v12->value_type == ValueType::POINTER_MENU;
            v11 = v12, v12 = v12->next) {
            if (this->m_sort_mode != sort_mode_t::undefined)
            {
                auto v7 = e->get_name();
                auto v2 = v12->get_name();
                if ((this->m_sort_mode == sort_mode_t::ascending && v2 > v7) ||
                        (this->m_sort_mode == sort_mode_t::descending && v2 < v7) )
                {
                    continue;
                }
                else
                {
                   break;
                }

            }
        }

        if (e->value_type != ValueType::POINTER_MENU) {
            for (; v12 != nullptr;
                    v11 = v12, v12 = v12->next) {
                if (this->m_sort_mode != sort_mode_t::undefined) {
                    mString v9;
                    mString v4;

                    if (this->m_sort_mode != sort_mode_t::ascending ||
                        (v9 = e->get_name(),
                         v4 = v12->get_name(),
                         !(v4 > v9) ))
                    {
                        if (this->m_sort_mode != sort_mode_t::descending) {
                            break;
                        }

                        auto v10 = e->get_name();
                        auto v5 = v12->get_name();
                        if ( v5 < v10 ) {
                        }
                        else
                        {
                            break;
                        }
                    }
                }

            }
        }

        e->prev = v11;
        e->next = v12;
        if (v11 == nullptr) {
            this->first = e;
        }

        if (v12 == nullptr) {
            this->last = e;
        }

        if (v11 != nullptr) {
            v11->next = e;
        }

        if (v12 != nullptr) {
            v12->prev = e;
        }
    } else {
        assert(this->last == nullptr);

        this->last = e;
        this->first = e;
        e->next = nullptr;
        e->prev = nullptr;
    }
}

void debug_menu::remove_entry(debug_menu_entry *e)
{
    assert(e != nullptr);

    if ( e == this->highlighted)
    {
        this->highlighted = nullptr;
    }

    if ( e->prev != nullptr )
    {
        e->prev->next = e->next;
    }

    if ( e->next != nullptr)
    {
        e->next->prev = e->prev;
    }

    if ( e == this->first)
    {
        this->first = this->first->next;
    }

    if ( e == this->last )
    {
        this->last = this->last->prev;
    }
    
    if ( e->field_C != nullptr )
    {
        e->field_C(e, e->m_data);
    }

    if ( e != nullptr )
    {
        e->~debug_menu_entry();
    }
}

int g_mem_checkpoint_debug_0 {-1};

void set_memtrack_checkpoint(debug_menu_entry *)
{
    g_mem_checkpoint_debug_0 = mem_set_checkpoint();
    debug_menu::hide();
}

void dump_memtrack_data(debug_menu_entry *)
{
    mem_check_leaks_since_checkpoint(g_mem_checkpoint_debug_0, 1u);
    mem_print_stats("\nMemory log\n");
    debug_menu::hide();
}

void create_memory_menu(debug_menu *parent)
{
    auto *memory_menu = create_menu("Memory", debug_menu::sort_mode_t::undefined);
    
    auto *v2 = create_menu_entry(memory_menu);
    parent->add_entry(v2);
    script_memtrack::create_debug_menu(memory_menu);

    auto *entry = create_menu_entry(mString {"Dump MemTrack Data Since Last Checkpoint"});
    entry->set_game_flags_handler(dump_memtrack_data);
    memory_menu->add_entry(entry);

    entry = create_menu_entry(mString {"Set MemTrack Checkpoint"});
    entry->set_game_flags_handler(set_memtrack_checkpoint);
    memory_menu->add_entry(entry);
    
    slab_allocator::create_slab_debug_menu(memory_menu);
}

void debug_menu::init()
{
    assert(root_menu == nullptr);

    root_menu = new debug_menu{"Debug Menu", sort_mode_t::undefined};
    active_menu = nullptr;
    create_camera_menu_items(root_menu);

    //create_dvar_menu(debug_menu::root_menu);
    create_warp_menu(debug_menu::root_menu);

    create_game_flags_menu(debug_menu::root_menu);

    /*
    j_create_missions_menu(debug_menu::root_menu);
    */
    
    create_debug_render_menu(debug_menu::root_menu);

    /*
    sub_6A757C(debug_menu::root_menu);
    sub_6991B1(debug_menu::root_menu);
    sub_68FFCB(debug_menu::root_menu);
    */

    create_memory_menu(root_menu);

    /*
    j_create_entity_variant_menu(debug_menu::root_menu);
    */

    create_entity_animation_menu(debug_menu::root_menu);
}

void debug_menu::hide()
{
    active_menu = nullptr;
    had_menu_this_frame = true;
    release_focus();
}

void debug_menu::show()
{
    active_menu = root_menu;
    grab_focus();
}

void debug_menu::grab_focus()
{
    physics_state_on_exit = !g_game_ptr->is_physics_enabled();
    g_game_ptr->enable_physics(false);
    has_focus = true;
}

void debug_menu::release_focus()
{
    g_game_ptr->enable_physics(physics_state_on_exit);
    has_focus = false;
}

mString &debug_menu::sub_B7E660()
{
    return this->field_C;
}

