#pragma once

#include "oldmath_po.h"
#include "float.hpp"

struct actor;
struct rigid_body;
struct event;
struct entity_base_vhandle;


struct prop_physics_body {
    po mesh_offset;
    rigid_body *body;
    actor *owner;
    bool contact;
};

namespace prop_system {
void collision_callback(event *base_event, entity_base_vhandle handle, void *context);
bool start(actor *owner, const vector3d &velocity, float randomness,
    float lifetime, int priority);
void stop(actor *owner, bool destroying);
void frame_advance(Float elapsed);
void environment_collision_callback(int &contact_count);
}
