#include "ai_tentacle_web_curly.h"

#include "ai_tentacle_info.h"
#include "common.h"
#include "func_wrapper.h"
#include "memory.h"
#include "dangler.h"
#include "game.h"
#include "camera.h"
#include "physical_interface.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>

VALIDATE_SIZE(ai_tentacle_web_curly, 0x54);

namespace {
void *__fastcall native_destroy(ai_tentacle_web_curly *self, void *, unsigned flags)
{
    self->~ai_tentacle_web_curly();
    if (flags & 1)
        ::operator delete(self);
    return self;
}
int __fastcall native_type(ai_tentacle_web_curly *, void *) { return 8; }
bool __fastcall native_advance(ai_tentacle_web_curly *self, void *, Float dt, bool modifier)
{
    return self->frame_advance(dt, modifier);
}
float random_unit() { return std::rand() * 3.0518509447574615e-05f; }
}

void *ai_tentacle_web_curly::native_vtable()
{
    static auto table = [] {
        std::array<void *, 6> result;
        auto **base = static_cast<void **>(ai_tentacle_dangle::native_vtable());
        for (unsigned i = 0; i < result.size(); ++i)
            result[i] = base[i];
        result[0] = reinterpret_cast<void *>(&native_destroy);
        result[1] = reinterpret_cast<void *>(&native_type);
        result[3] = reinterpret_cast<void *>(&native_advance);
        return result;
    }();
    return table.data();
}

ai_tentacle_web_curly::ai_tentacle_web_curly(ai_tentacle_info *info) : ai_tentacle_dangle(info), field_24{0}
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

void *ai_tentacle_web_curly::operator new(size_t size)
{
    return mem_alloc(size);
}

void ai_tentacle_web_curly::reset_curl()
{
    this->field_50 = -1;
}

void ai_tentacle_web_curly::setup(vhandle_type<actor> owner, entity_base *hand)
{
    ai_tentacle_dangle::setup(1.0f, ZEROVEC, true, false);
    field_28 = hand;
    field_24 = owner;
    const unsigned count = field_14->positions.size() + 1;
    field_2C.reserve(count);
    for (unsigned i = 0; i < count; ++i)
        field_2C.push_back(ZEROVEC);
    field_21 = false;
    field_48 = 0.0f;
    field_3C = ZVEC;
    field_50 = 0;
}

bool ai_tentacle_web_curly::frame_advance(Float dt, bool modifier)
{
    auto *owner = field_24.get_volatile_ptr();
    if (!owner)
        return false;
    vector3d velocity = owner->physical_ifc()->get_velocity();
    const float speed = velocity.length();
    if (speed > 20.0f)
        velocity *= 20.0f / speed;
    vector3d down = vector3d{0.0f, -20.0f, 0.0f} - velocity;
    if (down.length() < EPSILON)
        down.y = -1.0f;
    tentacle_dangler->gravity = down;
    tentacle_dangler->damping = 5.0f;
    tentacle_dangler->constraint_iterations = 16;
    if (ai_tentacle_dangle::frame_advance(dt, modifier))
        return true;

    const float blend = std::min(speed / 25.0f, 1.0f);
    if ((field_48 >= 1.0f && blend < 1.0f) || field_50 < 0) {

        const float choice = random_unit() * (0.33f + 0.33f + 0.33f);
        field_50 = choice <= 0.33f ? 0 : choice <= 0.33f + 0.33f ? 1 : 2;
        field_4C = random_unit() * 2.0f - 1.0f + 2.0f;
        const float z = random_unit() * 2.0f - 1.0f;
        const float y = random_unit() * 2.0f - 1.0f;
        field_3C = {random_unit() * 2.0f - 1.0f, y, z};
        if (field_3C.length() < EPSILON)
            field_3C = g_game_ptr->get_current_view_camera(0)->get_abs_po().get_x_facing();
        field_3C.normalize();
    }
    field_48 = blend;
    if (blend < 1.0f) {
        if (is_colinear(field_3C, down, 0.01f)) {
            field_3C = ZVEC;
            if (is_colinear(field_3C, down, 0.01f))
                field_3C = XVEC;
        }
        po basis;
        basis.set_po(field_3C, down, ZEROVEC);
        field_3C = basis.get_z_facing();
        const auto &x = basis.get_x_facing();
        const auto &y = basis.get_y_facing();
        const auto &z = basis.get_z_facing();
        vector3d position = field_14->base_node ? field_14->base_node->get_abs_position() : field_14->field_60;
        const int count = static_cast<int>(field_2C.size());
        if (field_50 == 0) {
            int straight = 0;
            int curl = 0;
            int phase = 0;
            position += y * 0.1f;
            for (auto &point : field_2C) {
                point = position;
                if (straight < 1) {
                    ++straight;
                    position += y * 0.1f;
                } else if (curl > 3) {
                    straight = 0;
                    curl = 0;
                    phase = 0;
                } else {
                    const float angle = static_cast<float>(phase) / 3.0f * 6.283185307179586f;
                    phase = ++curl;
                    point += (y * std::sin(angle) + x * std::abs(std::cos(angle) - 1.0f)) * 0.1f;
                }
            }
        } else if (field_50 > 0 && field_50 <= 2) {
            const float radius = field_50 == 1 ? 0.2f : 0.1f;
            const float step = 6.283185307179586f / count * field_4C;
            float increment = 1.0f / (count + 1);
            if (field_50 == 1)
                increment *= 2.0f;
            float angle = 0.0f;
            float amount = 0.0f;
            for (auto &point : field_2C) {
                angle += step;
                amount += increment;
                if (field_50 == 1) {
                    if (amount >= 1.0f) {
                        amount = 1.0f;
                        increment = -std::abs(increment);
                    } else if (amount <= 0.0f) {
                        amount = 0.0f;
                        increment = std::abs(increment);
                    }
                }
                position += y * 0.05f;
                point = position + (x * std::sin(angle) + z * std::cos(angle)) * (radius * amount * amount);
            }
        }
    }
    const unsigned count = field_14->positions.size();
    for (unsigned i = 0; i <= count; ++i) {
        auto &point = i < count ? field_14->positions[i] : field_14->end_pos;
        point = field_2C[i] + (point - field_2C[i]) * field_48;
    }
    return false;
}
