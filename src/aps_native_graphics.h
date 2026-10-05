#pragma once

#include <cstddef>
#include <cstdint>

struct nglTexture;
namespace aeps {
struct GroupRenderInfo;
}

namespace aps_native {
struct GraphicsVtable;
struct Graphics {
    const GraphicsVtable *vtable;
    std::uint32_t sort_by_distance;
    nglTexture *texture;
};
struct SpriteGraphics {
    Graphics base;
    std::uint32_t blend;
};
struct AtlasGraphics {
    SpriteGraphics sprite;
    float columns;
    float last_frame;
};
struct MeshGraphics {
    Graphics base;

    void *mesh_instance;
};
struct Float4 {
    float x, y, z, w;
};
struct SampledGraphicsValue {
    Float4 random[8];
    Float4 lifetime[16];
    Float4 bias;
    Float4 serialized_range[2];
};
struct ProgrammableGraphics {
    Graphics base;
    std::uint32_t animate;
    std::uint32_t rotate;
    std::uint32_t move;
    std::uint32_t face_camera;
    std::uint32_t frame_count;
    std::uint32_t field_20;
    float field_24;
    float position_precision;
    std::uint32_t list_type;
    std::uint32_t filter;
    std::uint32_t field_34[3];
    Float4 x_axis;
    Float4 y_axis;
    SampledGraphicsValue colour;
    SampledGraphicsValue size;
    Float4 rotation[8];
    Float4 frame[8];
    Float4 position_random;
    Float4 velocity;
    Float4 velocity_random;
    Float4 acceleration;
    Float4 position_min;
    Float4 position_max;
    float copies;
    float field_524;
    float time_offset;
    float time_scale;
    std::uint32_t blend;
    std::uint32_t wrap;
    std::uint32_t alpha_reference;
    float particle_rotation_scale;
};
static_assert(sizeof(Graphics) == 12);
static_assert(sizeof(SpriteGraphics) == 16);
static_assert(sizeof(AtlasGraphics) == 24);
static_assert(sizeof(MeshGraphics) == 16);
static_assert(sizeof(SampledGraphicsValue) == 432);
static_assert(sizeof(ProgrammableGraphics) == 1344);
static_assert(offsetof(ProgrammableGraphics, colour) == 96);
static_assert(offsetof(ProgrammableGraphics, size) == 528);
static_assert(offsetof(ProgrammableGraphics, rotation) == 960);
static_assert(offsetof(ProgrammableGraphics, position_random) == 1216);
static_assert(offsetof(ProgrammableGraphics, blend) == 1328);
}  // namespace aps_native


void aps_native_graphics_fixup(void *object, unsigned type);
unsigned aps_native_graphics_type(const void *object);
unsigned aps_native_graphics_format(const void *object);
void aps_native_graphics_render(void *object, aeps::GroupRenderInfo *info);
