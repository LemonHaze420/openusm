#include "consolevars.h"
#include "actor.h"
#include "base_ai_core.h"
#include "console.h"
#include "damage_interface.h"
#include "event.h"
#include "event_manager.h"
#include "game.h"
#include "game_settings.h"
#include "geometry_manager.h"
#include "message_board.h"
#include "mstring.h"
#include "os_developer_options.h"
#include "variables.h"
#include "wds.h"

#include <cassert>

std::list<ConsoleVariable *> *g_console_vars{nullptr};

ConsoleVariable::ConsoleVariable()
{
    this->setName({});

    if (g_console_vars == nullptr) {
        g_console_vars = new std::list<ConsoleVariable *>{};
    }

    g_console_vars->push_back(this);
}

static constexpr auto MAX_VARIABLE_NAME_LEN = 32;

void ConsoleVariable::setValue(const std::string &) {
    ;
}


void ConsoleVariable::setValue(const std::string &, const std::string &) {
    ;
}

std::string ConsoleVariable::getValue() {
    return {};
}

void ConsoleVariable::setName(const std::string &pName)
{
    assert(pName.size() < MAX_VARIABLE_NAME_LEN);

    auto *v2 = pName.c_str();
    strcpy(this->field_4, v2);
    strlwr(this->field_4);
}

bool ConsoleVariable::match(std::string a2) const
{
    std::string v4{this->field_4};

    return (v4 == a2);
}

std::string ConsoleVariable::getName() const
{
    std::string a2{this->field_4};
    return a2;
}

static ConsoleHeightVariable g_ConsoleHeightVariable{};

ConsoleHeightVariable::ConsoleHeightVariable() {
    setName("console_height");
}

void ConsoleHeightVariable::setValue(const std::string &a2) {
    auto *v7 = this;
    float v6 = 0.0;

    const char *v2 = nullptr;
    if (a2.length() <= 0 || (v2 = a2.c_str(), v6 = atof(v2), v6 < 25.0) || v6 > 480.0) {
        auto v4 = v7->getName();

        auto *v3 = v4.c_str();
        g_console->addToLog("'%s' must be between 25 and %d", v3, 480);

    } else {
        g_console->setHeight(v6);
    }
}

std::string ConsoleHeightVariable::getValue() {
    auto v2 = g_console->getHeight();

    char a1[32];
    sprintf(a1, "%.2f", v2);

    std::string out{a1};
    return out;
}

//static HealthVariable g_HealthVariable{};

HealthVariable::HealthVariable() {
    this->setName("health");
}

void HealthVariable::setValue(const std::string &arg0, const std::string &a1) {
    auto v41 = 0;
    if (arg0.length() > 0 && a1.length() > 0) {
        auto *v3 = arg0.c_str();
        v41 = atoi(v3);
        if (v41 >= 0)
        {
            mString v40 {a1.c_str()};
            v40.to_upper();

            auto *v39 = ai::ai_core::the_ai_core_list_high();
            if (!v39->empty()) {
                auto it = v39->begin();
                auto end = v39->end();
                bool v36 = false;

                while (it != end)
                {
                    auto &v4 = (*it);
                    auto *v35 = v4->get_actor(0);

                    auto v6 = v35->get_id();
                    auto v34 = v6;

                    string_hash v18{v40.c_str()};

                    auto v16 = (v34 == v18 && v35->has_damage_ifc());
                    auto v20 = v16;

                    if (v20) {
                        auto v22 = (float) v41;

                        auto *v7 = v35->damage_ifc();
                        v7->field_1FC.sub_48BFB0(v22);

                        auto *v8 = v35->damage_ifc();
                        int v33 = v8->field_1FC.field_0[0];

                        auto *v9 = a1.c_str();
                        mString v32{0, "%s's current hit points are now %d\n", v9, v33};

                        color32 v15{255, 255, 255, 255};
                        auto v14 = 3.0f;

                        mString v13 = v32;

                        auto *v10 = g_game_ptr->mb;

                        v10->post(*bit_cast<message_board::string *>(&v13), v14, (color32) v15);
                        v36 = true;
                        if (v41 == 0) {
                            auto v15 = v35->get_my_handle();

                            event_manager::raise_event(event::DESTROYED, v15);
                        }
                    }

                    ++it;
                }

                if (!v36) {
                    auto *v11 = a1.c_str();
                    mString v31{0, "Could not find an entity named %s\n", v11};

                    color32 v15{255, 255, 255, 255};
                    float v14 = 3.0;

                    mString v13 = v31;

                    auto *v12 = g_game_ptr->mb;

                    v12->post(*bit_cast<message_board::string *>(&v13), v14, v15);
                }
            }
        }
    }
}

void HealthVariable::setValue(const std::string &arg0) {
    auto v20 = 0;
    if (arg0.length() > 0) {
        auto *v2 = arg0.c_str();
        v20 = atoi(v2);
        if (v20 >= 0) {
            for (auto a2 = 0;; ++a2) {
                auto v3 = g_world_ptr->get_num_players();
                if (a2 >= v3) {
                    break;
                }

                auto *v12 = g_world_ptr->get_hero_ptr(a2);

                [[maybe_unused]] auto *v5 = v12->damage_ifc();
                //v5->field_1FC[0];

                auto v15 = (float) v20;

                v12 = g_world_ptr->get_hero_ptr(a2);

                auto *v7 = v12->damage_ifc();
                v7->field_1FC.sub_48BFB0(v15);
                if (v20 == 0) {
                    auto *v9 = g_world_ptr->get_hero_ptr(a2);

                    entity_base_vhandle v11 = v9->get_my_handle();

                    event_manager::raise_event(event::DESTROYED, v11);
                }
            }
        }
    }
}

bool get_boolean_value(const char *a1) {
    return a1 != nullptr && (!strcmpi(a1, "on") || !strcmpi(a1, "true") || !strcmp(a1, "1"));
}

static RenderFramerateVariable g_RenderFramerateVariable{};

RenderFramerateVariable::RenderFramerateVariable() {
    setName("render_fps");
}

void RenderFramerateVariable::setValue(const std::string &a1) {
    auto *v1 = a1.c_str();
    os_developer_options::instance->set_flag(mString{"SHOW_FPS"}, get_boolean_value(v1));
}

std::string RenderFramerateVariable::getValue() {
    auto show_fps = os_developer_options::instance->get_flag(mString{"SHOW_FPS"});

    return (show_fps ? "on" : "off");
}

static RenderInterfaceVariable g_RenderInterfaceVariable {};

RenderInterfaceVariable::RenderInterfaceVariable() {
    setName("render_interface");
}

void RenderInterfaceVariable::setValue(const std::string &a1)
{
    auto *v1 = a1.c_str();
    os_developer_options::instance->set_flag(mString{"INTERFACE_DISABLE"}, !get_boolean_value(v1));
}

std::string RenderInterfaceVariable::getValue()
{
    auto show_fps = !os_developer_options::instance->get_flag(mString{"SHOW_FPS"});

    return (show_fps ? "on" : "off");
}

//static ProjZoomVariable g_ProjZoomVariable{};

ProjZoomVariable::ProjZoomVariable() {
    setName("proj_zoom");
}

void ProjZoomVariable::setValue(const std::string &a2) {
    auto *v2 = a2.c_str();
    auto v6 = atof(v2);
    if (v6 < LARGE_EPSILON || v6 >= 2.0) {
        auto v4 = this->getName();

        auto *v3 = v4.c_str();
        g_console->addToLog("%s must be between 0.01f and 2.0f", v3);

    } else {
        geometry_manager::set_zoom(v6);
    }
}

std::string ProjZoomVariable::getValue() {
    float a2 = geometry_manager::get_zoom();
    return std::to_string(a2);
}

static DifficultyVariable g_DifficultyVariable{};

DifficultyVariable::DifficultyVariable() {
    setName("difficulty");
}

void DifficultyVariable::setValue(const std::string &a2) {
    auto *v2 = a2.c_str();
    auto v3 = atof(v2);
    int v7 = v3;
    if (v7 > 3 || v7 < 0) {
        auto v5 = this->getName();

        auto *v4 = v5.c_str();
        g_console->addToLog("%s must be an integer between 0 and 3", v4);

    } else {
        auto *v4 = g_game_ptr->get_game_settings();
        v4->field_340.m_difficulty = v7;
    }
}

std::string DifficultyVariable::getValue()
{
    auto *v1 = g_game_ptr->get_game_settings();
    float v2 = v1->field_340.m_difficulty;
    return std::to_string(v2);
}

static DisableOcclusionCullingVariable g_DisableOcclusionCullingVariable {};

void DisableOcclusionCullingVariable::setValue(const std::string &a2)
{
    g_disable_occlusion_culling = mString {a2.c_str()}.to_int();
}

std::string DisableOcclusionCullingVariable::getValue() {
    char a1[32] {};
    sprintf(a1, "%d", g_disable_occlusion_culling);
    return std::string {a1};
}
