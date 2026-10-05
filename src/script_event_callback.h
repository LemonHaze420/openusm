#pragma once
#include "event_callback.h"

struct event;
class script_instance;
struct vm_executable;
struct script_event_callback : event_callback {
    script_instance *instance;
    const vm_executable *executable;
    script_event_callback(script_instance *, const vm_executable *, const char *, bool);
    ~script_event_callback();
};
