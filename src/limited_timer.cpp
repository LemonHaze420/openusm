#include "limited_timer.h"

#include "variable.h"
#include "variables.h"

#include <windows.h>

limited_timer_base::limited_timer_base()
{
    if (!g_master_clock_is_up) {
        timeBeginPeriod(1u);
    }

    this->field_0 = timeGetTime();
}

limited_timer::limited_timer(Float a1) : limited_timer_base() {
    field_4 = a1;
}

limited_timer::~limited_timer() {
    this->reset();
}

void limited_timer_base::reset() {
    this->field_0 = timeGetTime();
}

double limited_timer_base::elapsed() const {
    return (double) (timeGetTime() - this->field_0) * 0.001;
}
