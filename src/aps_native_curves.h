#pragma once

#include <cstdint>



namespace aps_native {
struct Container {
    std::uint32_t end_offset, count, data, capacity;
};
struct Domain { std::uint32_t vtable; };
struct ScalarRange { Domain base; float minimum, maximum; };
struct VectorRange { Domain base; float minimum[3], maximum[3]; };
struct Point { Domain base; float value; };
struct VectorPoint { Domain base; float value[3]; };
struct QuaternionPoint { Domain base; float value[4]; };
struct Disc { Domain base; float center[3], axis_u[3], axis_v[3], radius; };
struct Sphere { Domain base; float center[3], radius; };
struct Line { Domain base; float origin[3], direction[3]; };
struct Action {
    std::uint32_t vtable;
    Container parameters;
    Container domains;
    std::uint32_t owns_domains;
    std::uint32_t is_operator;
    std::uint32_t required_attributes;
};
struct Emitter { Action base; std::uint32_t field_30; };
struct MovementAction { Action base; float previous_position[3]; };
struct ParticleFormat {
    std::uint32_t attributes, stride;
    std::uint8_t offsets[21];
    std::uint8_t padding[3];
};


struct CurveServices {
    void (*kill)(void *group, void *particle);
    void (*emit)(void *action, void *begin, void *end, void *group, float dt, const float *modifiers);
    void (*unmash)(void *object, void *info, int argument);
    void (*camera_basis)(float *right, float *up, float *angle);
};
static_assert(sizeof(Action) == 48 && sizeof(Emitter) == 52);
static_assert(sizeof(MovementAction) == 60 && sizeof(ParticleFormat) == 32);
static_assert(sizeof(Disc) == 44 && sizeof(VectorRange) == 28);
}

void aps_native_curve_set_services(const aps_native::CurveServices &services);
void aps_native_curve_fixup(void *object, unsigned type);
unsigned aps_native_curve_type(const void *object);


void aps_native_domain_sample(const void *object, int components, float *output);
bool aps_native_domain_contains(const void *object, int components, const float *value);

void aps_native_action_update(void *object, void *begin, void *end, void *group, float dt);
int aps_native_emitter_count(const void *object, void *group, float dt);


void aps_native_actions_run(const void *actions, void *group, float time, float dt, bool reset, const float *modifiers);
void aps_native_curve_release(void *object);
