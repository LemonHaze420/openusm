#pragma once

#include "progress.h"

struct limited_timer;

struct timed_progress : progress {
    limited_timer *field_4;
};
