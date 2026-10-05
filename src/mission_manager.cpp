#include "mission_manager.h"
#include "als_animation_logic_system.h"
#include "mission_stack_manager.h"
#include "entity.h"
#include "cut_scene_player.h"
#include "ngl.h"
#include "panelfile.h"
#include "panelquad.h"
#include "fetext.h"
#include "physical_interface.h"
#include "resource_partition.h"
#include "resource_pack_streamer.h"
#include "vtbl.h"

#include "common.h"
#include "event.h"
#include "event_manager.h"
#include "func_wrapper.h"
#include "game.h"
#include "log.h"
#include "mission_manager_script_data.h"
#include "mission_table_container.h"
#include "mstring.h"
#include "oldmath_po.h"
#include "parse_generic_mash.h"
#include "region.h"
#include "resource_manager.h"
#include "script_manager.h"
#include "sound_manager.h"
#include "script_executable.h"
#include "script_executable_entry.h"
#include "trace.h"
#include "trigger_manager.h"
#include "variables.h"
#include "wds.h"
#include "eligible_pack.h"
#include "script.h"
#include "chuck/vm/script_object.h"
#include "chuck/vm/vm_executable.h"
#include <algorithm>
#include <cstdlib>

#include <cassert>
#include <cfloat>

VALIDATE_SIZE(mission_manager, 0x100u);

mission_manager *&mission_manager::s_inst = var<mission_manager *>(0x00968518);

mString &mission_manager::current_mission_debug_title = var<mString>(0x00969E90);


static void suspend_mission_hero()
{
    auto *hero = g_world_ptr->get_hero_ptr(0);
    if (hero != nullptr) {
        hero->suspend(true);
        hero->field_8 |= 0x4000;
        hero->physical_ifc()->suspend(true);
        hero->physical_ifc()->enable(false);
    }
}

mission_manager::mission_manager()
{
    if constexpr (STANDALONE_SYSTEM) {
        s_inst = this;
        field_0 = 0.5f;
        field_4 = 0.0f;
        m_global_table_container = nullptr;
        for (auto &table : m_district_table_containers)
            table = nullptr;
        m_district_table_count = 0;
        m_script_to_load = nullptr;
        m_script = nullptr;
        m_unload_script = false;
        field_54 = 0;
        field_58 = 5.0f;
        field_5C = 0;
        field_60 = nullptr;
        field_64 = 0.0f;
        field_68 = 0;
        field_6C = nullptr;
        field_70 = 0.0f;
        field_74 = 1;
        field_78 = nullptr;
        field_7C = nullptr;
        field_80 = false;
        field_84 = -1;
        field_88 = fixedstring<8>{""};
        field_A8 = fixedstring<8>{""};
        field_C8 = -1;
        field_CC = false;
        field_D0 = fixedstring<8>{""};
        hero_switch_frame = -1;
        field_F4 = 0.0f;
        field_F8 = -1.0f;
        field_FC = 0;

        vector2d panel_positions[] = {
            {410.0f, 400.0f},
            {575.0f, 400.0f},
            {395.0f, 430.0f},
            {560.0f, 430.0f},
        };
        color32 panel_colors[] = {
            {21, 21, 99, 255},
            {21, 21, 99, 255},
            {21, 21, 99, 255},
            {21, 21, 99, 255},
        };
        field_8 = new PanelQuad;
        field_8->Init(panel_positions, panel_colors, static_cast<panel_layer>(1), 2.0f, "");
        vector2d border_positions[] = {
            {408.0f, 398.0f},
            {577.0f, 398.0f},
            {393.0f, 432.0f},
            {562.0f, 432.0f},
        };
        color32 border_colors[] = {
            {0, 0, 0, 255},
            {0, 0, 0, 255},
            {0, 0, 0, 255},
            {0, 0, 0, 255},
        };
        field_C = new PanelQuad;
        field_C->Init(border_positions, border_colors, static_cast<panel_layer>(1), 3.0f, "");
        field_10 = new FEText(static_cast<font_index>(1),
                              static_cast<global_text_enum>(293),
                              485.0f,
                              415.0f,
                              1,
                              static_cast<panel_layer>(1),
                              1.0f,
                              0,
                              0,
                              color32{});
    } else {
        void(__fastcall * func)(mission_manager *) = CAST(func, 0x005DA010);
        func(this);
    }
}

void mission_manager::prepare_unload_script()
{
    if (this->m_script != nullptr) {
        if (!this->m_unload_script) {
            auto *v1 = this->m_script->field_0.c_str();
            sp_log("Preparing to unload script '%s'", v1);
            this->m_unload_script = true;
            this->field_80 = false;
        }
    }
}

void mission_manager::force_mission(int a2, const char *a3, int a4, const char *a5)
{
    if constexpr (1) {
        this->prepare_unload_script();
        this->field_80 = true;
        this->field_84 = a2;

        fixedstring<8> v5{a3};
        this->field_88 = v5;
        this->field_C8 = a4;
        if (a5 != nullptr) {
            fixedstring<8> v6{a5};
            this->field_A8 = v6;
        } else {
            fixedstring<8> v7{mString::null};
            this->field_A8 = v7;
        }
    } else {
        THISCALL(0x005C5A00, this, a2, a3, a4, a5);
    }
}

void mission_manager::set_real_time()
{
    {
        mString a1 = mString{"real_world_timer"};

        this->field_60 = CAST(this->field_60, script_manager::get_game_var_address(a1, nullptr, nullptr));
    }

    float *v2 = CAST(v2, this->field_60);
    this->field_64 = 0.0;
    this->field_5C = static_cast<uint32_t>(*v2);
}

void mission_manager::show_mission_loading_panel(const mString &a1)
{
    TRACE("mission_manager::show_mission_loading_panel", a1.c_str());

    if constexpr (STANDALONE_SYSTEM) {
        if (a1 == mString{"fade"}) {
            sub_5BACA0(1.0f);
            return;
        }
        suspend_mission_hero();
        g_game_ptr->field_165 = true;
        field_FC = 4;
        g_cut_scene_player()->field_154 = -1.0f;
        const mString pack = mString{"ts_"} + a1;
        const mString label{"loading screen"};
        mission_stack_manager::s_inst->push_mission_pack(label, pack, -1, true);
        auto *partition = resource_manager::get_partition_pointer(RESOURCE_PARTITION_MISSION);
        resource_manager::push_resource_context(partition->get_pack_slots().front());
        const mString title = mString{"title_"} + a1;
        auto *panel = PanelFile::UnmashPanelFile(title.c_str(), static_cast<panel_layer>(7));
        resource_manager::pop_resource_context();
        for (int frame = 0; frame != 2; ++frame) {
            nglListInit();
            nglSetClearFlags(1);
            nglSetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            panel->Draw();
            nglListSend(true);
        }
        game::render_empty_list();
        mission_stack_manager::s_inst->pop_mission_pack(label, pack);
        partition->get_streamer()->flush(nullptr);
    } else {
        THISCALL(0x005DA4B0, this, &a1);
    }
}

void mission_manager::run_script(const mission_manager_script_data &data)
{
    if constexpr (STANDALONE_SYSTEM) {
        m_script = new mission_manager_script_data{data};
        current_mission_debug_title =
            mString{0, "%s (%s)", m_script->field_0.c_str(), m_script->field_A4.m_hash.to_string()};
        auto *slot = resource_manager::get_partition_pointer(RESOURCE_PARTITION_MISSION)->get_pack_slots().front();
        resource_manager::push_resource_context(slot);
        const resource_key key{string_hash{data.field_0.c_str()}, RESOURCE_KEY_TYPE_SCRIPT};
        auto *entry = script_manager::load(key, 0, slot, resource_key{});
        script_manager::link();
        if (data.field_B5) {
            static const string_hash callback{"mission_called_from_debug_menu()"};
            if (auto *function = entry->exec->find_function_by_name(callback)) {
                auto *instance = function->owner->instances->_first_element;
                const int index = script::find_function(callback, instance->parent, false);
                if (index >= 0)
                    script::new_thread(index, instance);
                script::exec_thread(false);
            }
        }
        if (entry != nullptr)
            entry->field_C = data.field_A4;
        resource_manager::pop_resource_context();
        if (!data.field_C8.empty() && _strcmpi(data.field_C8.c_str(), g_world_ptr->field_3E0.c_str()) != 0) {
            field_D0 = fixedstring<8>{data.field_C8.c_str()};
            hero_switch_frame = 0;
        }
    } else {
        THISCALL(0x005DEFA0, this, &data);
    }
}

void mission_manager::unlock()
{
    this->field_CC = false;
}

void mission_manager::lock()
{
    this->field_CC = true;
}

void mission_manager::unload_script_now()
{
    if (this->m_script == nullptr) {
        return;
    }

    auto *mission_partition = resource_manager::get_partition_pointer(RESOURCE_PARTITION_MISSION);
    if (this->m_script == nullptr) {
        goto LABEL_5;
    }

    if (!this->m_unload_script) {
        this->m_unload_script = true;
        this->field_80 = false;
    LABEL_5:

        if (!this->m_unload_script) {
            return;
        }

        goto LABEL_6;
    }

    do {
    LABEL_6:
        this->unload_script_if_requested();
        this->hero_switch_frame = -1;
        if (mission_partition != nullptr) {
            mission_partition->get_streamer()->flush(game::render_empty_list);
        }

    } while (this->m_unload_script);
}

void mission_manager::unload_script_if_requested()
{
    TRACE("mission_manager::unload_script_if_requested");

    if constexpr (STANDALONE_SYSTEM) {
        if (!m_unload_script)
            return;
        auto *partition = resource_manager::get_partition_pointer(RESOURCE_PARTITION_MISSION);
        if (field_54 != 0) {
            if (!partition->get_pack_slots().empty()) {
                auto *stack = mission_stack_manager::s_inst;
                if (stack->pack_loads_or_unloads_pending == 0)
                    stack->pop_mission_pack_internal();
            } else {
                if (m_script->field_D8.size() != 0 &&
                    _strcmpi(m_script->field_D8.c_str(), g_world_ptr->field_3E0.c_str()) != 0) {
                    field_D0 = fixedstring<8>{m_script->field_D8.c_str()};
                    hero_switch_frame = 0;
                }
                field_54 = 0;
                m_unload_script = false;
                delete m_script;
                m_script = nullptr;
                current_mission_debug_title = "";
            }
        } else {
            for (auto &entry : als::animation_logic_system_interface::the_als_list) {
                if (entry.field_0->sub_4933E0()) {
                    entry.field_0->reset_animation_player();
                    break;
                }
            }
            resource_manager::push_resource_context(partition->get_pack_slots().front());
            resource_key script_key{string_hash{m_script->field_0.c_str()}, RESOURCE_KEY_TYPE_SCRIPT};
            script_manager::un_load(script_key, true, resource_key{});
            resource_manager::pop_resource_context();
            field_54 = 1;
        }
    } else {
        THISCALL(0x005DBD00, this);
    }
}

void mission_manager::load_script(const mission_manager_script_data &data)
{
    TRACE("mission_manager::load_script");

    if constexpr (STANDALONE_SYSTEM) {
        assert(data.uses_script_stack);

        assert(m_script_to_load == nullptr);

        mString v10{"pk_"};

        mString a3 = v10 + data.field_0;

        string_hash v7{a3.c_str()};

        resource_key v8{v7, RESOURCE_KEY_TYPE_PACK};

        if (resource_manager::get_pack_file_stats(v8, nullptr, nullptr, nullptr)) {
            this->m_script_to_load = new mission_manager_script_data{};

            this->m_script_to_load->copy(data);
            if (!data.field_B8.empty()) {
                this->show_mission_loading_panel(data.field_B8);
            }

            this->field_4 = 0;
            mission_stack_manager::s_inst->push_mission_pack(data.field_0, a3, -1, false);
        }
    } else {
        THISCALL(0x005DEE40, this, &data);
    }
}

void mission_manager::render_fade()
{
    if (field_F4 >= 0.0001f) {
        unsigned int alpha = static_cast<unsigned int>(field_F4 * 255.0f);
        if (alpha > 250) {
            alpha = 255;
            if (field_F4 >= 1.0f) {
                g_game_ptr->field_165 = true;
                g_game_ptr->field_166 = false;
            }
        }
        nglQuad quad;
        nglInitQuad(&quad);
        nglSetQuadRect(&quad,
                       -0.5f,
                       -0.5f,
                       static_cast<float>(nglGetScreenWidth()) + 0.5f,
                       static_cast<float>(nglGetScreenHeight()) + 0.5f);
        nglSetQuadZ(&quad, 0.0f);
        nglSetQuadColor(&quad, alpha << 24);
        nglListAddQuad(&quad);
    }
}

void mission_manager::sub_5BACA0(Float a2)
{
    if (field_FC != 3 && field_FC != 4) {
        field_4 = 0.0f;
        if (a2 > 0.0f) {
            field_F8 = 1.0f / a2;
        } else {
            for (int frame = 0; frame != 2; ++frame) {
                nglListInit();
                nglListBeginScene(static_cast<nglSceneParamType>(0));
                nglSetClearFlags(1);
                nglSetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                nglListEndScene();
                nglListSend(true);
            }
            field_F4 = 1.0f;
            field_F8 = FLT_MAX;
        }
        field_FC = 1;
        g_game_ptr->field_166 = true;
        suspend_mission_hero();
    }
}

void mission_manager::frame_advance(Float a2)
{
    TRACE("mission_manager::frame_advance");
    if constexpr (STANDALONE_SYSTEM) {
        if ((!g_game_ptr->flag.physics_enabled || g_game_ptr->flag.single_step) && g_game_ptr->level.load_completed &&
            g_game_ptr->flag.level_is_loaded) {
            auto v4 = this->field_FC;
            if (v4 == 4 || v4 == 3) {
                this->sub_5BB220(a2);
            }

            auto v5 = a2 * this->field_F8 + this->field_F4;
            this->field_F4 = v5;
            if (v5 < 0.0f) {
                auto v6 = this->field_FC == 2;
                this->field_F4 = 0.0;
                this->field_F8 = 0.0;
                if (v6) {
                    this->field_FC = 0;
                    g_game_ptr->field_166 = 0;
                }
            }

            if (this->field_F4 > 1.f) {
                auto v6 = this->field_FC == 1;
                this->field_F4 = 1.0;
                this->field_F8 = 0.0;
                if (v6) {
                    this->field_FC = 3;
                }
            }

            if (this->field_60 == nullptr) {
                mString a1{"real_world_timer"};
                this->field_60 = (float *)script_manager::get_game_var_address(a1, nullptr, nullptr);
                *this->field_60 = 0.0;
            }

            auto v7 = a2 + this->field_64;
            this->field_64 = v7;
            if (v7 >= 1.f) {
                ++this->field_5C;
                *this->field_60 = static_cast<float>(this->field_5C);
                this->field_64 = this->field_64 - 1.f;
            }

            if (this->field_6C == nullptr) {
                mString a1{"game_clock_timer"};
                this->field_6C = (float *)script_manager::get_game_var_address(a1, nullptr, nullptr);
            }

            if (this->field_7C == nullptr) {
                mString a1{"game_day_of_the_week"};
                this->field_7C = (float *)script_manager::get_game_var_address(a1, nullptr, nullptr);
            }

            if (this->field_78 == nullptr) {
                mString v22{"game_days"};
                this->field_78 = (float *)script_manager::get_game_var_address(v22, nullptr, nullptr);
            }

            if (!g_game_ptr->flag.game_paused || s_freeze_game_time) {
                auto v11 = static_cast<double>(this->field_74) * a2 + this->field_70;
                this->field_70 = v11;
                if (v11 >= 1.f) {
                    do {
                        auto v12 = this->field_68 + 1;
                        this->field_68 = v12;
                        if ((v12 % 60) == 0) {
                            event_manager::raise_event(event::TIME_MINUTE_INC, entity_base_vhandle{0});
                            if (!(this->field_68 / 60 % 60)) {
                                event_manager::raise_event(event::TIME_HOUR_INC, entity_base_vhandle{0});
                            }
                        }

                        auto v13 = this->field_68;
                        if (v13 > 86400) {
                            this->field_68 = v13 - 86400;
                            *this->field_78 += 1.f;
                            event_manager::raise_event(event::TIME_DAY_INC, entity_base_vhandle{0});
                            *this->field_7C += 1.f;
                            auto *v14 = this->field_7C;
                            if (*v14 > 6.0f)
                                *v14 = 0.0;
                        }

                        *this->field_6C = static_cast<float>(this->field_68);
                        this->field_70 = this->field_70 - 1.f;
                    } while (this->field_70 >= 1.f);
                }
            }

            this->kill_braindead_script();
            mission_manager_script_data script_data{};
            this->unload_script_if_requested();

            bool v16, v17;
            if (this->m_script_to_load &&
                (v16 = mission_stack_manager::s_inst->pack_loads_or_unloads_pending == 0,
                 v17 = sound_manager::is_mission_sound_bank_ready(),
                 v16) &&
                v17 && this->field_FC != 1) {
                this->run_script(*this->m_script_to_load);
                delete this->m_script_to_load;

                this->m_script_to_load = nullptr;
            } else {
                this->sort_district_priorities();

                if (this->m_script_to_load == nullptr && this->m_script == nullptr && !this->field_CC &&
                    !g_game_ptr->flag.game_paused && this->get_script(&script_data)) {
                    if (script_data.uses_script_stack)
                        this->load_script(script_data);
                    else
                        this->run_script(script_data);
                }
            }

            this->update_hero_switch();
        }
    } else {
        THISCALL(0x005E16B0, this, a2);
    }
}

void mission_manager::kill_braindead_script()
{
    TRACE("mission_manager::kill_braindead_script");

    if constexpr (1) {
        if (this->m_script != nullptr && !this->m_unload_script) {
            auto *exec_list = script_manager::get_exec_list();

            script_executable_entry_key a2{};
            auto *v1 = this->m_script->field_0.c_str();
            string_hash v6{v1};
            resource_key v9{v6, RESOURCE_KEY_TYPE_SCRIPT};
            a2.field_0 = v9;
            a2.field_8 = resource_key{};
            auto it = exec_list->find(a2);
            auto end = exec_list->end();
            if (it != end) {
                auto &v4 = (*it);
                if (!v4.second.exec->has_threads()) {
                    if (!event_manager::does_script_have_callbacks(v4.second.exec)) {
                        assert(false && "mission script appears to be dead");
                    }
                }
            }
        }
    } else {
        THISCALL(0x005D7EF0, this);
    }
}

void mission_manager::sort_district_priorities()
{
    std::sort(m_district_table_containers,
              m_district_table_containers + m_district_table_count,
              [](const mission_table_container *left, const mission_table_container *right) {
                  return left->field_44->field_108.front()->get_priority() <
                         right->field_44->field_108.front()->get_priority();
              });
}

void mission_manager::sub_5BB220(Float a2)
{
    const float remaining = field_4 - a2;
    const bool draw_text = field_4 >= 0.0f && remaining < 0.0f;
    const bool clear_text = field_4 >= field_0 && remaining < field_0;
    if (draw_text || clear_text) {
        if (draw_text)
            field_10->SetText(static_cast<global_text_enum>(293));
        for (int frame = 0; frame != 2; ++frame) {
            nglListInit();
            nglListBeginScene(static_cast<nglSceneParamType>(0));
            nglSetClearFlags(field_FC == 3 ? 1 : 0);
            nglSetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            field_C->Draw();
            field_8->Draw();
            if (draw_text)
                field_10->Draw();
            nglListEndScene();
            nglListSend(true);
        }
    }
    field_4 = draw_text ? field_0 + field_0 : remaining;
}

void mission_manager::get_script_helper(mission_table_container *table, uint32_t *priority,
                                        _std::vector<mission_manager_script_data> *candidates)
{
    if (field_80 && !candidates->empty())
        return;
    mission_manager_script_data data;
    for (const auto &condition : table->field_38) {
        data.clear();
        if (!condition.check_condition(&data))
            continue;
        if (field_80) {
            data.field_B5 = true;
            candidates->clear();
            candidates->push_back(data);
            *priority = 0;
            break;
        }
        const auto candidate_priority = static_cast<uint32_t>(data.field_10);
        if (candidate_priority < *priority) {
            candidates->clear();
            *priority = candidate_priority;
        }
        if (candidate_priority == *priority)
            candidates->push_back(data);
    }
}

bool mission_manager::get_script(mission_manager_script_data *return_script_data)
{
    if (m_unload_script && field_80)
        return false;
    _std::vector<mission_manager_script_data> candidates;
    uint32_t priority = UINT32_MAX;
    if (m_global_table_container != nullptr && (!field_80 || field_84 == 0))
        get_script_helper(m_global_table_container, &priority, &candidates);
    if (m_district_table_count > 0) {
        static int next_district = 0;
        if (next_district >= m_district_table_count)
            next_district = 0;
        auto *table = m_district_table_containers[next_district++];
        if (!field_80 || field_84 == table->field_44->get_district_id())
            get_script_helper(table, &priority, &candidates);
    }
    if (candidates.empty())
        return false;
    field_80 = false;
    if (candidates.size() == 1) {
        return_script_data->copy(candidates.front());
    } else {
        bool repeated = true;
        for (int attempt = 0; attempt < 6; ++attempt) {
            const auto index = static_cast<unsigned int>(static_cast<double>(std::rand()) * candidates.size() /
                                                         (static_cast<double>(RAND_MAX) + 1.0));
            return_script_data->copy(candidates[index]);
            repeated = false;
            for (const auto &previous : field_44) {
                if (strncmp(return_script_data->field_0.c_str(), previous.c_str(), 0xFFFF) == 0) {
                    repeated = true;
                    break;
                }
            }
            if (!repeated)
                break;
        }
        if (!repeated) {
            field_44.push_front(return_script_data->field_0);
            if (field_44.size() > 10)
                field_44.pop_back();
        }
    }
    return true;
}

int mission_manager::add_global_table(const resource_key &key)
{
    auto *resource = resource_manager::get_resource(key, nullptr, nullptr);
    if (resource == nullptr) {
        return 0;
    }

    return parse_generic_object_mash(m_global_table_container, resource, nullptr, nullptr, nullptr, 0, 0, nullptr);
}

void mission_manager::add_district_table(void *a2, region *a3)
{
    if constexpr (1) {
        parse_generic_object_mash<mission_table_container>(
            this->m_district_table_containers[this->m_district_table_count],
            a2,
            nullptr,
            nullptr,
            nullptr,
            0u,
            0u,
            nullptr);

        this->m_district_table_containers[this->m_district_table_count++]->field_44 = a3;
    } else {
        THISCALL(0x005D1EE0, this, a2, a3);
    }
}

void mission_manager::rem_district_table(region *reg)
{
    for (int i = 0; i < m_district_table_count; ++i) {
        if (m_district_table_containers[i]->field_44 == reg) {
            m_district_table_containers[i] = m_district_table_containers[--m_district_table_count];
            return;
        }
    }
}

void mission_manager::update_hero_switch()
{
    if (int v3 = g_world_ptr->get_num_players(); v3 > 1) {
        do {
            g_world_ptr->remove_player(--v3);

        } while (v3 != 1);
    }

    if (auto v4 = this->hero_switch_frame; v4 != -1) {
        switch (v4) {
        case 0:
            assert(g_world_ptr->get_num_players() <= 1 && "update code for multiple players");

            if (g_world_ptr->get_num_players() <= 0) {
                ++this->hero_switch_frame;
            } else {
                sp_log("Removing player");
                g_world_ptr->remove_player(g_world_ptr->num_players - 1);
            }

            break;
        case 1:
            ++this->hero_switch_frame;
            break;
        case 2:
            ++this->hero_switch_frame;
            break;
        case 3: {
            auto old_num_players = g_world_ptr->get_num_players();

            sp_log("Adding player");

            mString a2 = mString{this->field_D0.to_string()};
            auto new_num_players = g_world_ptr->add_player(a2);

            assert(new_num_players > old_num_players && "unable to add player (while switching hero costumes)");

            this->hero_switch_frame = -1;
        } break;
        default:
            return;
        }
    }
}

entity_base *mission_manager::get_mission_key_entity() const
{
    assert(m_script != nullptr);

    string_hash a1{this->m_script->field_84.c_str()};
    return entity_handle_manager::find_entity(a1, IGNORE_FLAVOR, false);
}

trigger *mission_manager::get_mission_key_trigger() const
{
    mString v3{this->m_script->field_84.c_str()};
    auto *instance = trigger_manager::instance->find_instance(v3);
    return instance;
}

_std::vector<float> *mission_manager::get_mission_nums()
{
    assert(m_script != nullptr);

    assert(m_script->nums.size() > 0);

    return &this->m_script->nums;
}

_std::vector<mString> *mission_manager::get_mission_strings()
{
    assert(m_script != nullptr);

    assert(m_script->strings.size() > 0);

    return &this->m_script->strings;
}

void mission_manager::set_mission_key_po(const po &a2)
{
    assert(m_script != nullptr);

    *this->m_script->field_94 = a2;
}

po mission_manager::get_mission_key_po() const
{
    assert(m_script != nullptr);

    return (*this->m_script->field_94);
}

bool mission_manager::is_story_active() const
{
    mString v3{"gv_story_finished"};
    float *game_var_address = bit_cast<float *>(script_manager::get_game_var_address(v3, nullptr, nullptr));
    return equal(*game_var_address, 0.0f);
}

bool mission_manager::is_mission_active() const
{
    return this->m_script != nullptr;
}

void mission_manager::get_missions_nums_by_index(int a2, const char *a3, int a4, _std::vector<float> *nums_result)
{
    assert(nums_result != nullptr);

    for (int i = 0; i < this->m_district_table_count; ++i) {
        auto *reg = this->m_district_table_containers[i]->get_region();
        if (a2 == reg->get_district_id() && this->m_district_table_containers[i]->append_nums(a3, a4, nums_result)) {
            return;
        }
    }

    assert(0 && "Attempted to find nums for nonexistent mission.");
}

int mission_manager::sub_5C5BD0() const
{
    int v1 = (this->field_68 / 60u) / 60 % 24;
    if (v1 <= 7) {
        return 2;
    }

    if (v1 >= 19) {
        return 2;
    }

    return 1;
}

void mission_manager_patch()
{
    {
        FUNC_ADDRESS(address, &mission_manager::load_script);
        REDIRECT(0x005E1B1F, address);
    }

    {
        FUNC_ADDRESS(address, &mission_manager::run_script);
        REDIRECT(0x005E1A96, address);
        REDIRECT(0x005E1B2B, address);
    }

    {
        FUNC_ADDRESS(address, &mission_manager::show_mission_loading_panel);
        REDIRECT(0x005DEF4C, address);
    }

    {
        FUNC_ADDRESS(address, &mission_manager::unload_script_if_requested);
        REDIRECT(0x005DBE99, address);
        REDIRECT(0x005E1A65, address);
    }

    {
        FUNC_ADDRESS(address, &mission_manager::frame_advance);
        REDIRECT(0x0055D75B, address);
    }

    {
        FUNC_ADDRESS(address, &mission_manager::kill_braindead_script);
        SET_JUMP(0x005D7EF0, address);
    }
}
