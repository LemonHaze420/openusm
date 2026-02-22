#include "panel_resource_handler.h"

#include "combo_words.h"
#include "common.h"
#include "fe_crosshair.h"
#include "fe_distance_chase.h"
#include "fe_distance_race.h"
#include "fe_health_widget.h"
#include "fe_hotpursuit_indicator.h"
#include "fe_score_widget.h"
#include "femanager.h"
#include "igofrontend.h"
#include "medal_award_ui.h"
#include "race_announcer.h"
#include "resource_location.h"
#include "tutorial_controller_gauge.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"
#include "worldly_pack_slot.h"

VALIDATE_SIZE(panel_resource_handler, 0x14);

panel_resource_handler::panel_resource_handler(worldly_pack_slot *a2)
{
    if constexpr (1) {
        static void * g_vtbl[] = {
            func_address(&finalize),
            func_address(&_handle),
            func_address(&_pre_handle_resources),
            func_address(&_handle_resource),
        };

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00888AC4;
    }

    this->my_slot = a2;
    this->field_10 = RESOURCE_KEY_TYPE_PANEL;
}

bool panel_resource_handler::_handle(worldly_resource_handler::eBehavior a2, limited_timer *a3)
{
    TRACE("panel_resource_handler::handle");

    return base_engine_resource_handler::_handle(a2, a3);
}

//FIXME
bool panel_resource_handler::_handle_resource(worldly_resource_handler::eBehavior behavior,
                                             resource_location *a3)
{
    TRACE("panel_resource_handler::handle_resource");


    if constexpr (1)
    {
        const char * bosses[] = {
                                    "HG_BOSS_SPIDERMAN",
                                    "HG_BOSS_VENOM",
                                    "HG_BOSS_BEETLE",
                                    "HG_BOSS_CARNAGE",
                                    "HG_BOSS_WOLVERINE",
                                    "HG_BOSS_RHINO",
                                    "HG_BOSS_SABLE",
                                    "HG_BOSS_GOBLIN",
                                    "HG_BOSS_SHOCKER",
                                    "HG_BOSS_ELECTRO_NOSUIT",
                                    "HG_BOSS_ELECTRO_SUIT",
                                    "HG_BOSS_MYSTIQUE"
                                };

        const char * heroes[] = {
                                "HG_HERO_SPIDERMAN",
                                "HG_HERO_SPIDERMAN_EX_01",
                                "HG_HERO_SPIDERMAN_EX_02",
                                "HG_HERO_SPIDERMAN_EX_03",
                                "HG_HERO_VENOM",
                                "HG_HERO_PETER"
                            };

        const char * third_party[] = {
                                "HG_TP_SPIDERMAN",
                                "HG_TP_SABLE",
                                "HG_TP_WOLVERINE"
                            };

        const char * chase_meter[] = {
                                "CHASE_METER_RHINO",
                                "CHASE_METER_BEETLE",
                                "CHASE_METER_GOBLIN",
                                "CHASE_METER_ELECTRO",
                                "CHASE_METER_SABLE",
                                "CHASE_METER_VENOM",
                                "CHASE_METER_QUESTION"
                             };

        auto &dir = this->my_slot->get_resource_directory();
        auto *resource = dir.get_resource(a3, nullptr);
        assert(resource != nullptr);

        if (behavior == worldly_resource_handler::UNLOAD)
        {
            if (g_femanager.IGO != nullptr)
            {
                for ( uint32_t i {0}; i < std::size(heroes); ++i )
                {
                    if (a3->field_0.m_hash == heroes[i]) {
                        g_femanager.IGO->hero_health->DeInit(i);
                        ++this->field_C;
                        return false;
                    }
                }

                for (uint32_t i {0}; i < std::size(third_party); ++i )
                {
                    if (a3->field_0.m_hash == third_party[i])
                    {
                        g_femanager.IGO->third_party_health->DeInit(i);
                        ++this->field_C;
                        return false;
                    }
                }

                for ( uint32_t i {0}; i < std::size(bosses); ++i )
                {
                    if ( a3->field_0.m_hash == bosses[i] )
                    {
                        g_femanager.IGO->boss_health->DeInit(i);
                        ++this->field_C;
                        return false;
                    }
                }

                if (a3->field_0.m_hash == string_hash{"COMBO_WORDS"}) {
                    g_femanager.IGO->field_48->DeInit();
                    ++this->field_C;
                    return false;
                }

                if (a3->field_0.m_hash == string_hash{"MG_HOTPURSUIT_INDICATOR"}) {
                    g_femanager.IGO->field_4C->DeInit();
                    ++this->field_C;
                    return false;
                }

                if (a3->field_0.m_hash == string_hash{"HIGHSCORETRACKING"}) {
                    g_femanager.IGO->field_50->DeInit();
                    ++this->field_C;
                    return false;
                }

                if (a3->field_0.m_hash == string_hash{"MEDALS_INTERFACE"}) {
                    g_femanager.IGO->field_34->DeInit();
                    ++this->field_C;
                    return false;
                }

                if (a3->field_0.m_hash == string_hash{"RACE_ANNOUNCER"}) {
                    g_femanager.IGO->field_38->DeInit();
                    ++this->field_C;
                    return false;
                }

                if (a3->field_0.m_hash == string_hash{"TARGETING_RETICLE_VENOM"}) {
                    g_femanager.IGO->field_3C->DeInit();
                    ++this->field_C;
                    return false;
                }

                if (a3->field_0.m_hash == string_hash{"CONTROLLER_TUT_GC"} ||
                    a3->field_0.m_hash == string_hash{"CONTROLLER_TUT_XB"} ||
                    a3->field_0.m_hash == string_hash{"CONTROLLER_TUT_PS"})
                {
                    g_femanager.IGO->field_30->DeInit();
                    ++this->field_C;
                    return false;
                }

                for ( uint32_t i {0}; i < std::size(chase_meter); ++i )
                {
                    if (a3->field_0.m_hash == chase_meter[i]) {
                        g_femanager.IGO->field_18->DeInit(i);
                        ++this->field_C;
                        return false;
                    }
                }

                if (a3->field_0.m_hash == string_hash{"RACE_METER"}) {
                    g_femanager.IGO->field_1C->DeInit();
                }
            }
        }
        else
        {
            if (a3->field_0.m_hash == string_hash{"HG_HERO_PETER"}) {

                g_femanager.IGO->hero_health->Init(5, "HG_HERO_PETER", 0);
                ++this->field_C;
                return false;
            }

            if (a3->field_0.m_hash == string_hash{"HG_HERO_SPIDERMAN"})
            {
                g_femanager.IGO->hero_health->Init(0, "HG_HERO_SPIDERMAN", false);

                static Var<bool> globaly_packed_bar_need_init{0x00922544};

                if (globaly_packed_bar_need_init()) {
                    g_femanager.IGO->hero_health->Init(1, "HG_HERO_SPIDERMAN_EX_01", false);
                    g_femanager.IGO->hero_health->Init(2, "HG_HERO_SPIDERMAN_EX_02", false);
                    g_femanager.IGO->hero_health->Init(3, "HG_HERO_SPIDERMAN_EX_03", false);
                    globaly_packed_bar_need_init() = false;
                    ++this->field_C;
                    return false;
                }
            }
            else
            {
                if (a3->field_0.m_hash == string_hash{"HG_HERO_VENOM"}) {
                    g_femanager.IGO->hero_health->Init(4, "HG_HERO_VENOM", false);
                    ++this->field_C;
                    return false;
                }

                for (uint32_t i {0}; i < std::size(third_party); ++i )
                {
                    if (a3->field_0.m_hash == third_party[i])
                    {
                        g_femanager.IGO->third_party_health->Init(i, third_party[i], false);
                        ++this->field_C;
                        return false;
                    }
                }

                for ( uint32_t i {0}; i < std::size(bosses); ++i )
                {
                    if ( a3->field_0.m_hash == bosses[i] )
                    {
                        g_femanager.IGO->boss_health->Init(i, bosses[i], true);
                        ++this->field_C;
                        return false;
                    }
                }

                if (a3->field_0.m_hash == string_hash{"COMBO_WORDS"}) {
                    g_femanager.IGO->field_48->Init();
                    ++this->field_C;
                    return false;
                }

                if (a3->field_0.m_hash == string_hash{"MG_HOTPURSUIT_INDICATOR"}) {
                    g_femanager.IGO->field_4C->Init();
                    ++this->field_C;
                    return false;
                }

                if (a3->field_0.m_hash == string_hash{"HIGHSCORETRACKING"}) {
                    g_femanager.IGO->field_50->Init();
                    ++this->field_C;
                    return false;
                }

                if (a3->field_0.m_hash == string_hash{"MEDALS_INTERFACE"}) {
                    g_femanager.IGO->field_34->Init();
                    ++this->field_C;
                    return false;
                }

                if (a3->field_0.m_hash == string_hash{"RACE_ANNOUNCER"}) {
                    g_femanager.IGO->field_38->Init();
                    ++this->field_C;
                    return false;
                }

                if (a3->field_0.m_hash == string_hash{"TARGETING_RETICLE_VENOM"}) {
                    g_femanager.IGO->field_3C->Init();
                    ++this->field_C;
                    return false;
                }

                if (a3->field_0.m_hash == string_hash{"CONTROLLER_TUT_PS"} ||
                    a3->field_0.m_hash == string_hash{"CONTROLLER_TUT_XB"} ||
                    a3->field_0.m_hash == string_hash{"CONTROLLER_TUT_GC"})
                {
                    g_femanager.IGO->field_30->Init();
                    ++this->field_C;
                    return false;
                }

                for ( uint32_t i {0}; i < std::size(chase_meter); ++i )
                {
                    if (a3->field_0.m_hash == chase_meter[i])
                    {
                        g_femanager.IGO->field_18->Init(i, chase_meter[i]);
                        ++this->field_C;
                        return false;
                    }
                }

                if (a3->field_0.m_hash == string_hash{"RACE_METER"})
                {
                    g_femanager.IGO->field_1C->Init();
                    ++this->field_C;
                    return false;
                }
            }
        }

        ++this->field_C;
        return false;
    }
    else
    {
        bool (__fastcall *func)(void *, void *, worldly_resource_handler::eBehavior ,
                                             resource_location *) = CAST(func, 0x00571800);
        return func(this, nullptr, behavior, a3);
    }
}

void panel_resource_handler_patch()
{
    {
        FUNC_ADDRESS(address, &panel_resource_handler::_handle);
        set_vfunc(0x00888AC8, address);
    }

    {
        FUNC_ADDRESS(address, &panel_resource_handler::_handle_resource);
        set_vfunc(0x00888AD0, address);
    }
}
