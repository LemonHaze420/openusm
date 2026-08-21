#include "spiderman_consolecmds.h"

#include "console.h"
#include "geometry_manager.h"
#include "us_lighting.h"
#include "trace.h"
#include "wds.h"

static AmbientLightCommand g_AmbientLightCommand{};

AmbientLightCommand::AmbientLightCommand()
{
    TRACE("AmbientLightCommand::AmbientLightCommand");
    this->setName("amb_light");
}

bool AmbientLightCommand::process_cmd(const std::vector<std::string> &a2)
{
    auto *hero_ptr = g_world_ptr->get_hero_ptr(0);
    auto *mgr = hero_ptr->get_light_set();
    assert(mgr != nullptr);

    if (a2.size() != 1 && a2.size() != 3) {
        assert(0 && "This command is no supported until we figure out what to do with that damn lites structure");
        g_console->addToLog("Ambient light:");
    }

    return true;
}

static SetTimeOfDayCommand g_SetTimeOfDayCommand{};

SetTimeOfDayCommand::SetTimeOfDayCommand()
{
    this->setName("tod");
}

bool SetTimeOfDayCommand::process_cmd(const std::vector<std::string> &a2)
{
    if (a2.size() == 1) {
        auto &v2 = a2.at(0);
        auto *v6 = v2.c_str();
        auto v5 = v6[0] - '0';
        if (v5 > 3 || v6[1]) {
            assert("Time of day out of range!");
        }

        us_lighting_switch_time_of_day(v5);
    } else {
        auto *v3 = this->helpText();
        g_console->addToLog(v3);
    }

    return true;
}

#if 1

#include "game.h"
#include "message_board.h"
#include "ngl_mesh.h"
#include "ngl_scene.h"
#include "nglrendernode.h"
#include "us_person.h"

static SetBlendModeCommand g_SetBlendModeCommand{};

SetBlendModeCommand::SetBlendModeCommand()
{
    this->setName("blend");
}

bool SetBlendModeCommand::process_cmd(const std::vector<std::string> &a2)
{
    TRACE("SetBlendModeCommand::process_cmd");
    if (a2.size() == 1) {
        auto &v2 = a2.at(0);
        auto *v6 = v2.c_str();
        int mode = v6[0] - '0';

        if (mode < 0 || mode > 8) {
            return true;
        }

        auto *hero = g_world_ptr->get_hero_ptr(0);
        auto *mesh = hero->get_mesh();

        for (auto i = 0u; i < mesh->NSections; ++i) {
            auto *MeshSection = mesh->Sections[i].Section;

            auto *Material = MeshSection->Material;
            Material->m_blend_mode = static_cast<nglBlendModeType>(mode);
        }
    } else {
        auto *v3 = this->helpText();
        g_console->addToLog(v3);
    }

    return true;
}

static SetMaterialFeaturesCommand g_SetMaterialFeaturesCommand{};

SetMaterialFeaturesCommand::SetMaterialFeaturesCommand()
{
    this->setName("mat");
}

bool SetMaterialFeaturesCommand::process_cmd(const std::vector<std::string> &a2)
{
    TRACE("SetMaterialFeaturesCommand::process_cmd");
    if (a2.size() == 1) {
        auto &v2 = a2.at(0);
        auto *v6 = v2.c_str();
        int mode = v6[0] - '0';

        if (mode < 0 || mode > 4) {
            return true;
        }

        if (mode == 4) {
            auto *hero = g_world_ptr->get_hero_ptr(0);
            auto *mesh = hero->get_mesh();
            for (auto i = 0u; i < mesh->NSections; ++i) {
                auto *MeshSection = mesh->Sections[i].Section;

                auto *Material = MeshSection->Material;

                Material->field_44 = !Material->field_44;
            }

            auto *v5 = g_game_ptr->mb;
            constexpr float v9 = 2.0;
            color32 v10{255, 255, 255, 255};
            mString v1{"Changed lightParam"};
            auto v8 = *bit_cast<message_board::string *>(&v1);
            v5->post(v8, v9, v10);
            return true;
        }

        std::tuple<int, int, const char *> arr[4] = {{0, 0, "Disabled Ink and Highlight features!"},
                                                     {0, 1, "Enabled Ink and disabled Highlight feature!"},
                                                     {1, 0, "Disabled Ink and enabled Highlight feature!"},
                                                     {1, 1, "Enabled Ink and Highlight features!"}};

        auto *hero = g_world_ptr->get_hero_ptr(0);
        auto *mesh = hero->get_mesh();

        for (auto i = 0u; i < mesh->NSections; ++i) {
            auto *MeshSection = mesh->Sections[i].Section;

            auto *Material = MeshSection->Material;
            Material->field_3C = std::get<0>(arr[mode]);
            Material->field_40 = std::get<1>(arr[mode]);
        }

        auto *v5 = g_game_ptr->mb;
        constexpr float v9 = 2.0;
        color32 v10{255, 255, 255, 255};
        mString v1{std::get<2>(arr[mode])};
        auto v8 = *bit_cast<message_board::string *>(&v1);
        v5->post(v8, v9, v10);
    } else {
        auto *v3 = this->helpText();
        g_console->addToLog(v3);
    }

    return true;
}

static SetCameraCommand g_SetCameraCommand{};

SetCameraCommand::SetCameraCommand()
{
    this->setName("camera");
}

bool SetCameraCommand::process_cmd(const std::vector<std::string> &a2)
{
    TRACE("SetBlendModeCommand::process_cmd");
    if (a2.size() == 1) {
        auto &v2 = a2.at(0);
        auto *v6 = v2.c_str();
        int mode = v6[0] - '0';

        if (mode < 0 || mode > 2) {
            return true;
        }

        auto v16 = mode;

        if (v16) {
            if (v16 == 1) {
                if (geometry_manager::is_scene_analyzer_enabled()) {
                    geometry_manager::enable_scene_analyzer(false);
                }

                g_game_ptr->enable_user_camera(true);

            } else if (v16 == 2) {
                g_game_ptr->enable_user_camera(false);
                geometry_manager::enable_scene_analyzer(true);
            }
        } else {
            if (geometry_manager::is_scene_analyzer_enabled()) {
                geometry_manager::enable_scene_analyzer(false);
            }

            g_game_ptr->enable_user_camera(false);
        }

    } else {
        auto *v3 = this->helpText();
        g_console->addToLog(v3);
    }

    return true;
}

#endif
