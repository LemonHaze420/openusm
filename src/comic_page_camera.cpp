#include "comic_page_camera.h"

#include "common.h"
#include "nal_math.h"
#include "variables.h"
#include "vector3d.h"
#include <functional>
#include <algorithm>
#include <cmath>
#include <cstring>
#include "vtbl.h"

VALIDATE_SIZE(comic_panels::page_camera, 0xF0);
VALIDATE_OFFSET(comic_panels::page_camera, field_3C, 0x3C);

namespace comic_panels {
Var<page_camera *> cur_page_camera{0x0096F7D8};

struct page_camera_layer {
    nalGeneric::nalGenericInstance *instance;
    float speed;
    float end;
    void *event;
    void *modifier;
    float time;
    float previous;
    float weight;
    float blend_in;
    int state;
    int generation;
    page_camera_layer *next;
    unsigned mask;
    float priority;
    float blend_out;
    unsigned mode;
};

namespace {
void release_layer(page_camera_layer &layer)
{
    if (layer.event) {
        reinterpret_cast<void(__fastcall *)(void *, void *)>(get_vfunc(*static_cast<int *>(layer.event), 8))(
            layer.event, nullptr);
        layer.event = nullptr;
    }
    if (layer.modifier) {
        reinterpret_cast<void(__fastcall *)(void *, void *)>(get_vfunc(*static_cast<int *>(layer.modifier), 16))(
            layer.modifier, nullptr);
        layer.modifier = nullptr;
    }
    if (layer.instance) {
        layer.instance->Finalize(true);
        layer.instance = nullptr;
    }
}
void advance_layer_time(page_camera_layer &layer, Float dt)
{
    if (layer.modifier)
        reinterpret_cast<void(__fastcall *)(void *, void *, page_camera_layer *, Float)>(
            get_vfunc(*static_cast<int *>(layer.modifier), 0))(layer.modifier, nullptr, &layer, dt);
    else
        layer.time += layer.instance->field_8 * layer.speed * dt.value;
}
template <typename T>
T read_channel(nalGeneric::nalGenericPose &pose, const char *channel, T fallback)
{
    auto *skeleton = pose.GetSkeleton();
    const tlFixedString name{"PageCam01"};
    const tlFixedString key{channel};
    int channel_index = 0;
    for (int i = 0; i < skeleton->field_88; ++i) {
        const auto &info = skeleton->field_8C[i];
        for (int j = 0; j < info.field_28; ++j, ++channel_index) {
            const auto *entry = reinterpret_cast<const char *>(skeleton->field_84 + channel_index * 40);
            const auto *node =
                reinterpret_cast<const void *>(skeleton->field_78 + 48 * *reinterpret_cast<const int *>(entry + 32));
            if (std::memcmp(entry, &key, 32) == 0 && std::memcmp(node, &name, 32) == 0) {
                T result;
                std::memcpy(static_cast<void *>(&result),
                            reinterpret_cast<const void *>(pose.field_4 + info.field_2C + j * sizeof(T)),
                            sizeof(T));
                return result;
            }
        }
    }
    return fallback;
}
void *__fastcall create(page_camera *self, void *, nalAnimClass<nalAnyPose> *anim)
{
    return self->CreateInstance(anim);
}
void __fastcall advance_scene(page_camera *self, void *, nalGeneric::nalGenericInstance *instance, Float time,
                              Float previous, int a, int b)
{
    self->Advance(instance, time, previous, a, b);
}
void __fastcall render_scene(page_camera *self, void *, nalAnimClass<nalAnyPose>::nalInstanceClass *instance,
                             Float time)
{
    self->Render(instance, time);
}
void __fastcall release_scene(page_camera *, void *) {}
void __fastcall destroy(page_camera *self, void *, bool release)
{
    self->finalize(release);
}
}

page_camera::pose_layers::pose_layers(nalGeneric::nalGenericSkeleton *value)
    : skeleton(value), field_4(value->field_CC, true), field_10(value), count(0), layers{}, active(nullptr),
      free(nullptr), generation(0)
{
    for (auto &layer : layers)
        layer = tlMemAlloc(0x28, 8, 0);
}


page_camera::page_camera()
    : field_4(static_cast<nalGeneric::nalGenericSkeleton *>(nalGetSkeleton(tlFixedString{"page_camera"}))), field_3C{},
      field_4C(identity_matrix), field_8C(identity_matrix), field_CC(1.0f), field_D0(1000.0f), field_D4(0.0f),
      field_D8(0.0f), field_DC(0.57f), field_E0(0.57f), field_E4(0.57f), field_E8(1.0f), field_EC(true),
      field_ED(false), field_EE(false), field_EF(0)
{
    static callbacks table{create, advance_scene, render_scene, release_scene, destroy};
    m_vtbl = &table;
}

page_camera::~page_camera()
{
    for (int index = 0; index < field_4.count; ++index)
        release_layer(*static_cast<page_camera_layer *>(field_4.layers[index]));
    for (auto *allocation : field_4.layers)
        tlMemFree(allocation);
    for (auto *layer = field_4.active; layer;) {
        auto *next = layer->next;
        release_layer(*layer);
        tlMemFree(layer);
        layer = next;
    }
    for (auto *layer = field_4.free; layer;) {
        auto *next = layer->next;
        tlMemFree(layer);
        layer = next;
    }
}


vector2d page_camera::ortho_size() const
{
    const float width = std::tan(field_D4 * 0.5f) * ((field_D0 + field_CC) * 0.5f);
    return {width, width / field_D8};
}


void page_camera::interpret_pose(nalGeneric::nalGenericPose &pose)
{
    const auto position = read_channel(pose, "NAL_POSITION", vector3d{});
    auto rotation = read_channel(pose, "NAL_QUATERNION", vector4d{});
    const float thresholds[]{.00135f, .00001f, .0001f, .00001f, .0001f};
    const float values[]{0.0f, 1.0f, .7071068286895752f, -1.0f, -.7071068286895752f};
    for (unsigned index = 0; index < 5; ++index) {
        if (std::fabs(rotation.w - values[index]) > thresholds[index])
            continue;
        const float w = values[index];
        const double inverse = 1.0 / std::sqrt(double(rotation.x) * rotation.x + double(rotation.y) * rotation.y +
                                               double(rotation.z) * rotation.z + double(w) * w);
        rotation.x *= inverse;
        rotation.y *= inverse;
        rotation.z *= inverse;
        const double second = 1.0 / std::sqrt(double(rotation.x) * rotation.x + double(rotation.y) * rotation.y +
                                              double(rotation.z) * rotation.z + double(w) * w);
        rotation.x *= second;
        rotation.y *= second;
        rotation.z *= second;
        rotation.w = w;
        break;
    }
    field_D4 = read_channel(pose, "MaxParamFloat.Field Of View", 0.0f);
    field_CC = std::max(0.02f, read_channel(pose, "MaxParamFloat.Near Plane", 0.0f));
    field_D0 =
        std::max(field_CC + static_cast<float>(LARGE_EPSILON), read_channel(pose, "MaxParamFloat.Far Plane", 0.0f));
    field_D8 = read_channel(pose, "MaxParamFloat.Aspect Ratio", 0.0f);
    field_EC = read_channel(pose, "MaxParamInt.Ortho", static_cast<unsigned char>(0)) != 0;
    const auto background = read_channel(pose, "MaxParamColor.Background Color", vector3d{});
    field_DC = background.x;
    field_E0 = background.y;
    field_E4 = background.z;
    field_E8 = 1.0f;
    const quaternion q{rotation.w, -rotation.x, -rotation.y, rotation.z};
    const quaternion fix{0.0f, 0.0f, -0.70710701f, -0.70710701f};
    const quaternion product{q[0] * fix[0] - q[1] * fix[1] - q[2] * fix[2] - q[3] * fix[3],
                             q[0] * fix[1] + q[1] * fix[0] + q[2] * fix[3] - q[3] * fix[2],
                             q[0] * fix[2] - q[1] * fix[3] + q[2] * fix[0] + q[3] * fix[1],
                             q[0] * fix[3] + q[1] * fix[2] - q[2] * fix[1] + q[3] * fix[0]};
    product.to_matrix(field_4C);
    matrix4x4 flip{identity_matrix};
    flip[2][2] = -1.0f;
    field_4C = flip * field_4C;
    const auto size = ortho_size();
    const vector3d page_position{-100.0f * position.x, -100.0f * position.z, 100.0f * position.y};
    field_3C = {page_position.x - size.x, page_position.y - size.y, page_position.x + size.x, page_position.y + size.y};
    field_4C[3] = vector4d{page_position.x, page_position.y, page_position.z, 1.0f};
    field_8C = field_4C;
    field_4C = field_4C.inverse();
}


void page_camera::Advance(nalGeneric::nalGenericInstance *instance, Float time, Float previous, int, int)
{
    nalGeneric::nalGenericPose pose{instance->field_C};
    instance->GetPose(time, previous, pose, instance->field_C->field_CC);
    interpret_pose(pose);
}

void page_camera::advance(Float dt)
{
    if (!field_EE)
        return;
    ++field_4.generation;
    auto **link = &field_4.active;
    while (auto *layer = *link) {
        if (layer->generation == field_4.generation) {
            link = &layer->next;
            continue;
        }
        advance_layer_time(*layer, dt);
        if (layer->event && layer->time >= layer->end) {
            void *event = layer->event;
            layer->event = nullptr;
            const int state = layer->state;
            layer->state = 2;
            const bool changed = reinterpret_cast<bool(__fastcall *)(void *, void *, void *)>(
                get_vfunc(*static_cast<int *>(event), 0))(event, nullptr, &field_4);
            if (!changed && layer->state == 2)
                layer->state = state;
            reinterpret_cast<void(__fastcall *)(void *, void *)>(get_vfunc(*static_cast<int *>(event), 8))(event,
                                                                                                           nullptr);
        }
        bool finished = false;
        if (layer->state == 0) {
            layer->weight += dt.value * layer->blend_in;
            if (layer->weight >= 1.0f) {
                if (layer->mode == 1 || layer->mode == 2)
                    for (auto *other = field_4.active; other; other = other->next)
                        if (other != layer && (layer->mask & other->mask) == other->mask &&
                            other->priority <= layer->priority) {
                            if (other->event)
                                reinterpret_cast<void(__fastcall *)(void *, void *)>(
                                    get_vfunc(*static_cast<int *>(other->event), 8))(other->event, nullptr);
                            other->event = nullptr;
                            other->state = 3;
                            other->weight = 0.0f;
                        }
                layer->weight = 1.0f;
                layer->state = 1;
            }
        } else if (layer->state == 2 || layer->state == 3) {
            if (layer->state == 2 && std::equal_to<float>{}(layer->blend_out, 0.0f))
                finished = true;
            else {
                layer->state = 3;
                layer->weight -= dt.value * layer->blend_out;
                finished = layer->weight <= 0.0f;
            }
        }
        if (finished) {
            *link = layer->next;
            release_layer(*layer);
            layer->next = field_4.free;
            field_4.free = layer;
        } else
            link = &layer->next;
    }
    int retained = 0;
    for (; retained < field_4.count; ++retained) {
        auto *layer = static_cast<page_camera_layer *>(field_4.layers[retained]);
        advance_layer_time(*layer, dt);
        if (layer->event && layer->time >= layer->end) {
            void *event = layer->event;
            layer->event = nullptr;
            reinterpret_cast<void(__fastcall *)(void *, void *, void *)>(get_vfunc(*static_cast<int *>(event), 0))(
                event, nullptr, &field_4);
            reinterpret_cast<void(__fastcall *)(void *, void *)>(get_vfunc(*static_cast<int *>(event), 8))(event,
                                                                                                           nullptr);
        }
        layer->weight += dt.value * layer->blend_in;
        if (layer->weight >= 1.0f) {
            layer->weight = 1.0f;
            ++retained;
            break;
        }
    }
    for (int index = retained; index < field_4.count; ++index)
        release_layer(*static_cast<page_camera_layer *>(field_4.layers[index]));
    field_4.count = retained;
    nalGeneric::nalGenericPose pose{field_4.skeleton};
    pose = field_4.field_4;
    page_camera_layer *start = nullptr;
    for (auto *layer = field_4.active; layer; layer = layer->next)
        if (layer->mode == 2 && layer->weight >= 1.0f)
            start = layer;
    if (!start)
        for (int index = field_4.count - 1; index >= 0; --index) {
            auto *layer = static_cast<page_camera_layer *>(field_4.layers[index]);
            layer->instance->GetPose(
                layer->time, layer->previous, field_4.field_10, layer->instance->field_C->field_CC);
            nalGeneric::Blend(&pose, layer->weight, &pose, &field_4.field_10);
            layer->previous = layer->time;
        }
    for (auto *layer = start ? start : field_4.active; layer; layer = layer->next) {
        const auto *reference = layer->mode == 2 ? &layer->instance->field_C->field_CC : &pose;
        if (layer->modifier)
            reinterpret_cast<void(__fastcall *)(void *,
                                                void *,
                                                page_camera_layer *,
                                                nalGeneric::nalGenericPose *,
                                                nalGeneric::nalGenericPose *,
                                                const nalGeneric::nalGenericPose *)>(
                get_vfunc(*static_cast<int *>(layer->modifier), 4))(
                layer->modifier, nullptr, layer, &pose, &field_4.field_10, reference);
        else {
            layer->instance->GetPose(layer->time, layer->previous, field_4.field_10, *reference);
            nalGeneric::Blend(&pose, layer->weight, &pose, &field_4.field_10);
        }
        layer->previous = layer->time;
    }
    interpret_pose(pose);
}

void page_camera::finalize(bool release)
{
    if (cur_page_camera() == this)
        cur_page_camera() = nullptr;
    if (release)
        delete this;
}


void *page_camera::CreateInstance(nalAnimClass<nalAnyPose> *anim)
{
    return anim->VirtualCreateInstance(nullptr);
}


void page_camera::Render(nalAnimClass<nalAnyPose>::nalInstanceClass *, Float) {}


page_camera *create_page_camera()
{
    cur_page_camera() = new page_camera;
    return cur_page_camera();
}
}  // namespace comic_panels
