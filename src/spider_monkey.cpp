#include "spider_monkey.h"

#include "func_wrapper.h"
#include "input_mgr.h"
#include "log.h"
#include "os_developer_options.h"
#include "trace.h"
#include "utility.h"

float spider_monkey::state_callback(int a1)
{
    return spider_monkey::m_game_control_state()[a1];
}

float spider_monkey::delta_callback(int a1)
{
    return spider_monkey::m_game_control_state()[a1] - spider_monkey::m_game_control_state_last_frame()[a1];
}

#include "game.h"

void spider_monkey::render()
{
    CDECL_CALL(0x004B6890);
}

void spider_monkey::on_level_load()
{
    CDECL_CALL(0x004B3910);
}

void spider_monkey::on_level_unload()
{
    CDECL_CALL(0x004B3B20);
}

void spider_monkey::start()
{
    if constexpr (1) {
        input_mgr::instance->set_control_state_monkey_callback(spider_monkey::state_callback);
        input_mgr::instance->set_control_delta_monkey_callback(spider_monkey::delta_callback);
        m_running() = 1;
        m_ook_timer() = 0.0;
        m_clock().reset();
        m_runtime() = 0.0;
        m_runtime_text() = 0;
        m_runtime_monkey_text() = 0;
        os_developer_options::instance->set_int(9, 1);  //MONKEY_MODE
    } else {
        CDECL_CALL(0x004B6690);
    }
}

void spider_monkey::stop() {}

void spider_monkey::frame_advance(Float a1)
{
    TRACE("spider_monkey::frame_advance");

    CDECL_CALL(0x004B6770, a1);
}

bool spider_monkey::is_running()
{
    //sp_log("spider_monkey::is_running(): %d", spider_monkey::m_running());

    return spider_monkey::m_running();
}

void spider_monkey_patch()
{
    {
        REDIRECT(0x0055D761, spider_monkey::frame_advance);
    }

    REDIRECT(0x0052B4BF, spider_monkey::render);

    REDIRECT(0x0052B5DC, spider_monkey::is_running);
}
