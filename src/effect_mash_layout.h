#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

// PC effect mash records. Pointer words remain serialized 32-bit values here.
namespace effect_mash {
struct Vector {
    std::uint32_t end_offset;
    std::uint32_t count;
    std::uint32_t data;
    std::uint32_t capacity;
    std::uint32_t flags;
};
struct ActionList {
    std::uint32_t type;
    std::uint32_t field_4;
    Vector slots;
};
struct EnxMesh {
    std::uint32_t type;
    std::array<std::uint32_t, 6> field_4;
    ActionList actions;
};
struct ParticleInstance {
    std::uint32_t type;
    std::array<std::uint32_t, 31> field_4;
    Vector points;
    std::uint32_t field_94;
};
struct ParticleTemplate {
    std::array<std::uint32_t, 5> field_0;
    std::uint32_t graphics;
    std::uint32_t curves;
};
struct EffectTemplate {
    Vector particles;
    Vector auxiliaries;
    std::array<std::uint32_t, 26> field_28;
};
struct Curve {
    std::uint32_t type;
    // Unlike Vector, curve containers have no trailing flags word.
    std::array<std::uint32_t, 4> samples;
    std::array<std::uint32_t, 4> values;
    std::array<std::uint32_t, 3> field_24;
};
struct Point { std::array<std::uint32_t, 6> words; };
struct Auxiliary { std::array<std::uint32_t, 11> words; };

// APS graphics types 0..7 have fixed payloads; 8..22 are fixed curve values; 23..54 contain
// the two Curve containers (52/53 additionally contain an inline 12-byte value).
inline constexpr std::array<std::uint16_t, 55> aps_sizes{
    16,16,16,24,16,16,24,1344,
    12,28,8,16,20,12,28,44,44,20,20,28,20,20,20,
    52,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,60,60,48
};
// PC mash_virtual_base types 5,6,8,9: event, particle, script and sound slots.
inline constexpr std::array<std::uint16_t, 10> action_sizes{0,0,0,0,0,64,112,0,152,80};
inline constexpr std::uint32_t enx_mesh_type = 3;
inline constexpr std::uint32_t particle_slot_type = 6;
inline constexpr std::uint32_t particle_instance_type = 7;
inline constexpr std::array<std::uint8_t, 8> interface_marker{0xA2,0xA2,0xA2,0xA2,0xA2,0xA2,0xA2,0xA2};
inline constexpr std::uint8_t alignment_marker = 0xA1;

static_assert(sizeof(Vector) == 20);
static_assert(sizeof(ActionList) == 28);
static_assert(sizeof(EnxMesh) == 56 && offsetof(EnxMesh, actions) == 28);
static_assert(sizeof(ParticleInstance) == 152 && offsetof(ParticleInstance, points) == 128);
static_assert(sizeof(ParticleTemplate) == 28 && offsetof(ParticleTemplate, graphics) == 20);
static_assert(sizeof(EffectTemplate) == 144 && offsetof(EffectTemplate, auxiliaries) == 20);
static_assert(sizeof(Curve) == 48 && offsetof(Curve, values) == 20);
}
