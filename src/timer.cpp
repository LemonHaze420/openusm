#include "timer.h"

#include "common.h"
#include "func_wrapper.h"
#include "game.h"
#include "trace.h"
#include "utility.h"

#include <profileapi.h>

VALIDATE_SIZE(Timer, 0x58);

#if !STANDALONE_SYSTEM
Timer *&g_timer = var<Timer *>(0x00965BF0);
#else
static Timer *g_timer_storage = nullptr;
Timer *&g_timer = g_timer_storage;
#endif

Timer::Timer(Float a2, Float a3)
{
    TRACE("Timer::Timer", std::to_string(float(a2)).c_str(), std::to_string(float(a3)).c_str());

    this->field_24 = QueryPerformanceFrequency(&this->field_0);
    this->field_8.LowPart = 0;
    this->field_8.HighPart = 0;
    this->field_10 = 0;
    this->field_14 = 0;
    this->field_1C = 0;
    this->field_20 = 0;
    this->field_3C = 0;
    this->field_54 = 0;
    this->field_40 = 0;
    this->field_44 = 0;
    this->field_48 = 0;
    this->field_4C = 0;
    this->field_50 = 0;
    this->field_18 = 1000;
    this->field_28 = a2;
    this->field_2C = a3;
    this->field_38 = 1;
    this->field_30 = 1.0 / a2;
    this->field_34 = 1.0 / a3;
}

float Timer::sub_5821D0()
{
    if constexpr (STANDALONE_SYSTEM) {
        if ((field_24 != 0 && (field_10 != 0 || field_14 != 0)) ||
            (field_24 == 0 && field_20 != 0)) {
            return 0.0f;
        }

        float elapsed;
        if (field_24 != 0) {
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            const LONGLONG ticks = now.QuadPart - field_8.QuadPart;
            field_8 = now;
            elapsed = static_cast<float>(
                static_cast<double>(ticks) / static_cast<double>(field_0.QuadPart));
        } else {
            const DWORD now = GetTickCount();
            const DWORD milliseconds = now - static_cast<DWORD>(field_1C);
            field_1C = static_cast<int>(now);
            elapsed = static_cast<float>(milliseconds) / static_cast<float>(field_18);
        }

        field_3C += elapsed;
        if (field_3C >= 1.0f) {
            field_48 = field_44;
            field_44 = 0;
            field_50 = field_4C;
            field_4C = 0;
            field_54 = field_3C > 0.0f ? static_cast<float>(field_50) / field_3C : 0.0f;
            field_3C -= 1.0f;
            field_40 -= 1.0f;
        }

        if (field_40 + field_34 <= field_3C) {
            field_40 += field_34;
            ++field_44;
            field_38 = 0;
            return field_34;
        }

        if (field_40 + field_30 <= field_3C) {
            const float increment = field_3C - field_40;
            field_40 += increment;
            ++field_44;
            field_38 = 0;
            return increment;
        }

        if (field_38 != 0)
            return 0.0f;

        field_38 = 1;
        ++field_4C;
        return -1.0f;
    } else {
        float(__fastcall *func)(void *) = CAST(func, 0x005821D0);
        return func(this);
    }
}

void Timer::sub_582180()
{
    if (this->field_24) {
        QueryPerformanceCounter(&this->field_8);
        this->field_3C = 0;
        this->field_54 = 0;
        this->field_40 = 0;
        this->field_44 = 0;
        this->field_4C = 0;
    } else {
        DWORD v2 = GetTickCount();
        this->field_3C = 0;
        this->field_54 = 0;
        this->field_40 = 0;
        this->field_44 = 0;
        this->field_4C = 0;
        this->field_1C = v2;
    }
}

Timer *__fastcall Timer_constructor(Timer *self, void *, Float a2, Float a3)
{
    return new (self) Timer{a2, a3};
}

void Timer_patch()
{
    {
        REDIRECT(0x005AC399, Timer_constructor);
        REDIRECT(0x0076E7DF, Timer_constructor);
    }

    {
        FUNC_ADDRESS(address, &Timer::sub_5821D0);
        REDIRECT(0x005D7000, address);
        REDIRECT(0x005D702C, address);
    }
}
